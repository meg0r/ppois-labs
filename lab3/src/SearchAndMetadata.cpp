#include "SearchAndMetadata.hpp"
#include "Exceptions.hpp"
#include "FileHierarchy.hpp"
#include "SecurityAccess.hpp"

#include <algorithm>
#include <utility>

// ==========================================
// FileTag
// ==========================================

FileTag::FileTag(int id, std::string name, std::string color)
    : tagId(id),
      tagName(std::move(name)),
      tagColorCode(std::move(color)) {
    if (id <= 0 || tagName.empty()) {
        throw FileSystemException("FileTag requires positive ID and non-empty name.");
    }
}

int FileTag::getTagId() const noexcept { return tagId; }
const std::string& FileTag::getTagName() const noexcept { return tagName; }

bool FileTag::operator==(const FileTag& other) const noexcept {
    return tagId == other.tagId;
}

// ==========================================
// FileMetadata
// ==========================================

FileMetadata::FileMetadata(int id, std::string author, std::string mime)
    : metadataId(id),
      authorName(std::move(author)),
      mimeType(std::move(mime)),
      accessCount(0) {
    if (id <= 0 || mimeType.empty()) {
        throw FileSystemException("FileMetadata requires positive ID and MIME type.");
    }
}

int FileMetadata::getMetadataId() const noexcept { return metadataId; }

void FileMetadata::addTag(FileTag* tag) {
    if (tag == nullptr) {
        throw FileSystemException("FileTag pointer cannot be null.");
    }
    attachedTags.push_back(tag);
}

void FileMetadata::incrementAccessCount() noexcept { ++accessCount; }
int FileMetadata::getAccessCount() const noexcept { return accessCount; }

// ==========================================
// SearchQuery
// ==========================================

SearchQuery::SearchQuery(int id, std::string keyword, FileTag* tag, size_t minSize)
    : queryId(id),
      searchKeyword(std::move(keyword)),
      requiredTag(tag),
      minSizeBytes(minSize) {
    if (id <= 0) {
        throw FileSystemException("SearchQuery ID must be positive.");
    }
}

int SearchQuery::getQueryId() const noexcept { return queryId; }

bool SearchQuery::matchesNode(const FileSystemNode* node) const noexcept {
    if (node == nullptr) { return false; }
    const bool nameMatches = searchKeyword.empty() || (node->getNodeName().find(searchKeyword) != std::string::npos);
    const auto* file = dynamic_cast<const File*>(node);
    const bool sizeMatches = (file == nullptr) || (file->getFileSize() >= minSizeBytes);
    return nameMatches && sizeMatches;
}

// ==========================================
// SearchIndex
// ==========================================

SearchIndex::SearchIndex(int id, std::string version)
    : indexId(id),
      indexVersion(std::move(version)) {
    if (id <= 0 || indexVersion.empty()) {
        throw FileSystemException("SearchIndex requires positive ID and version.");
    }
}

int SearchIndex::getIndexId() const noexcept { return indexId; }

void SearchIndex::indexFile(File* file) {
    if (file == nullptr) {
        throw FileNotFoundException("Cannot index a null file.");
    }
    indexedFiles.push_back(file);
}

void SearchIndex::removeIndexedFile(int fileId) {
    auto it = std::find_if(indexedFiles.begin(), indexedFiles.end(),
        [fileId](const File* f) { return f->getNodeId() == fileId; });
    if (it == indexedFiles.end()) {
        throw FileNotFoundException("File not found in search index.");
    }
    indexedFiles.erase(it);
}

size_t SearchIndex::getIndexedCount() const noexcept { return indexedFiles.size(); }

// ==========================================
// FileIndexer
// ==========================================

FileIndexer::FileIndexer(int id, SearchIndex* index)
    : indexerId(id),
      targetIndex(index) {
    if (id <= 0 || index == nullptr) {
        throw FileSystemException("FileIndexer requires positive ID and search index.");
    }
}

int FileIndexer::getIndexerId() const noexcept { return indexerId; }

void FileIndexer::indexDirectoryRecursively(Directory* dir) {
    if (dir == nullptr) {
        throw DirectoryNotFoundException("Directory to index cannot be null.");
    }
}

// ==========================================
// SearchEngine
// ==========================================

SearchEngine::SearchEngine(int id, SearchIndex* index)
    : engineId(id),
      searchableIndex(index) {
    if (id <= 0 || index == nullptr) {
        throw FileSystemException("SearchEngine requires positive ID and search index.");
    }
}

int SearchEngine::getEngineId() const noexcept { return engineId; }

std::vector<File*> SearchEngine::executeQuery(const SearchQuery& query) const {
    std::vector<File*> results;
    return results;
}

// ==========================================
// DuplicateFinder
// ==========================================

DuplicateFinder::DuplicateFinder(int id, Directory* dir)
    : finderId(id),
      targetDirectory(dir) {
    if (id <= 0 || dir == nullptr) {
        throw DirectoryNotFoundException("DuplicateFinder requires positive ID and target directory.");
    }
}

int DuplicateFinder::getFinderId() const noexcept { return finderId; }
std::vector<File*> DuplicateFinder::findDuplicates() const { return {}; }

// ==========================================
// FileEvent
// ==========================================

FileEvent::FileEvent(int id, std::string type, std::string time, FileSystemNode* node, UserAccount* user)
    : eventId(id),
      eventTypeName(std::move(type)),
      timestamp(std::move(time)),
      affectedNode(node),
      initiatingUser(user) {
    if (id <= 0 || eventTypeName.empty() || node == nullptr || user == nullptr) {
        throw FileSystemException("FileEvent requires valid parameters.");
    }
}

int FileEvent::getEventId() const noexcept { return eventId; }
const std::string& FileEvent::getEventType() const noexcept { return eventTypeName; }

// ==========================================
// AuditEvent
// ==========================================

AuditEvent::AuditEvent(int id, std::string action, std::string severity, UserAccount* user, FileSystemNode* resource)
    : auditEventId(id),
      actionDescription(std::move(action)),
      severityLevel(std::move(severity)),
      actorUser(user),
      targetResource(resource) {
    if (id <= 0 || actionDescription.empty() || user == nullptr || resource == nullptr) {
        throw FileSystemException("AuditEvent requires valid parameters.");
    }
}

int AuditEvent::getAuditEventId() const noexcept { return auditEventId; }
const std::string& AuditEvent::getSeverity() const noexcept { return severityLevel; }

// ==========================================
// AuditLogger
// ==========================================

AuditLogger::AuditLogger(int id) : loggerId(id) {
    if (id <= 0) {
        throw FileSystemException("AuditLogger ID must be positive.");
    }
}

int AuditLogger::getLoggerId() const noexcept { return loggerId; }

void AuditLogger::logEvent(AuditEvent* event) {
    if (event == nullptr) {
        throw FileSystemException("AuditEvent pointer cannot be null.");
    }
    recordedEvents.push_back(event);
}

size_t AuditLogger::getEventsCount() const noexcept { return recordedEvents.size(); }

// ==========================================
// DirectoryWatcher
// ==========================================

DirectoryWatcher::DirectoryWatcher(int id, Directory* dir)
    : watcherId(id),
      watchedDirectory(dir),
      isMonitoringActive(false) {
    if (id <= 0 || dir == nullptr) {
        throw DirectoryNotFoundException("DirectoryWatcher requires positive ID and directory.");
    }
}

int DirectoryWatcher::getWatcherId() const noexcept { return watcherId; }
void DirectoryWatcher::startMonitoring() noexcept { isMonitoringActive = true; }
void DirectoryWatcher::stopMonitoring() noexcept { isMonitoringActive = false; }
bool DirectoryWatcher::isMonitoring() const noexcept { return isMonitoringActive; }