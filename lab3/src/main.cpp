#include "Exceptions.hpp"
#include "FileHierarchy.hpp"
#include "IOOperations.hpp"
#include "LifecycleAndBackup.hpp"
#include "SearchAndMetadata.hpp"
#include "SecurityAccess.hpp"
#include "StorageHardware.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main() {
    try {
        // 1. Инициализация базовых сущностей
        auto* rootPerms = new PermissionMask(755, true, true, true);
        auto* adminUser = new AdminUser(1, "root", "sha256_admin_secure_hash", 10);
        auto* rootInode = new Inode(1, 1, "2026-10-26", rootPerms);
        auto* rootDir = new Directory(1, "root", "/root/", nullptr, rootInode, adminUser);
        
        auto* disk = new DiskDrive(1, "SN-998822", "NVMe SSD 2TB", 2000000000ULL);
        auto* part = new Partition(1, "SystemPartition", "ext4", 1000000000ULL, disk);
        auto* volume = new StorageVolume(1, "UUID-ROOT-VOL", "BootVolume", part, rootDir);

        auto* trashDir = new Directory(2, ".Trash", "/root/.Trash/", rootDir, nullptr, adminUser);
        auto* recycleBin = new RecycleBin(1, 50000000ULL, trashDir);

        auto* vfs = new VirtualFileSystem(1, volume, recycleBin);
        auto* searchIndex = new SearchIndex(1, "v1.0");
        auto* searchEngine = new SearchEngine(1, searchIndex);
        auto* backupScheduler = new BackupScheduler(1);
        auto* quotaManager = new QuotaManager(1);

        // 2. Запуск консольного интерфейса
        FileManagerCLI cliApp(vfs, adminUser, searchEngine, backupScheduler, quotaManager);
        cliApp.run();

    } catch (const FileSystemException& domainEx) {
        std::cerr << "Filesystem Domain Error: " << domainEx.what() << '\n';
        return EXIT_FAILURE;
    } catch (const std::exception& stdEx) {
        std::cerr << "Unexpected System Failure: " << stdEx.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}