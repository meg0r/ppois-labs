#include "FileHierarchy.hpp"
#include "Exceptions.hpp"
#include "IOOperations.hpp"
#include "SearchAndMetadata.hpp"
#include "SecurityAccess.hpp"
#include "StorageHardware.hpp"

#include <algorithm>
#include <utility>

// ==========================================
// FileSystemNode
// ==========================================

FileSystemNode::FileSystemNode(int id, std::string name, std::string path, Directory* parent,
                               Inode* inode, UserAccount* owner)
    : nodeId(id),
      nodeName(std::move(name)),
      absolutePath(std::move(path)),
      isHiddenNode(false),
      parentDirectory(parent),
      associatedInode(inode),
      nodeOwner(owner) {
    if (id <= 0 || nodeName.empty() || absolutePath.empty()) {
        throw InvalidPathException("FileSystemNode requires positive ID, non-empty name and path.");
    }
    if (!nodeName.empty() && nodeName[0] == '.') {
        isHiddenNode = true;
    }
}

int FileSystemNode::getNodeId() const noexcept { return nodeId; }
const std::string& FileSystemNode::getNodeName() const noexcept { return nodeName; }
const std::string& FileSystemNode::getAbsolutePath() const noexcept { return absolutePath; }

void FileSystemNode::renameNode(const std::string& newName) {
    if (newName.empty() || newName.find('/') != std::string::npos) {
        throw InvalidPathException("New node name is invalid or contains path separators.");
    }
    nodeName = newName;
    isHiddenNode = (nodeName[0] == '.');
}

void FileSystemNode::moveNode(Directory* newParent, const std::string& newPath) {
    if (newParent == nullptr || newPath.empty()) {
        throw InvalidPathException("Target parent directory and path must be valid.");
    }
    parentDirectory = newParent;
    absolutePath = newPath;
}

void FileSystemNode::setHidden(bool hidden) noexcept { isHiddenNode = hidden; }
bool FileSystemNode::isHidden() const noexcept { return isHiddenNode; }

bool FileSystemNode::operator==(const FileSystemNode& other) const noexcept {
    return nodeId == other.nodeId;
}

// ==========================================
// Directory
// ==========================================

Directory::Directory(int id, std::string name, std::string path, Directory* parent, Inode* inode, UserAccount* owner)
    : FileSystemNode(id, std::move(name), std::move(path), parent, inode, owner),
      maxEntriesLimit(DEFAULT_MAX_ENTRIES),
      isSystemDirectory(false),
      activeWatcher(nullptr) {}

void Directory::addChildNode(FileSystemNode* node) {
    if (node == nullptr) {
        throw FileSystemException("Cannot add a null node to directory.");
    }
    if (childNodes.size() >= maxEntriesLimit) {
        throw OutOfDiskSpaceException("Directory maximum entries limit reached.");
    }
    if (findChildByName(node->getNodeName()) != nullptr) {
        throw FileAlreadyExistsException("Node with the same name already exists in directory.");
    }
    childNodes.push_back(node);
}

void Directory::removeChildNode(const std::string& targetName) {
    auto it = std::find_if(childNodes.begin(), childNodes.end(),
        [&targetName](const FileSystemNode* n) { return n->getNodeName() == targetName; });
    if (it == childNodes.end()) {
        throw FileNotFoundException("Target child node not found in directory.");
    }
    childNodes.erase(it);
}

FileSystemNode* Directory::findChildByName(const std::string& targetName) const noexcept {
    for (auto* node : childNodes) {
        if (node->getNodeName() == targetName) { return node; }
    }
    return nullptr;
}

size_t Directory::getChildrenCount() const noexcept { return childNodes.size(); }
bool Directory::isEmpty() const noexcept { return childNodes.empty(); }

void Directory::attachWatcher(DirectoryWatcher* watcher) {
    if (watcher == nullptr) {
        throw FileSystemException("DirectoryWatcher pointer cannot be null.");
    }
    activeWatcher = watcher;
}

size_t Directory::calculateTotalDirectorySize() const noexcept {
    size_t totalBytes = 0;
    for (const auto* node : childNodes) {
        if (const auto* file = dynamic_cast<const File*>(node)) {
            totalBytes += file->getFileSize();
        }
    }
    return totalBytes;
}

// ==========================================
// SymbolicLink
// ==========================================

SymbolicLink::SymbolicLink(int id, std::string name, std::string path, Directory* parent, Inode* inode,
                           UserAccount* owner, std::string targetPath, FileSystemNode* targetNode)
    : FileSystemNode(id, std::move(name), std::move(path), parent, inode, owner),
      targetRawPath(std::move(targetPath)),
      resolvedTargetNode(targetNode),
      resolutionHopDepth(1) {
    if (targetRawPath.empty()) {
        throw InvalidPathException("Symbolic link target path cannot be empty.");
    }
}

FileSystemNode* SymbolicLink::resolveLink(int currentHopCount) const {
    if (currentHopCount >= MAX_SYMLINK_HOPS) {
        throw CyclicSymlinkException("Symbolic link resolution exceeded maximum hop limit (cycle detected).");
    }
    if (resolvedTargetNode == nullptr) {
        throw FileNotFoundException("Symbolic link points to a non-existent target (dangling link).");
    }
    if (const auto* nestedSymlink = dynamic_cast<const SymbolicLink*>(resolvedTargetNode)) {
        return nestedSymlink->resolveLink(currentHopCount + 1);
    }
    return resolvedTargetNode;
}

void SymbolicLink::retargetLink(const std::string& newPath, FileSystemNode* newTarget) {
    if (newPath.empty()) {
        throw InvalidPathException("New symbolic link target path cannot be empty.");
    }
    targetRawPath = newPath;
    resolvedTargetNode = newTarget;
}

bool SymbolicLink::isDanglingLink() const noexcept {
    return resolvedTargetNode == nullptr;
}

// ==========================================
// HardLink
// ==========================================

HardLink::HardLink(int id, std::string name, std::string path, Directory* parent, Inode* sharedInode, UserAccount* owner)
    : FileSystemNode(id, std::move(name), std::move(path), parent, sharedInode, owner),
      linkReferenceId(id),
      sharedTargetInode(sharedInode) {
    if (sharedInode == nullptr) {
        throw FileNotFoundException("Hard link requires a valid existing target Inode.");
    }
    sharedTargetInode->incrementLinkCount();
}

void HardLink::unlinkNode() {
    if (sharedTargetInode == nullptr) {
        throw FileSystemException("Hard link is already unlinked.");
    }
    sharedTargetInode->decrementLinkCount();
    sharedTargetInode = nullptr;
}

bool HardLink::pointsToSameInode(const Inode* otherInode) const noexcept {
    return (sharedTargetInode != nullptr) && (sharedTargetInode == otherInode);
}

// ==========================================
// File
// ==========================================

File::File(int id, std::string name, std::string ext, std::string path, Directory* parent,
           Inode* inode, UserAccount* owner, size_t initialSize)
    : FileSystemNode(id, std::move(name), std::move(path), parent, inode, owner),
      fileExtension(std::move(ext)),
      contentSizeBytes(initialSize),
      isReadOnlyFlag(false),
      fileMetadata(nullptr),
      activeLock(nullptr) {}

size_t File::getFileSize() const noexcept { return contentSizeBytes; }
const std::string& File::getExtension() const noexcept { return fileExtension; }

void File::writeContentBytes(size_t addedBytes) {
    if (isReadOnlyFlag) {
        throw PermissionDeniedException("Cannot write to a read-only file.");
    }
    if (isLocked()) {
        throw FileLockedException("Cannot modify file while an active lock is held.");
    }
    contentSizeBytes += addedBytes;
}

void File::truncateFile() {
    if (isReadOnlyFlag) {
        throw PermissionDeniedException("Cannot truncate a read-only file.");
    }
    if (isLocked()) {
        throw FileLockedException("Cannot truncate a locked file.");
    }
    contentSizeBytes = 0;
}

void File::attachMetadata(FileMetadata* metadata) {
    if (metadata == nullptr) {
        throw FileSystemException("FileMetadata pointer cannot be null.");
    }
    fileMetadata = metadata;
}

void File::acquireLock(FileLock* lock) {
    if (lock == nullptr) {
        throw FileSystemException("FileLock pointer cannot be null.");
    }
    if (activeLock != nullptr) {
        throw FileLockedException("File is already locked by another session.");
    }
    activeLock = lock;
}

void File::releaseLock() noexcept { activeLock = nullptr; }
bool File::isLocked() const noexcept { return activeLock != nullptr; }

// ==========================================
// TextFile
// ==========================================

TextFile::TextFile(int id, std::string name, std::string path, Directory* parent, Inode* inode,
                   UserAccount* owner, std::string encoding)
    : File(id, std::move(name), "txt", std::move(path), parent, inode, owner, 0),
      characterEncoding(std::move(encoding)),
      lineCount(0),
      textPayload("") {
    if (characterEncoding.empty()) {
        throw FileSystemException("TextFile character encoding cannot be empty.");
    }
}

void TextFile::appendTextLine(const std::string& line) {
    writeContentBytes(line.size() + 1);
    textPayload += line + "\n";
    ++lineCount;
}

void TextFile::clearText() {
    truncateFile();
    textPayload.clear();
    lineCount = 0;
}

bool TextFile::containsSubstring(const std::string& keyword) const noexcept {
    return !keyword.empty() && (textPayload.find(keyword) != std::string::npos);
}

int TextFile::getLineCount() const noexcept { return lineCount; }
const std::string& TextFile::getTextPayload() const noexcept { return textPayload; }

// ==========================================
// ConfigFile
// ==========================================

ConfigFile::ConfigFile(int id, std::string name, std::string path, Directory* parent, Inode* inode,
                       UserAccount* owner, std::string format)
    : TextFile(id, std::move(name), std::move(path), parent, inode, owner, "UTF-8"),
      configFormat(std::move(format)),
      keysCount(0),
      isValidatedSchema(true) {
    if (configFormat.empty()) {
        throw FileSystemException("Config format specification cannot be empty.");
    }
}

void ConfigFile::setConfigEntry(const std::string& key, const std::string& value) {
    if (key.empty() || value.empty()) {
        throw FileSystemException("Configuration key and value cannot be empty.");
    }
    appendTextLine(key + "=" + value);
    ++keysCount;
}

bool ConfigFile::validateConfigurationSyntax() {
    isValidatedSchema = (keysCount >= 0) && (textPayload.find('=') != std::string::npos || keysCount == 0);
    if (!isValidatedSchema) {
        throw CorruptedFileException("Configuration file syntax validation failed.");
    }
    return isValidatedSchema;
}

bool ConfigFile::isSchemaValid() const noexcept { return isValidatedSchema; }

// ==========================================
// LogFile
// ==========================================

LogFile::LogFile(int id, std::string name, std::string path, Directory* parent, Inode* inode,
                 UserAccount* owner, size_t maxBytes)
    : TextFile(id, std::move(name), std::move(path), parent, inode, owner, "UTF-8"),
      maxLogSizeBytes(maxBytes),
      rotationGeneration(0),
      severityThreshold("INFO") {
    if (maxBytes == 0) {
        throw FileSystemException("Maximum log file size must be positive.");
    }
}

void LogFile::appendLogEntry(const std::string& severity, const std::string& message) {
    if (severity.empty() || message.empty()) {
        throw FileSystemException("Log severity and message cannot be empty.");
    }
    appendTextLine("[" + severity + "] " + message);
}

void LogFile::rotateLogFile() {
    clearText();
    ++rotationGeneration;
}

bool LogFile::requiresRotation() const noexcept {
    return contentSizeBytes >= maxLogSizeBytes;
}

int LogFile::getRotationGeneration() const noexcept { return rotationGeneration; }

// ==========================================
// BinaryFile
// ==========================================

BinaryFile::BinaryFile(int id, std::string name, std::string ext, std::string path, Directory* parent,
                       Inode* inode, UserAccount* owner, std::string mimeType, int magic)
    : File(id, std::move(name), std::move(ext), std::move(path), parent, inode, owner, 0),
      mimeTypeSignature(std::move(mimeType)),
      binaryHeaderMagic(magic),
      internalBuffer(nullptr) {
    if (mimeTypeSignature.empty()) {
        throw FileSystemException("BinaryFile MIME type cannot be empty.");
    }
}

void BinaryFile::loadIntoBuffer(ByteBuffer* buffer) {
    if (buffer == nullptr) {
        throw FileSystemException("ByteBuffer pointer cannot be null.");
    }
    internalBuffer = buffer;
}

bool BinaryFile::verifyMagicHeader(int expectedMagic) const noexcept {
    return binaryHeaderMagic == expectedMagic;
}

// ==========================================
// ExecutableFile
// ==========================================

ExecutableFile::ExecutableFile(int id, std::string name, std::string path, Directory* parent, Inode* inode,
                               UserAccount* owner, std::string arch, PermissionMask* mask)
    : BinaryFile(id, std::move(name), "bin", std::move(path), parent, inode, owner, "application/x-executable", 0x7F454C46),
      targetArchitecture(std::move(arch)),
      isDigitallySigned(false),
      executionMask(mask) {
    if (targetArchitecture.empty() || mask == nullptr) {
        throw FileSystemException("ExecutableFile requires target architecture and permission mask.");
    }
}

void ExecutableFile::signBinary() noexcept { isDigitallySigned = true; }

void ExecutableFile::executeProcess(const UserAccount* caller) const {
    if (caller == nullptr) {
        throw AuthenticationFailedException("Caller user account is required to execute binary.");
    }
    if (!canExecute()) {
        throw PermissionDeniedException("Executable bit is not set in permission mask.");
    }
}

bool ExecutableFile::canExecute() const noexcept {
    return (executionMask != nullptr) && executionMask->canExecute();
}

// ==========================================
// MediaFile
// ==========================================

MediaFile::MediaFile(int id, std::string name, std::string ext, std::string path, Directory* parent,
                     Inode* inode, UserAccount* owner, int duration, int bitrate, std::string codec)
    : BinaryFile(id, std::move(name), std::move(ext), std::move(path), parent, inode, owner, "video/mp4", 0x66747970),
      durationSeconds(duration),
      bitrateKbps(bitrate),
      codecName(std::move(codec)),
      resolutionDimensions("1920x1080") {
    if (duration <= 0 || bitrate <= 0 || codecName.empty()) {
        throw FileSystemException("Invalid media file stream parameters.");
    }
}

double MediaFile::calculateUncompressedSizeMB() const noexcept {
    constexpr double BITS_PER_BYTE = 8.0;
    constexpr double KB_PER_MB = 1024.0;
    return (static_cast<double>(durationSeconds) * static_cast<double>(bitrateKbps)) / (BITS_PER_BYTE * KB_PER_MB);
}

void MediaFile::transcodeStream(const std::string& newCodec, int newBitrate) {
    if (newCodec.empty() || newBitrate <= 0) {
        throw FileSystemException("Transcoding requires valid codec and positive bitrate.");
    }
    codecName = newCodec;
    bitrateKbps = newBitrate;
}

bool MediaFile::isHighDefinition() const noexcept {
    return resolutionDimensions == "1920x1080" || resolutionDimensions == "3840x2160";
}

// ==========================================
// ArchiveFile
// ==========================================

ArchiveFile::ArchiveFile(int id, std::string name, std::string path, Directory* parent, Inode* inode,
                         UserAccount* owner, CompressionCodec* codec)
    : File(id, std::move(name), "zip", std::move(path), parent, inode, owner, 0),
      compressionRatio(0.5),
      isPasswordProtected(false),
      usedCodec(codec) {
    if (codec == nullptr) {
        throw FileSystemException("ArchiveFile requires a valid CompressionCodec.");
    }
}

void ArchiveFile::addNodeToArchive(FileSystemNode* node, size_t compressedBytes) {
    if (node == nullptr) {
        throw FileNotFoundException("Cannot archive a null filesystem node.");
    }
    bundledEntries.push_back(node);
    writeContentBytes(compressedBytes);
}

void ArchiveFile::extractAllToDirectory(Directory* destinationDir) const {
    if (destinationDir == nullptr) {
        throw DirectoryNotFoundException("Destination directory for extraction cannot be null.");
    }
    for (auto* entry : bundledEntries) {
        destinationDir->addChildNode(entry);
    }
}

size_t ArchiveFile::getArchivedEntriesCount() const noexcept {
    return bundledEntries.size();
}

// ==========================================
// EncryptedFile
// ==========================================

EncryptedFile::EncryptedFile(int id, std::string name, std::string path, Directory* parent, Inode* inode,
                             UserAccount* owner, std::string algorithm, EncryptionEngine* engine, CryptoKey* key)
    : File(id, std::move(name), "enc", std::move(path), parent, inode, owner, 0),
      cipherAlgorithm(std::move(algorithm)),
      isCurrentlyDecrypted(false),
      cryptoEngine(engine),
      requiredKey(key) {
    if (cipherAlgorithm.empty() || engine == nullptr || key == nullptr) {
        throw FileSystemException("EncryptedFile requires cipher algorithm, engine, and key.");
    }
}

void EncryptedFile::encryptPayload(const CryptoKey* providedKey) {
    if (providedKey == nullptr || !(*providedKey == *requiredKey)) {
        throw PermissionDeniedException("Invalid cryptographic key for encryption.");
    }
    isCurrentlyDecrypted = false;
}

void EncryptedFile::decryptPayload(const CryptoKey* providedKey) {
    if (providedKey == nullptr || !(*providedKey == *requiredKey)) {
        throw PermissionDeniedException("Decryption failed: cryptographic key mismatch.");
    }
    isCurrentlyDecrypted = true;
}

bool EncryptedFile::isDecrypted() const noexcept { return isCurrentlyDecrypted; }