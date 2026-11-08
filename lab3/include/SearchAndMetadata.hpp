#pragma once

#include <string>
#include <vector>

class FileSystemNode;
class UserAccount;
class File;
class Directory;

class FileTag {
private:
    int tagId;
    std::string tagName;
    std::string tagColorCode;

public:
    FileTag(int id, std::string name, std::string color);
    int getTagId() const noexcept;
    const std::string& getTagName() const noexcept;
    bool operator==(const FileTag& other) const noexcept;
};

class FileMetadata {
private:
    int metadataId;
    std::string authorName;
    std::string mimeType;
    int accessCount;
    std::vector<FileTag*> attachedTags;

public:
    FileMetadata(int id, std::string author, std::string mime);
    int getMetadataId() const noexcept;
    void addTag(FileTag* tag);
    void incrementAccessCount() noexcept;
    int getAccessCount() const noexcept;
};

class SearchQuery {
private:
    int queryId;
    std::string searchKeyword;
    FileTag* requiredTag;
    size_t minSizeBytes;

public:
    SearchQuery(int id, std::string keyword, FileTag* tag, size_t minSize);
    int getQueryId() const noexcept;
    bool matchesNode(const FileSystemNode* node) const noexcept;
};

class SearchIndex {
private:
    int indexId;
    std::string indexVersion;
    std::vector<File*> indexedFiles;

public:
    SearchIndex(int id, std::string version);
    int getIndexId() const noexcept;
    void indexFile(File* file);
    void removeIndexedFile(int fileId);
    size_t getIndexedCount() const noexcept;
};

class FileIndexer {
private:
    int indexerId;
    SearchIndex* targetIndex;

public:
    FileIndexer(int id, SearchIndex* index);
    int getIndexerId() const noexcept;
    void indexDirectoryRecursively(Directory* dir);
};

class SearchEngine {
private:
    int engineId;
    SearchIndex* searchableIndex;

public:
    SearchEngine(int id, SearchIndex* index);
    int getEngineId() const noexcept;
    std::vector<File*> executeQuery(const SearchQuery& query) const;
};

class DuplicateFinder {
private:
    int finderId;
    Directory* targetDirectory;

public:
    DuplicateFinder(int id, Directory* dir);
    int getFinderId() const noexcept;
    std::vector<File*> findDuplicates() const;
};

class FileEvent {
private:
    int eventId;
    std::string eventTypeName;
    std::string timestamp;
    FileSystemNode* affectedNode;
    UserAccount* initiatingUser;

public:
    FileEvent(int id, std::string type, std::string time, FileSystemNode* node, UserAccount* user);
    int getEventId() const noexcept;
    const std::string& getEventType() const noexcept;
};

class AuditEvent {
private:
    int auditEventId;
    std::string actionDescription;
    std::string severityLevel;
    UserAccount* actorUser;
    FileSystemNode* targetResource;

public:
    AuditEvent(int id, std::string action, std::string severity, UserAccount* user, FileSystemNode* resource);
    int getAuditEventId() const noexcept;
    const std::string& getSeverity() const noexcept;
};

class AuditLogger {
private:
    int loggerId;
    std::vector<AuditEvent*> recordedEvents;

public:
    AuditLogger(int id);
    int getLoggerId() const noexcept;
    void logEvent(AuditEvent* event);
    size_t getEventsCount() const noexcept;
};

class DirectoryWatcher {
private:
    int watcherId;
    Directory* watchedDirectory;
    bool isMonitoringActive;

public:
    DirectoryWatcher(int id, Directory* dir);
    int getWatcherId() const noexcept;
    void startMonitoring() noexcept;
    void stopMonitoring() noexcept;
    bool isMonitoring() const noexcept;
};