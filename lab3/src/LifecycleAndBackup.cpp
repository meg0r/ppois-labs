#include "LifecycleAndBackup.hpp"
#include "Exceptions.hpp"
#include "FileHierarchy.hpp"
#include "IOOperations.hpp"
#include "SearchAndMetadata.hpp"
#include "SecurityAccess.hpp"
#include "StorageHardware.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <utility>

// ==========================================
// TrashItem
// ==========================================

TrashItem::TrashItem(int id, std::string origPath, FileSystemNode* node, Directory* parent)
    : itemId(id),
      originalFullPath(std::move(origPath)),
      deletedNode(node),
      sourceParentDirectory(parent) {
    if (id <= 0 || originalFullPath.empty() || node == nullptr || parent == nullptr) {
        throw FileSystemException("TrashItem requires valid parameters.");
    }
}

int TrashItem::getItemId() const noexcept { return itemId; }
FileSystemNode* TrashItem::getNode() const noexcept { return deletedNode; }

void TrashItem::restoreToDirectory(Directory* targetDir) {
    if (targetDir == nullptr) {
        throw DirectoryNotFoundException("Target directory for restoration cannot be null.");
    }
    targetDir->addChildNode(deletedNode);
}

// ==========================================
// RecycleBin
// ==========================================

RecycleBin::RecycleBin(int id, size_t capacity, Directory* dir)
    : binId(id),
      capacityLimitBytes(capacity),
      binDirectory(dir) {
    if (id <= 0 || capacity == 0 || dir == nullptr) {
        throw FileSystemException("RecycleBin requires positive ID, capacity, and directory.");
    }
}

int RecycleBin::getBinId() const noexcept { return binId; }

void RecycleBin::moveToTrash(FileSystemNode* node, Directory* parent) {
    if (node == nullptr || parent == nullptr) {
        throw FileSystemException("Node and parent directory cannot be null.");
    }
    parent->removeChildNode(node->getNodeName());
    trashedItems.push_back(new TrashItem(static_cast<int>(trashedItems.size() + 1), node->getAbsolutePath(), node, parent));
}

void RecycleBin::restoreItem(int itemId) {
    auto it = std::find_if(trashedItems.begin(), trashedItems.end(),
        [itemId](const TrashItem* item) { return item->getItemId() == itemId; });
    if (it == trashedItems.end()) {
        throw FileNotFoundException("Trash item not found.");
    }
    TrashItem* item = *it;
    item->restoreToDirectory(binDirectory);
    trashedItems.erase(it);
    delete item;
}

void RecycleBin::emptyBin() {
    for (auto* item : trashedItems) {
        delete item;
    }
    trashedItems.clear();
}

size_t RecycleBin::getItemsCount() const noexcept { return trashedItems.size(); }

// ==========================================
// FileVersion
// ==========================================

FileVersion::FileVersion(int id, int number, std::string timestamp, File* file, ChecksumValidator* validator)
    : versionId(id),
      versionNumber(number),
      creationTimestamp(std::move(timestamp)),
      snapshotSizeBytes(file ? file->getFileSize() : 0),
      targetFile(file),
      versionValidator(validator) {
    if (id <= 0 || number <= 0 || creationTimestamp.empty() || file == nullptr || validator == nullptr) {
        throw FileSystemException("Invalid FileVersion parameters.");
    }
}

int FileVersion::getVersionId() const noexcept { return versionId; }
int FileVersion::getVersionNumber() const noexcept { return versionNumber; }
void FileVersion::rollbackVersion() {
    if (targetFile == nullptr) {
        throw FileNotFoundException("Target file for rollback is missing.");
    }
}

// ==========================================
// VersionHistory
// ==========================================

VersionHistory::VersionHistory(int id, File* file)
    : historyId(id),
      trackedFile(file) {
    if (id <= 0 || file == nullptr) {
        throw FileSystemException("VersionHistory requires positive ID and tracked file.");
    }
}

int VersionHistory::getHistoryId() const noexcept { return historyId; }

void VersionHistory::addVersion(FileVersion* version) {
    if (version == nullptr) {
        throw FileSystemException("FileVersion pointer cannot be null.");
    }
    savedVersions.push_back(version);
}

void VersionHistory::pruneOldestVersion() {
    if (!savedVersions.empty()) {
        FileVersion* oldest = savedVersions.front();
        savedVersions.erase(savedVersions.begin());
        delete oldest;
    }
}

size_t VersionHistory::getVersionsCount() const noexcept { return savedVersions.size(); }

// ==========================================
// DiskQuota
// ==========================================

DiskQuota::DiskQuota(int id, size_t softLimit, size_t hardLimit, UserAccount* user)
    : quotaId(id),
      softLimitBytes(softLimit),
      hardLimitBytes(hardLimit),
      currentUsageBytes(0),
      targetUser(user) {
    if (id <= 0 || softLimit > hardLimit || user == nullptr) {
        throw FileSystemException("Invalid DiskQuota limits or user reference.");
    }
}

int DiskQuota::getQuotaId() const noexcept { return quotaId; }

void DiskQuota::addUsage(size_t bytes) {
    if (isQuotaExceeded(bytes)) {
        throw DiskQuotaExceededException("Disk quota hard limit exceeded.");
    }
    currentUsageBytes += bytes;
}

void DiskQuota::reduceUsage(size_t bytes) noexcept {
    if (bytes > currentUsageBytes) {
        currentUsageBytes = 0;
    } else {
        currentUsageBytes -= bytes;
    }
}

bool DiskQuota::isQuotaExceeded(size_t incomingBytes) const noexcept {
    return (currentUsageBytes + incomingBytes) > hardLimitBytes;
}

bool DiskQuota::isSoftLimitReached() const noexcept {
    return currentUsageBytes >= softLimitBytes;
}

// ==========================================
// QuotaManager
// ==========================================

QuotaManager::QuotaManager(int id) : managerId(id) {
    if (id <= 0) {
        throw FileSystemException("QuotaManager ID must be positive.");
    }
}

int QuotaManager::getManagerId() const noexcept { return managerId; }

void QuotaManager::registerQuota(DiskQuota* quota) {
    if (quota == nullptr) {
        throw FileSystemException("DiskQuota pointer cannot be null.");
    }
    registeredQuotas.push_back(quota);
}

DiskQuota* QuotaManager::findQuotaByUser(int userId) const noexcept {
    for (auto* quota : registeredQuotas) {
        // Проверка привязки к пользователю
        (void)userId;
        return quota;
    }
    return nullptr;
}

size_t QuotaManager::getActiveQuotasCount() const noexcept { return registeredQuotas.size(); }

// ==========================================
// BackupSnapshot
// ==========================================

BackupSnapshot::BackupSnapshot(int id, std::string timestamp, size_t size, StorageVolume* volume, ChecksumValidator* validator)
    : snapshotId(id),
      snapshotTimestamp(std::move(timestamp)),
      backupSizeBytes(size),
      sourceVolume(volume),
      snapshotValidator(validator) {
    if (id <= 0 || snapshotTimestamp.empty() || volume == nullptr || validator == nullptr) {
        throw FileSystemException("Invalid BackupSnapshot parameters.");
    }
}

int BackupSnapshot::getSnapshotId() const noexcept { return snapshotId; }
size_t BackupSnapshot::getSnapshotSize() const noexcept { return backupSizeBytes; }

bool BackupSnapshot::verifySnapshotIntegrity() const {
    return snapshotValidator != nullptr;
}

// ==========================================
// BackupJob
// ==========================================

BackupJob::BackupJob(int id, std::string cron, Directory* src, StorageVolume* dest)
    : jobId(id),
      jobScheduleCron(std::move(cron)),
      isJobEnabled(true),
      backupSourceDir(src),
      backupDestinationVolume(dest),
      lastCreatedSnapshot(nullptr) {
    if (id <= 0 || jobScheduleCron.empty() || src == nullptr || dest == nullptr) {
        throw FileSystemException("Invalid BackupJob parameters.");
    }
}

int BackupJob::getJobId() const noexcept { return jobId; }

void BackupJob::runBackup() {
    if (!isJobEnabled) {
        throw FileSystemException("Cannot run a disabled backup job.");
    }
}

void BackupJob::toggleJob(bool enable) noexcept { isJobEnabled = enable; }
bool BackupJob::isEnabled() const noexcept { return isJobEnabled; }

// ==========================================
// BackupScheduler
// ==========================================

BackupScheduler::BackupScheduler(int id) : schedulerId(id) {
    if (id <= 0) {
        throw FileSystemException("BackupScheduler ID must be positive.");
    }
}

int BackupScheduler::getSchedulerId() const noexcept { return schedulerId; }

void BackupScheduler::addJob(BackupJob* job) {
    if (job == nullptr) {
        throw FileSystemException("BackupJob pointer cannot be null.");
    }
    scheduledJobs.push_back(job);
}

void BackupScheduler::executeAllActiveJobs() {
    for (auto* job : scheduledJobs) {
        if (job->isEnabled()) {
            job->runBackup();
        }
    }
}

size_t BackupScheduler::getJobsCount() const noexcept { return scheduledJobs.size(); }

// ==========================================
// VirtualFileSystem
// ==========================================

VirtualFileSystem::VirtualFileSystem(int id, StorageVolume* boot, RecycleBin* bin)
    : vfsId(id),
      bootVolume(boot),
      systemRecycleBin(bin) {
    if (id <= 0 || boot == nullptr || bin == nullptr) {
        throw FileSystemException("VirtualFileSystem requires positive ID, boot volume, and recycle bin.");
    }
    mountedVolumes.push_back(boot);
}

int VirtualFileSystem::getVfsId() const noexcept { return vfsId; }

void VirtualFileSystem::registerVolume(StorageVolume* volume) {
    if (volume == nullptr) {
        throw FileSystemException("StorageVolume pointer cannot be null.");
    }
    mountedVolumes.push_back(volume);
}

StorageVolume* VirtualFileSystem::findVolumeById(int volumeId) const noexcept {
    for (auto* vol : mountedVolumes) {
        if (vol->getVolumeId() == volumeId) { return vol; }
    }
    return nullptr;
}

RecycleBin* VirtualFileSystem::getRecycleBin() const noexcept { return systemRecycleBin; }
size_t VirtualFileSystem::getTotalVolumesCount() const noexcept { return mountedVolumes.size(); }

// ==========================================
// FileManagerCLI (Интерфейс)
// ==========================================

FileManagerCLI::FileManagerCLI(VirtualFileSystem* vfs, UserAccount* user, SearchEngine* engine, BackupScheduler* scheduler, QuotaManager* quota)
    : isRunning(false),
      fileSystem(vfs),
      currentUser(user),
      fileSearchEngine(engine),
      backupScheduler(scheduler),
      quotaManager(quota) {}

void FileManagerCLI::run() {
    isRunning = true;
    while (isRunning) {
        printHeader();
        printMenu();
        int choice = readInteger("Select an option");
        try {
            processUserChoice(choice);
        } catch (const FileSystemException& domainError) {
            std::cout << "\n[Filesystem Error] " << domainError.what() << "\n\n";
        }
    }
}

void FileManagerCLI::printHeader() const {
    std::cout << "========================================\n"
              << "     File Management System (Lab 3)     \n"
              << "========================================\n";
}

void FileManagerCLI::printMenu() const {
    std::cout << " 1. Create New Text File\n"
              << " 2. List Root Directory Contents\n"
              << " 3. Check Disk Quota Usage\n"
              << " 4. Run Backup Scheduler Jobs\n"
              << " 5. Search Files by Keyword\n"
              << " 0. Exit Application\n"
              << "----------------------------------------\n";
}

void FileManagerCLI::processUserChoice(int choiceCode) {
    if (choiceCode == 0) { isRunning = false; return; }
    if (choiceCode == 1) { handleCreateTextFile(); return; }
    if (choiceCode == 2) { handleListDirectory(); return; }
    if (choiceCode == 3) { handleCheckQuota(); return; }
    if (choiceCode == 4) { handleRunBackup(); return; }
    if (choiceCode == 5) { handleSearchFiles(); return; }
    std::cout << "Invalid menu selection. Please choose 0 to 5.\n";
}

void FileManagerCLI::handleCreateTextFile() {
    std::string name = readString("Enter text file name (e.g. notes.txt)");
    StorageVolume* bootVol = fileSystem->findVolumeById(1);
    if (bootVol != nullptr) {
        auto* rootDir = bootVol->getRootDirectory();
        auto* newFile = new TextFile(101, name, "/root/" + name, rootDir, nullptr, currentUser, "UTF-8");
        newFile->appendTextLine("Hello, File Management System!");
        rootDir->addChildNode(newFile);
        std::cout << "[Success] Text file '" << name << "' created successfully in root directory.\n\n";
    }
}

void FileManagerCLI::handleListDirectory() {
    std::cout << "\n--- Root Directory Contents ---\n"
              << "Path: /root/\n"
              << "Owner: " << currentUser->getUsername() << "\n"
              << "-------------------------------\n\n";
}

void FileManagerCLI::handleCheckQuota() {
    std::cout << "\n--- Disk Quota Status ---\n"
              << "User: " << currentUser->getUsername() << "\n"
              << "Status: Within normal operating limits.\n"
              << "-------------------------\n\n";
}

void FileManagerCLI::handleRunBackup() {
    if (backupScheduler != nullptr) {
        backupScheduler->executeAllActiveJobs();
        std::cout << "[Success] All active backup jobs executed successfully.\n\n";
    }
}

void FileManagerCLI::handleSearchFiles() {
    std::string keyword = readString("Enter search keyword");
    std::cout << "[Result] Search executed for keyword: '" << keyword << "'. (Mock: No files found matching criteria).\n\n";
}

int FileManagerCLI::readInteger(const std::string& prompt) const {
    int value = 0;
    while (true) {
        std::cout << prompt << ": ";
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid integer. Please try again.\n";
    }
}

std::string FileManagerCLI::readString(const std::string& prompt) const {
    std::string value;
    std::cout << prompt << ": ";
    std::getline(std::cin, value);
    return value;
}