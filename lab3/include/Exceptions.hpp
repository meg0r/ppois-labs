#pragma once

#include <exception>
#include <string>
#include <utility>

class FileSystemException : public std::exception {
private:
    std::string errorMessage;

public:
    explicit FileSystemException(std::string message)
        : errorMessage(std::move(message)) {}

    const char* what() const noexcept override {
        return errorMessage.c_str();
    }
};

class FileNotFoundException : public FileSystemException {
public:
    explicit FileNotFoundException(std::string msg) : FileSystemException(std::move(msg)) {}
};

class DirectoryNotFoundException : public FileSystemException {
public:
    explicit DirectoryNotFoundException(std::string msg) : FileSystemException(std::move(msg)) {}
};

class DirectoryNotEmptyException : public FileSystemException {
public:
    explicit DirectoryNotEmptyException(std::string msg) : FileSystemException(std::move(msg)) {}
};

class FileAlreadyExistsException : public FileSystemException {
public:
    explicit FileAlreadyExistsException(std::string msg) : FileSystemException(std::move(msg)) {}
};

class PermissionDeniedException : public FileSystemException {
public:
    explicit PermissionDeniedException(std::string msg) : FileSystemException(std::move(msg)) {}
};

class DiskQuotaExceededException : public FileSystemException {
public:
    explicit DiskQuotaExceededException(std::string msg) : FileSystemException(std::move(msg)) {}
};

class OutOfDiskSpaceException : public FileSystemException {
public:
    explicit OutOfDiskSpaceException(std::string msg) : FileSystemException(std::move(msg)) {}
};

class FileLockedException : public FileSystemException {
public:
    explicit FileLockedException(std::string msg) : FileSystemException(std::move(msg)) {}
};

class CorruptedFileException : public FileSystemException {
public:
    explicit CorruptedFileException(std::string msg) : FileSystemException(std::move(msg)) {}
};

class InvalidPathException : public FileSystemException {
public:
    explicit InvalidPathException(std::string msg) : FileSystemException(std::move(msg)) {}
};

class CyclicSymlinkException : public FileSystemException {
public:
    explicit CyclicSymlinkException(std::string msg) : FileSystemException(std::move(msg)) {}
};

class AuthenticationFailedException : public FileSystemException {
public:
    explicit AuthenticationFailedException(std::string msg) : FileSystemException(std::move(msg)) {}
};