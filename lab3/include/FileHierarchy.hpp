#pragma once

#include <string>
#include <vector>

class Inode;
class PermissionMask;
class UserAccount;
class Directory;
class DirectoryWatcher;
class FileMetadata;
class FileLock;
class ByteBuffer;
class CompressionCodec;
class EncryptionEngine;
class CryptoKey;

class FileSystemNode {
protected:
    int nodeId;
    std::string nodeName;
    std::string absolutePath;
    bool isHiddenNode;
    Directory* parentDirectory;
    Inode* associatedInode;
    UserAccount* nodeOwner;

public:
    FileSystemNode(int id, std::string name, std::string path, Directory* parent, Inode* inode, UserAccount* owner);
    virtual ~FileSystemNode() = default;

    int getNodeId() const noexcept;
    const std::string& getNodeName() const noexcept;
    const std::string& getAbsolutePath() const noexcept;
    void renameNode(const std::string& newName);
    void moveNode(Directory* newParent, const std::string& newPath);
    void setHidden(bool hidden) noexcept;
    bool isHidden() const noexcept;
    bool operator==(const FileSystemNode& other) const noexcept;
};

class Directory : public FileSystemNode {
private:
    size_t maxEntriesLimit;
    bool isSystemDirectory;
    DirectoryWatcher* activeWatcher;
    std::vector<FileSystemNode*> childNodes;

public:
    static constexpr size_t DEFAULT_MAX_ENTRIES = 1024;

    Directory(int id, std::string name, std::string path, Directory* parent, Inode* inode, UserAccount* owner);
    void addChildNode(FileSystemNode* node);
    void removeChildNode(const std::string& targetName);
    FileSystemNode* findChildByName(const std::string& targetName) const noexcept;
    size_t getChildrenCount() const noexcept;
    bool isEmpty() const noexcept;
    void attachWatcher(DirectoryWatcher* watcher);
    size_t calculateTotalDirectorySize() const noexcept;
};

class SymbolicLink : public FileSystemNode {
private:
    std::string targetRawPath;
    FileSystemNode* resolvedTargetNode;
    int resolutionHopDepth;

public:
    static constexpr int MAX_SYMLINK_HOPS = 8;

    SymbolicLink(int id, std::string name, std::string path, Directory* parent, Inode* inode,
                 UserAccount* owner, std::string targetPath, FileSystemNode* targetNode);
    FileSystemNode* resolveLink(int currentHopCount = 0) const;
    void retargetLink(const std::string& newPath, FileSystemNode* newTarget);
    bool isDanglingLink() const noexcept;
};

class HardLink : public FileSystemNode {
private:
    int linkReferenceId;
    Inode* sharedTargetInode;

public:
    HardLink(int id, std::string name, std::string path, Directory* parent, Inode* sharedInode, UserAccount* owner);
    void unlinkNode();
    bool pointsToSameInode(const Inode* otherInode) const noexcept;
};

class File : public FileSystemNode {
protected:
    std::string fileExtension;
    size_t contentSizeBytes;
    bool isReadOnlyFlag;
    FileMetadata* fileMetadata;
    FileLock* activeLock;

public:
    File(int id, std::string name, std::string ext, std::string path, Directory* parent,
         Inode* inode, UserAccount* owner, size_t initialSize);
    size_t getFileSize() const noexcept;
    const std::string& getExtension() const noexcept;
    void writeContentBytes(size_t addedBytes);
    void truncateFile();
    void attachMetadata(FileMetadata* metadata);
    void acquireLock(FileLock* lock);
    void releaseLock() noexcept;
    bool isLocked() const noexcept;
};

class TextFile : public File {
protected:
    std::string characterEncoding;
    int lineCount;
    std::string textPayload;

public:
    TextFile(int id, std::string name, std::string path, Directory* parent, Inode* inode,
             UserAccount* owner, std::string encoding);
    void appendTextLine(const std::string& line);
    void clearText();
    bool containsSubstring(const std::string& keyword) const noexcept;
    int getLineCount() const noexcept;
    const std::string& getTextPayload() const noexcept;
};

class ConfigFile : public TextFile {
private:
    std::string configFormat;
    int keysCount;
    bool isValidatedSchema;

public:
    ConfigFile(int id, std::string name, std::string path, Directory* parent, Inode* inode,
               UserAccount* owner, std::string format);
    void setConfigEntry(const std::string& key, const std::string& value);
    bool validateConfigurationSyntax();
    bool isSchemaValid() const noexcept;
};

class LogFile : public TextFile {
private:
    size_t maxLogSizeBytes;
    int rotationGeneration;
    std::string severityThreshold;

public:
    LogFile(int id, std::string name, std::string path, Directory* parent, Inode* inode,
            UserAccount* owner, size_t maxBytes);
    void appendLogEntry(const std::string& severity, const std::string& message);
    void rotateLogFile();
    bool requiresRotation() const noexcept;
    int getRotationGeneration() const noexcept;
};

class BinaryFile : public File {
protected:
    std::string mimeTypeSignature;
    int binaryHeaderMagic;
    ByteBuffer* internalBuffer;

public:
    BinaryFile(int id, std::string name, std::string ext, std::string path, Directory* parent,
               Inode* inode, UserAccount* owner, std::string mimeType, int magic);
    void loadIntoBuffer(ByteBuffer* buffer);
    bool verifyMagicHeader(int expectedMagic) const noexcept;
};

class ExecutableFile : public BinaryFile {
private:
    std::string targetArchitecture;
    bool isDigitallySigned;
    PermissionMask* executionMask;

public:
    ExecutableFile(int id, std::string name, std::string path, Directory* parent, Inode* inode,
                   UserAccount* owner, std::string arch, PermissionMask* mask);
    void signBinary() noexcept;
    void executeProcess(const UserAccount* caller) const;
    bool canExecute() const noexcept;
};

class MediaFile : public BinaryFile {
private:
    int durationSeconds;
    int bitrateKbps;
    std::string codecName;
    std::string resolutionDimensions;

public:
    MediaFile(int id, std::string name, std::string ext, std::string path, Directory* parent,
              Inode* inode, UserAccount* owner, int duration, int bitrate, std::string codec);
    double calculateUncompressedSizeMB() const noexcept;
    void transcodeStream(const std::string& newCodec, int newBitrate);
    bool isHighDefinition() const noexcept;
};

class ArchiveFile : public File {
private:
    double compressionRatio;
    bool isPasswordProtected;
    CompressionCodec* usedCodec;
    std::vector<FileSystemNode*> bundledEntries;

public:
    ArchiveFile(int id, std::string name, std::string path, Directory* parent, Inode* inode,
                UserAccount* owner, CompressionCodec* codec);
    void addNodeToArchive(FileSystemNode* node, size_t compressedBytes);
    void extractAllToDirectory(Directory* destinationDir) const;
    size_t getArchivedEntriesCount() const noexcept;
};

class EncryptedFile : public File {
private:
    std::string cipherAlgorithm;
    bool isCurrentlyDecrypted;
    EncryptionEngine* cryptoEngine;
    CryptoKey* requiredKey;

public:
    EncryptedFile(int id, std::string name, std::string path, Directory* parent, Inode* inode,
                  UserAccount* owner, std::string algorithm, EncryptionEngine* engine, CryptoKey* key);
    void encryptPayload(const CryptoKey* providedKey);
    void decryptPayload(const CryptoKey* providedKey);
    bool isDecrypted() const noexcept;
};