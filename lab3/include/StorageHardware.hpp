#pragma once

#include <string>
#include <vector>

class SMARTMonitor;
class Partition;
class StorageBlock;
class Inode;
class InodeTable;
class BlockAllocator;
class StorageVolume;
class MountManager;
class Directory;
class PermissionMask;
class DiskQuota;
class DiskDrive;

class StorageBlock {
private:
    int blockId;
    size_t blockSizeBytes;
    bool isAllocatedStatus;
    std::string rawDataChecksum;

public:
    static constexpr size_t DEFAULT_BLOCK_SIZE = 4096;

    StorageBlock(int id, size_t sizeBytes);
    int getBlockId() const noexcept;
    void allocateBlock(const std::string& checksum);
    void freeBlock() noexcept;
    bool isFree() const noexcept;
    bool operator==(const StorageBlock& other) const noexcept;
};

class Inode {
private:
    int inodeNumber;
    size_t fileSizeBytes;
    int hardLinksCount;
    std::string creationTimestamp;
    std::string modificationTimestamp;
    int ownerUserId;
    PermissionMask* assignedPermissions;
    std::vector<StorageBlock*> dataBlocks;

public:
    static constexpr size_t MAX_DIRECT_BLOCKS = 128;

    Inode(int number, int ownerId, std::string createdTime, PermissionMask* mask);
    int getInodeNumber() const noexcept;
    size_t getFileSize() const noexcept;
    void attachStorageBlock(StorageBlock* block, size_t addedBytes);
    void incrementLinkCount() noexcept;
    void decrementLinkCount();
    int getLinkCount() const noexcept;
    void updateModificationTime(const std::string& timestamp);
    bool operator==(const Inode& other) const noexcept;
};

class InodeTable {
private:
    int tableId;
    size_t maximumInodesCapacity;
    Partition* hostPartition;
    std::vector<Inode*> allocatedInodes;

public:
    InodeTable(int id, size_t maxCapacity, Partition* partition);
    int getTableId() const noexcept;
    void registerInode(Inode* inode);
    void releaseInode(int targetInodeNumber);
    Inode* findInodeByNumber(int targetInodeNumber) const noexcept;
    bool isTableFull() const noexcept;
    bool operator==(const InodeTable& other) const noexcept;
};

class SMARTMonitor {
private:
    int monitorId;
    double temperatureCelsius;
    int powerOnHours;
    int reallocatedSectorsCount;
    DiskDrive* monitoredDrive;

public:
    static constexpr double CRITICAL_TEMP_CELSIUS = 70.0;
    static constexpr int MAX_SAFE_BAD_SECTORS = 10;

    SMARTMonitor(int id, double initialTemp, DiskDrive* drive);
    int getMonitorId() const noexcept;
    void recordOperatingHours(int addedHours);
    void updateTemperature(double currentTemp);
    void registerBadSector();
    bool isDriveHealthy() const noexcept;
};

class DiskDrive {
private:
    int driveId;
    std::string serialNumber;
    std::string vendorModel;
    size_t totalCapacityBytes;
    bool isOnlineStatus;
    SMARTMonitor* smartSensor;
    std::vector<Partition*> diskPartitions;

public:
    static constexpr size_t MAX_PARTITIONS_COUNT = 16;

    DiskDrive(int id, std::string serial, std::string model, size_t capacityBytes);
    int getDriveId() const noexcept;
    void attachSMARTMonitor(SMARTMonitor* monitor);
    void addPartition(Partition* partition);
    void spinDownDrive() noexcept;
    void spinUpDrive() noexcept;
    bool isOperational() const noexcept;
    bool operator==(const DiskDrive& other) const noexcept;
};

class Partition {
private:
    int partitionId;
    std::string partitionLabel;
    std::string fileSystemType;
    size_t partitionSizeBytes;
    bool isBootableFlag;
    DiskDrive* parentDrive;
    InodeTable* partitionInodeTable;

public:
    Partition(int id, std::string label, std::string fsType, size_t sizeBytes, DiskDrive* drive);
    int getPartitionId() const noexcept;
    size_t getPartitionSize() const noexcept;
    void formatPartition(const std::string& newFsType, InodeTable* newTable);
    void markAsBootable() noexcept;
    bool isFormatted() const noexcept;
    bool operator==(const Partition& other) const noexcept;
};

class BlockAllocator {
private:
    int allocatorId;
    size_t totalBlocksManaged;
    size_t allocatedBlocksCount;
    Partition* targetPartition;
    std::vector<StorageBlock*> blockPool;

public:
    BlockAllocator(int id, size_t totalBlocks, Partition* partition);
    int getAllocatorId() const noexcept;
    void registerBlockInPool(StorageBlock* block);
    StorageBlock* requestFreeBlock(const std::string& checksum, const DiskQuota* quota);
    void reclaimBlock(int blockId);
    double calculateFragmentationRatio() const noexcept;
};

class StorageVolume {
private:
    int volumeId;
    std::string volumeUUID;
    std::string volumeName;
    size_t totalSpaceBytes;
    size_t usedSpaceBytes;
    bool isReadOnlyVolume;
    Partition* backingPartition;
    Directory* rootDirectory;
    BlockAllocator* volumeAllocator;

public:
    StorageVolume(int id, std::string uuid, std::string name, Partition* partition, Directory* root);
    int getVolumeId() const noexcept;
    size_t getAvailableSpace() const noexcept;
    Directory* getRootDirectory() const noexcept;
    void allocateSpace(size_t bytesToAllocate);
    void releaseSpace(size_t bytesToRelease);
    void bindAllocator(BlockAllocator* allocator);
    void setReadOnlyMode(bool readOnly) noexcept;
    bool isReadOnly() const noexcept;
    bool operator==(const StorageVolume& other) const noexcept;
};

class MountManager {
private:
    int managerId;
    int maxMountPointsLimit;
    std::vector<StorageVolume*> mountedVolumes;
    std::vector<std::string> activeMountPaths;

public:
    MountManager(int id, int maxMounts);
    int getManagerId() const noexcept;
    void mountVolume(StorageVolume* volume, const std::string& mountPath);
    void unmountVolume(int volumeId);
    bool isVolumeMounted(int volumeId) const noexcept;
    size_t getMountedCount() const noexcept;
};