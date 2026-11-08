#pragma once

#include <string>
#include <vector>

class File;
class UserAccount;
class Directory;
class FileSystemNode;

class ByteBuffer {
private:
    int bufferId;
    size_t capacityBytes;
    size_t dataSizeBytes;
    char* rawMemoryData;

public:
    ByteBuffer(int id, size_t capacity);
    ~ByteBuffer();
    int getBufferId() const noexcept;
    size_t getCapacity() const noexcept;
    void writeBytes(const char* data, size_t length);
    void clearBuffer() noexcept;
};

class FileDescriptor {
private:
    int descriptorId;
    int accessModeFlags;
    bool isOpenStatus;
    File* targetFile;
    UserAccount* fileOwner;

public:
    FileDescriptor(int id, int mode, File* file, UserAccount* owner);
    int getDescriptorId() const noexcept;
    void closeDescriptor() noexcept;
    bool isOpen() const noexcept;
    File* getTargetFile() const noexcept;
};

class FileLock {
private:
    int lockId;
    bool isExclusiveLock;
    int holderUserId;
    File* lockedFile;

public:
    FileLock(int id, bool exclusive, int userId, File* file);
    int getLockId() const noexcept;
    bool isExclusive() const noexcept;
    bool matchesHolder(int userId) const noexcept;
};

class FileStream {
private:
    int streamId;
    size_t currentCursorPosition;
    bool isAtEofFlag;
    FileDescriptor* activeDescriptor;
    ByteBuffer* internalIoBuffer;

public:
    FileStream(int id, FileDescriptor* descriptor, ByteBuffer* buffer);
    int getStreamId() const noexcept;
    size_t readStream(char* destinationBuffer, size_t bytesToRead);
    void writeStream(const char* sourceData, size_t bytesToWrite);
    void seekCursor(size_t absoluteOffset);
    bool isEof() const noexcept;
};

class FileCopyTask {
private:
    int taskId;
    size_t totalBytesToCopy;
    size_t copiedBytesCount;
    bool isTaskCompleted;
    File* sourceFile;
    Directory* destinationDirectory;

public:
    FileCopyTask(int id, File* source, Directory* dest);
    int getTaskId() const noexcept;
    void executeCopyStep(size_t chunkBytes);
    bool isCompleted() const noexcept;
    double getProgressPercentage() const noexcept;
};

class PathResolver {
private:
    int resolverId;
    int maxResolutionDepth;
    Directory* rootDirectoryReference;

public:
    PathResolver(int id, int maxDepth, Directory* root);
    int getResolverId() const noexcept;
    FileSystemNode* resolveAbsolutePath(const std::string& absolutePath) const;
    std::string normalizePath(const std::string& rawPath) const;
};

class ChecksumValidator {
private:
    int validatorId;
    std::string hashingAlgorithmName;

public:
    ChecksumValidator(int id, std::string algorithm);
    int getValidatorId() const noexcept;
    std::string calculateFileChecksum(const File* file) const;
    bool verifyChecksum(const File* file, const std::string& expectedChecksum) const;
};

class CompressionCodec {
private:
    int codecId;
    std::string codecName;
    int compressionLevel;

public:
    CompressionCodec(int id, std::string name, int level);
    int getCodecId() const noexcept;
    void compressBuffer(ByteBuffer* sourceBuffer, ByteBuffer* destBuffer) const;
    void decompressBuffer(ByteBuffer* sourceBuffer, ByteBuffer* destBuffer) const;
};