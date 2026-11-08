#include "IOOperations.hpp"
#include "Exceptions.hpp"
#include "FileHierarchy.hpp"
#include "SecurityAccess.hpp"

#include <algorithm>
#include <cstring>
#include <utility>

// ==========================================
// ByteBuffer
// ==========================================

ByteBuffer::ByteBuffer(int id, size_t capacity)
    : bufferId(id),
      capacityBytes(capacity),
      dataSizeBytes(0),
      rawMemoryData(new char[capacity]()) {
    if (id <= 0 || capacity == 0) {
        throw FileSystemException("ByteBuffer requires positive ID and capacity.");
    }
}

ByteBuffer::~ByteBuffer() {
    delete[] rawMemoryData;
}

int ByteBuffer::getBufferId() const noexcept { return bufferId; }
size_t ByteBuffer::getCapacity() const noexcept { return capacityBytes; }

void ByteBuffer::writeBytes(const char* data, size_t length) {
    if (data == nullptr || length == 0) {
        throw FileSystemException("Source data pointer cannot be null and length must be positive.");
    }
    if (dataSizeBytes + length > capacityBytes) {
        throw OutOfDiskSpaceException("ByteBuffer capacity exceeded.");
    }
    std::memcpy(rawMemoryData + dataSizeBytes, data, length);
    dataSizeBytes += length;
}

void ByteBuffer::clearBuffer() noexcept {
    std::memset(rawMemoryData, 0, capacityBytes);
    dataSizeBytes = 0;
}

// ==========================================
// FileDescriptor
// ==========================================

FileDescriptor::FileDescriptor(int id, int mode, File* file, UserAccount* owner)
    : descriptorId(id),
      accessModeFlags(mode),
      isOpenStatus(true),
      targetFile(file),
      fileOwner(owner) {
    if (id <= 0 || file == nullptr || owner == nullptr) {
        throw FileSystemException("FileDescriptor requires valid ID, file, and owner.");
    }
}

int FileDescriptor::getDescriptorId() const noexcept { return descriptorId; }
void FileDescriptor::closeDescriptor() noexcept { isOpenStatus = false; }
bool FileDescriptor::isOpen() const noexcept { return isOpenStatus; }
File* FileDescriptor::getTargetFile() const noexcept { return targetFile; }

// ==========================================
// FileLock
// ==========================================

FileLock::FileLock(int id, bool exclusive, int userId, File* file)
    : lockId(id),
      isExclusiveLock(exclusive),
      holderUserId(userId),
      lockedFile(file) {
    if (id <= 0 || userId <= 0 || file == nullptr) {
        throw FileSystemException("FileLock requires positive ID, user ID, and target file.");
    }
}

int FileLock::getLockId() const noexcept { return lockId; }
bool FileLock::isExclusive() const noexcept { return isExclusiveLock; }
bool FileLock::matchesHolder(int userId) const noexcept { return holderUserId == userId; }

// ==========================================
// FileStream
// ==========================================

FileStream::FileStream(int id, FileDescriptor* descriptor, ByteBuffer* buffer)
    : streamId(id),
      currentCursorPosition(0),
      isAtEofFlag(false),
      activeDescriptor(descriptor),
      internalIoBuffer(buffer) {
    if (id <= 0 || descriptor == nullptr || !descriptor->isOpen() || buffer == nullptr) {
        throw FileSystemException("FileStream requires an open descriptor and a valid buffer.");
    }
}

int FileStream::getStreamId() const noexcept { return streamId; }

size_t FileStream::readStream(char* destinationBuffer, size_t bytesToRead) {
    if (destinationBuffer == nullptr || bytesToRead == 0) {
        throw FileSystemException("Destination buffer cannot be null and read size must be positive.");
    }
    if (!activeDescriptor->isOpen()) {
        throw PermissionDeniedException("Cannot read from a closed file descriptor stream.");
    }
    currentCursorPosition += bytesToRead;
    if (currentCursorPosition >= activeDescriptor->getTargetFile()->getFileSize()) {
        isAtEofFlag = true;
    }
    return bytesToRead;
}

void FileStream::writeStream(const char* sourceData, size_t bytesToWrite) {
    if (sourceData == nullptr || bytesToWrite == 0) {
        throw FileSystemException("Source data cannot be null and write size must be positive.");
    }
    if (!activeDescriptor->isOpen()) {
        throw PermissionDeniedException("Cannot write to a closed file descriptor stream.");
    }
    activeDescriptor->getTargetFile()->writeContentBytes(bytesToWrite);
    currentCursorPosition += bytesToWrite;
}

void FileStream::seekCursor(size_t absoluteOffset) {
    currentCursorPosition = absoluteOffset;
    isAtEofFlag = (currentCursorPosition >= activeDescriptor->getTargetFile()->getFileSize());
}

bool FileStream::isEof() const noexcept { return isAtEofFlag; }

// ==========================================
// FileCopyTask
// ==========================================

FileCopyTask::FileCopyTask(int id, File* source, Directory* dest)
    : taskId(id),
      totalBytesToCopy(source ? source->getFileSize() : 0),
      copiedBytesCount(0),
      isTaskCompleted(false),
      sourceFile(source),
      destinationDirectory(dest) {
    if (id <= 0 || source == nullptr || dest == nullptr) {
        throw FileSystemException("FileCopyTask requires valid source file and destination directory.");
    }
}

int FileCopyTask::getTaskId() const noexcept { return taskId; }

void FileCopyTask::executeCopyStep(size_t chunkBytes) {
    if (isTaskCompleted) {
        throw FileSystemException("Copy task is already completed.");
    }
    copiedBytesCount += chunkBytes;
    if (copiedBytesCount >= totalBytesToCopy) {
        copiedBytesCount = totalBytesToCopy;
        isTaskCompleted = true;
    }
}

bool FileCopyTask::isCompleted() const noexcept { return isTaskCompleted; }

double FileCopyTask::getProgressPercentage() const noexcept {
    if (totalBytesToCopy == 0) { return 100.0; }
    constexpr double PERCENT_FACTOR = 100.0;
    return (static_cast<double>(copiedBytesCount) / static_cast<double>(totalBytesToCopy)) * PERCENT_FACTOR;
}

// ==========================================
// PathResolver
// ==========================================

PathResolver::PathResolver(int id, int maxDepth, Directory* root)
    : resolverId(id),
      maxResolutionDepth(maxDepth),
      rootDirectoryReference(root) {
    if (id <= 0 || maxDepth <= 0 || root == nullptr) {
        throw FileSystemException("PathResolver requires positive ID, depth, and root directory.");
    }
}

int PathResolver::getResolverId() const noexcept { return resolverId; }

FileSystemNode* PathResolver::resolveAbsolutePath(const std::string& absolutePath) const {
    if (absolutePath.empty() || absolutePath[0] != '/') {
        throw InvalidPathException("Path must be absolute and start with '/'");
    }
    if (absolutePath == "/") {
        return rootDirectoryReference;
    }
    FileSystemNode* current = rootDirectoryReference;
    if (auto* dir = dynamic_cast<Directory*>(current)) {
        std::string subName = absolutePath.substr(1);
        return dir->findChildByName(subName);
    }
    throw FileNotFoundException("Path resolution failed.");
}

std::string PathResolver::normalizePath(const std::string& rawPath) const {
    if (rawPath.empty()) {
        throw InvalidPathException("Cannot normalize an empty path.");
    }
    return rawPath;
}

// ==========================================
// ChecksumValidator
// ==========================================

ChecksumValidator::ChecksumValidator(int id, std::string algorithm)
    : validatorId(id),
      hashingAlgorithmName(std::move(algorithm)) {
    if (id <= 0 || hashingAlgorithmName.empty()) {
        throw FileSystemException("ChecksumValidator requires valid ID and algorithm name.");
    }
}

int ChecksumValidator::getValidatorId() const noexcept { return validatorId; }

std::string ChecksumValidator::calculateFileChecksum(const File* file) const {
    if (file == nullptr) {
        throw FileNotFoundException("Cannot calculate checksum for a null file.");
    }
    return "SHA256_MOCK_HASH_" + std::to_string(file->getFileSize());
}

bool ChecksumValidator::verifyChecksum(const File* file, const std::string& expectedChecksum) const {
    return calculateFileChecksum(file) == expectedChecksum;
}

// ==========================================
// CompressionCodec
// ==========================================

CompressionCodec::CompressionCodec(int id, std::string name, int level)
    : codecId(id),
      codecName(std::move(name)),
      compressionLevel(level) {
    if (id <= 0 || codecName.empty() || level < 0 || level > 9) {
        throw FileSystemException("CompressionCodec requires valid ID, name, and level [0-9].");
    }
}

int CompressionCodec::getCodecId() const noexcept { return codecId; }

void CompressionCodec::compressBuffer(ByteBuffer* sourceBuffer, ByteBuffer* destBuffer) const {
    if (sourceBuffer == nullptr || destBuffer == nullptr) {
        throw FileSystemException("Source and destination buffers cannot be null.");
    }
}

void CompressionCodec::decompressBuffer(ByteBuffer* sourceBuffer, ByteBuffer* destBuffer) const {
    if (sourceBuffer == nullptr || destBuffer == nullptr) {
        throw FileSystemException("Source and destination buffers cannot be null.");
    }
}