#pragma once

#include <string>
#include <vector>

class FileSystemNode;
class Directory;
class File;
class UserAccount;
class ChecksumValidator;
class StorageVolume;
class Partition;
class InodeTable;
class SearchEngine;
class BackupScheduler;
class AccessControlList;
class RecycleBin;

class TrashItem {
private:
    int itemId;
    std::string originalFullPath;
    FileSystemNode* deletedNode;
    Directory* sourceParentDirectory;

public:
    TrashItem(int id, std::string origPath, FileSystemNode* node, Directory* parent);
    int getItemId() const noexcept;
    FileSystemNode* getNode() const noexcept;
    void restoreToDirectory(Directory* targetDir);
};

class RecycleBin {
private:
    int binId;
    size_t capacityLimitBytes;
    Directory* binDirectory;
    std::vector<TrashItem*> trashedItems;

public:
    RecycleBin(int id, size_t capacity, Directory* dir);
    int getBinId() const noexcept;
    void moveToTrash(FileSystemNode* node, Directory* parent);
    void restoreItem(int itemId);
    void emptyBin();
    size_t getItemsCount() const noexcept;
};

class FileVersion {
private:
    int versionId;
    int versionNumber;
    std::string creationTimestamp;
    size_t snapshotSizeBytes;
    File* targetFile;
    ChecksumValidator* versionValidator;

public:
    FileVersion(int id, int number, std::string timestamp, File* file, ChecksumValidator* validator);
    int getVersionId() const noexcept;
    int getVersionNumber() const noexcept;
    void rollbackVersion();
};

class VersionHistory {
private:
    int historyId;
    File* trackedFile;
    std::vector<FileVersion*> savedVersions;

public:
    VersionHistory(int id, File* file);
    int getHistoryId() const noexcept;
    void addVersion(FileVersion* version);
    void pruneOldestVersion();
    size_t getVersionsCount() const noexcept;
};

class DiskQuota {
private:
    int quotaId;
    size_t softLimitBytes;
    size_t hardLimitBytes;
    size_t currentUsageBytes;
    UserAccount* targetUser;

public:
    DiskQuota(int id, size_t softLimit, size_t hardLimit, UserAccount* user);
    int getQuotaId() const noexcept;
    void addUsage(size_t bytes);
    void reduceUsage(size_t bytes) noexcept;
    bool isQuotaExceeded(size_t incomingBytes) const noexcept;
    bool isSoftLimitReached() const noexcept;
};

class QuotaManager {
private:
    int managerId;
    std::vector<DiskQuota*> registeredQuotas;

public:
    QuotaManager(int id);
    int getManagerId() const noexcept;
    void registerQuota(DiskQuota* quota);
    DiskQuota* findQuotaByUser(int userId) const noexcept;
    size_t getActiveQuotasCount() const noexcept;
};

class BackupSnapshot {
private:
    int snapshotId;
    std::string snapshotTimestamp;
    size_t backupSizeBytes;
    StorageVolume* sourceVolume;
    ChecksumValidator* snapshotValidator;

public:
    BackupSnapshot(int id, std::string timestamp, size_t size, StorageVolume* volume, ChecksumValidator* validator);
    int getSnapshotId() const noexcept;
    size_t getSnapshotSize() const noexcept;
    bool verifySnapshotIntegrity() const;
};

class BackupJob {
private:
    int jobId;
    std::string jobScheduleCron;
    bool isJobEnabled;
    Directory* backupSourceDir;
    StorageVolume* backupDestinationVolume;
    BackupSnapshot* lastCreatedSnapshot;

public:
    BackupJob(int id, std::string cron, Directory* src, StorageVolume* dest);
    int getJobId() const noexcept;
    void runBackup();
    void toggleJob(bool enable) noexcept;
    bool isEnabled() const noexcept;
};

class BackupScheduler {
private:
    int schedulerId;
    std::vector<BackupJob*> scheduledJobs;

public:
    BackupScheduler(int id);
    int getSchedulerId() const noexcept;
    void addJob(BackupJob* job);
    void executeAllActiveJobs();
    size_t getJobsCount() const noexcept;
};

class VirtualFileSystem {
private:
    int vfsId;
    std::vector<StorageVolume*> mountedVolumes;
    StorageVolume* bootVolume;
    RecycleBin* systemRecycleBin;

public:
    VirtualFileSystem(int id, StorageVolume* boot, RecycleBin* bin);
    int getVfsId() const noexcept;
    void registerVolume(StorageVolume* volume);
    StorageVolume* findVolumeById(int volumeId) const noexcept;
    RecycleBin* getRecycleBin() const noexcept;
    size_t getTotalVolumesCount() const noexcept;
};

class FileManagerCLI {
private:
    bool isRunning;
    VirtualFileSystem* fileSystem;
    UserAccount* currentUser;
    SearchEngine* fileSearchEngine;
    BackupScheduler* backupScheduler;
    QuotaManager* quotaManager;

public:
    FileManagerCLI(VirtualFileSystem* vfs, UserAccount* user, SearchEngine* engine, BackupScheduler* scheduler, QuotaManager* quota);
    void run();
    void printHeader() const;
    void printMenu() const;
    void processUserChoice(int choiceCode);
    void handleCreateTextFile();
    void handleListDirectory();
    void handleCheckQuota();
    void handleRunBackup();
    void handleSearchFiles();
    int readInteger(const std::string& prompt) const;
    std::string readString(const std::string& prompt) const;
};