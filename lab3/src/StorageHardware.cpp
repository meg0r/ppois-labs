#include "StorageHardware.hpp"
#include "Exceptions.hpp"
#include "FileHierarchy.hpp"
#include "LifecycleAndBackup.hpp"
#include "SecurityAccess.hpp"

#include <algorithm>
#include <utility>

// ==========================================
// StorageBlock
// ==========================================

StorageBlock::StorageBlock(int id, size_t sizeBytes)
    : blockId(id),
      blockSizeBytes(sizeBytes),
      isAllocatedStatus(false),
      rawDataChecksum("") {
    if (id <= 0 || sizeBytes == 0) {
        throw FileSystemException("Storage block ID and size must be strictly positive.");
    }
}

int StorageBlock::getBlockId() const noexcept { return blockId; }

void StorageBlock::allocateBlock(const std::string& checksum) {
    if (isAllocatedStatus) {
        throw OutOfDiskSpaceException("Storage block is already allocated.");
    }
    if (checksum.empty()) {
        throw FileSystemException("Block data checksum cannot be empty.");
    }
    isAllocatedStatus = true;
    rawDataChecksum = checksum;
}

void StorageBlock::freeBlock() noexcept {
    isAllocatedStatus = false;
    rawDataChecksum.clear();
}

bool StorageBlock::isFree() const noexcept { return !isAllocatedStatus; }

bool StorageBlock::operator==(const StorageBlock& other) const noexcept {
    return blockId == other.blockId;
}

// ==========================================
// Inode
// ==========================================

Inode::Inode(int number, int ownerId, std::string createdTime, PermissionMask* mask)
    : inodeNumber(number),
      fileSizeBytes(0),
      hardLinksCount(1),
      creationTimestamp(std::move(createdTime)),
      modificationTimestamp(creationTimestamp),
      ownerUserId(ownerId),
      assignedPermissions(mask) {
    if (number <= 0 || ownerId <= 0) {
        throw FileSystemException("Inode number and owner user ID must be positive.");
    }
    if (creationTimestamp.empty() || mask == nullptr) {
        throw FileSystemException("Inode requires valid timestamp and permission mask.");
    }
}

int Inode::getInodeNumber() const noexcept { return inodeNumber; }
size_t Inode::getFileSize() const noexcept { return fileSizeBytes; }
int Inode::getLinkCount() const noexcept { return hardLinksCount; }

void Inode::attachStorageBlock(StorageBlock* block, size_t addedBytes) {
    if (block == nullptr) {
        throw OutOfDiskSpaceException("Cannot attach a null storage block to inode.");
    }
    if (dataBlocks.size() >= MAX_DIRECT_BLOCKS) {
        throw OutOfDiskSpaceException("Inode maximum direct blocks limit reached.");
    }
    dataBlocks.push_back(block);
    fileSizeBytes += addedBytes;
}

void Inode::incrementLinkCount() noexcept { ++hardLinksCount; }

void Inode::decrementLinkCount() {
    if (hardLinksCount <= 0) {
        throw FileSystemException("Inode hard link count is already zero.");
    }
    --hardLinksCount;
}

void Inode::updateModificationTime(const std::string& timestamp) {
    if (timestamp.empty()) {
        throw FileSystemException("Modification timestamp cannot be empty.");
    }
    modificationTimestamp = timestamp;
}

bool Inode::operator==(const Inode& other) const noexcept {
    return inodeNumber == other.inodeNumber;
}

// ==========================================
// InodeTable
// ==========================================

InodeTable::InodeTable(int id, size_t maxCapacity, Partition* partition)
    : tableId(id),
      maximumInodesCapacity(maxCapacity),
      hostPartition(partition) {
    if (id <= 0 || maxCapacity == 0 || partition == nullptr) {
        throw FileSystemException("InodeTable requires valid ID, capacity, and host partition.");
    }
}

int InodeTable::getTableId() const noexcept { return tableId; }

void InodeTable::registerInode(Inode* inode) {
    if (inode == nullptr) {
        throw FileSystemException("Cannot register a null inode.");
    }
    if (isTableFull()) {
        throw OutOfDiskSpaceException("Inode table capacity exhausted.");
    }
    allocatedInodes.push_back(inode);
}

void InodeTable::releaseInode(int targetInodeNumber) {
    auto it = std::find_if(allocatedInodes.begin(), allocatedInodes.end(),
        [targetInodeNumber](const Inode* node) { return node->getInodeNumber() == targetInodeNumber; });
    if (it == allocatedInodes.end()) {
        throw FileNotFoundException("Target inode not found in inode table.");
    }
    allocatedInodes.erase(it);
}

Inode* InodeTable::findInodeByNumber(int targetInodeNumber) const noexcept {
    for (auto* node : allocatedInodes) {
        if (node->getInodeNumber() == targetInodeNumber) { return node; }
    }
    return nullptr;
}

bool InodeTable::isTableFull() const noexcept {
    return allocatedInodes.size() >= maximumInodesCapacity;
}

bool InodeTable::operator==(const InodeTable& other) const noexcept {
    return tableId == other.tableId;
}

// ==========================================
// SMARTMonitor
// ==========================================

SMARTMonitor::SMARTMonitor(int id, double initialTemp, DiskDrive* drive)
    : monitorId(id),
      temperatureCelsius(initialTemp),
      powerOnHours(0),
      reallocatedSectorsCount(0),
      monitoredDrive(drive) {
    if (id <= 0 || drive == nullptr || initialTemp < 0.0) {
        throw FileSystemException("Invalid SMART monitor initialization parameters.");
    }
}

int SMARTMonitor::getMonitorId() const noexcept { return monitorId; }

void SMARTMonitor::recordOperatingHours(int addedHours) {
    if (addedHours <= 0) {
        throw FileSystemException("Operating hours increment must be positive.");
    }
    powerOnHours += addedHours;
}

void SMARTMonitor::updateTemperature(double currentTemp) {
    if (currentTemp < 0.0) {
        throw FileSystemException("Temperature cannot be negative.");
    }
    temperatureCelsius = currentTemp;
}

void SMARTMonitor::registerBadSector() {
    ++reallocatedSectorsCount;
}

bool SMARTMonitor::isDriveHealthy() const noexcept {
    return (temperatureCelsius < CRITICAL_TEMP_CELSIUS) &&
           (reallocatedSectorsCount <= MAX_SAFE_BAD_SECTORS);
}

// ==========================================
// DiskDrive
// ==========================================

DiskDrive::DiskDrive(int id, std::string serial, std::string model, size_t capacityBytes)
    : driveId(id),
      serialNumber(std::move(serial)),
      vendorModel(std::move(model)),
      totalCapacityBytes(capacityBytes),
      isOnlineStatus(true),
      smartSensor(nullptr) {
    if (id <= 0 || serialNumber.empty() || vendorModel.empty() || capacityBytes == 0) {
        throw FileSystemException("Invalid disk drive parameters.");
    }
}

int DiskDrive::getDriveId() const noexcept { return driveId; }

void DiskDrive::attachSMARTMonitor(SMARTMonitor* monitor) {
    if (monitor == nullptr) {
        throw FileSystemException("SMART monitor pointer cannot be null.");
    }
    smartSensor = monitor;
}

void DiskDrive::addPartition(Partition* partition) {
    if (partition == nullptr) {
        throw FileSystemException("Partition pointer cannot be null.");
    }
    if (diskPartitions.size() >= MAX_PARTITIONS_COUNT) {
        throw OutOfDiskSpaceException("Maximum partition count on drive reached.");
    }
    diskPartitions.push_back(partition);
}

void DiskDrive::spinDownDrive() noexcept { isOnlineStatus = false; }
void DiskDrive::spinUpDrive() noexcept { isOnlineStatus = true; }

bool DiskDrive::isOperational() const noexcept {
    const bool smartOk = (smartSensor == nullptr) || smartSensor->isDriveHealthy();
    return isOnlineStatus && smartOk;
}

bool DiskDrive::operator==(const DiskDrive& other) const noexcept {
    return driveId == other.driveId;
}

// ==========================================
// Partition
// ==========================================

Partition::Partition(int id, std::string label, std::string fsType, size_t sizeBytes, DiskDrive* drive)
    : partitionId(id),
      partitionLabel(std::move(label)),
      fileSystemType(std::move(fsType)),
      partitionSizeBytes(sizeBytes),
      isBootableFlag(false),
      parentDrive(drive),
      partitionInodeTable(nullptr) {
    if (id <= 0 || partitionLabel.empty() || sizeBytes == 0 || drive == nullptr) {
        throw FileSystemException("Invalid partition parameters.");
    }
}

int Partition::getPartitionId() const noexcept { return partitionId; }
size_t Partition::getPartitionSize() const noexcept { return partitionSizeBytes; }

void Partition::formatPartition(const std::string& newFsType, InodeTable* newTable) {
    if (newFsType.empty() || newTable == nullptr) {
        throw FileSystemException("Formatting requires valid filesystem type and inode table.");
    }
    fileSystemType = newFsType;
    partitionInodeTable = newTable;
}

void Partition::markAsBootable() noexcept { isBootableFlag = true; }

bool Partition::isFormatted() const noexcept {
    return !fileSystemType.empty() && (partitionInodeTable != nullptr);
}

bool Partition::operator==(const Partition& other) const noexcept {
    return partitionId == other.partitionId;
}

// ==========================================
// BlockAllocator
// ==========================================

BlockAllocator::BlockAllocator(int id, size_t totalBlocks, Partition* partition)
    : allocatorId(id),
      totalBlocksManaged(totalBlocks),
      allocatedBlocksCount(0),
      targetPartition(partition) {
    if (id <= 0 || totalBlocks == 0 || partition == nullptr) {
        throw FileSystemException("BlockAllocator requires valid ID, block count, and partition.");
    }
}

int BlockAllocator::getAllocatorId() const noexcept { return allocatorId; }

void BlockAllocator::registerBlockInPool(StorageBlock* block) {
    if (block == nullptr) {
        throw FileSystemException("Block pointer cannot be null.");
    }
    blockPool.push_back(block);
}

StorageBlock* BlockAllocator::requestFreeBlock(const std::string& checksum, const DiskQuota* quota) {
    if (quota != nullptr && quota->isQuotaExceeded(StorageBlock::DEFAULT_BLOCK_SIZE)) {
        throw DiskQuotaExceededException("User disk quota exceeded during block allocation.");
    }
    for (auto* block : blockPool) {
        if (block->isFree()) {
            block->allocateBlock(checksum);
            ++allocatedBlocksCount;
            return block;
        }
    }
    throw OutOfDiskSpaceException("No free storage blocks available in allocator pool.");
}

void BlockAllocator::reclaimBlock(int blockId) {
    for (auto* block : blockPool) {
        if (block->getBlockId() == blockId && !block->isFree()) {
            block->freeBlock();
            --allocatedBlocksCount;
            return;
        }
    }
    throw FileNotFoundException("Allocated block ID not found in pool.");
}

double BlockAllocator::calculateFragmentationRatio() const noexcept {
    if (totalBlocksManaged == 0) { return 0.0; }
    return static_cast<double>(allocatedBlocksCount) / static_cast<double>(totalBlocksManaged);
}

// ==========================================
// StorageVolume
// ==========================================

StorageVolume::StorageVolume(int id, std::string uuid, std::string name, Partition* partition, Directory* root)
    : volumeId(id),
      volumeUUID(std::move(uuid)),
      volumeName(std::move(name)),
      totalSpaceBytes(partition ? partition->getPartitionSize() : 0),
      usedSpaceBytes(0),
      isReadOnlyVolume(false),
      backingPartition(partition),
      rootDirectory(root),
      volumeAllocator(nullptr) {
    if (id <= 0 || volumeUUID.empty() || volumeName.empty() || partition == nullptr || root == nullptr) {
        throw FileSystemException("StorageVolume requires valid partition and root directory.");
    }
}

int StorageVolume::getVolumeId() const noexcept { return volumeId; }

size_t StorageVolume::getAvailableSpace() const noexcept {
    return (totalSpaceBytes > usedSpaceBytes) ? (totalSpaceBytes - usedSpaceBytes) : 0;
}

Directory* StorageVolume::getRootDirectory() const noexcept { return rootDirectory; }

void StorageVolume::allocateSpace(size_t bytesToAllocate) {
    if (isReadOnlyVolume) {
        throw PermissionDeniedException("Cannot allocate space on a read-only storage volume.");
    }
    if (bytesToAllocate > getAvailableSpace()) {
        throw OutOfDiskSpaceException("Insufficient free space on storage volume.");
    }
    usedSpaceBytes += bytesToAllocate;
}

void StorageVolume::releaseSpace(size_t bytesToRelease) {
    if (bytesToRelease > usedSpaceBytes) {
        usedSpaceBytes = 0;
    } else {
        usedSpaceBytes -= bytesToRelease;
    }
}

void StorageVolume::bindAllocator(BlockAllocator* allocator) {
    if (allocator == nullptr) {
        throw FileSystemException("BlockAllocator pointer cannot be null.");
    }
    volumeAllocator = allocator;
}

void StorageVolume::setReadOnlyMode(bool readOnly) noexcept { isReadOnlyVolume = readOnly; }
bool StorageVolume::isReadOnly() const noexcept { return isReadOnlyVolume; }

bool StorageVolume::operator==(const StorageVolume& other) const noexcept {
    return volumeId == other.volumeId;
}

// ==========================================
// MountManager
// ==========================================

MountManager::MountManager(int id, int maxMounts)
    : managerId(id),
      maxMountPointsLimit(maxMounts) {
    if (id <= 0 || maxMounts <= 0) {
        throw FileSystemException("MountManager requires positive ID and mount limit.");
    }
}

int MountManager::getManagerId() const noexcept { return managerId; }

void MountManager::mountVolume(StorageVolume* volume, const std::string& mountPath) {
    if (volume == nullptr || mountPath.empty() || mountPath[0] != '/') {
        throw InvalidPathException("Mount requires a valid volume and an absolute mount path.");
    }
    if (static_cast<int>(mountedVolumes.size()) >= maxMountPointsLimit) {
        throw FileSystemException("Maximum active mount points limit reached.");
    }
    if (isVolumeMounted(volume->getVolumeId())) {
        throw FileAlreadyExistsException("Storage volume is already mounted.");
    }
    mountedVolumes.push_back(volume);
    activeMountPaths.push_back(mountPath);
}

void MountManager::unmountVolume(int volumeId) {
    for (size_t i = 0; i < mountedVolumes.size(); ++i) {
        if (mountedVolumes[i]->getVolumeId() == volumeId) {
            mountedVolumes.erase(mountedVolumes.begin() + static_cast<std::ptrdiff_t>(i));
            activeMountPaths.erase(activeMountPaths.begin() + static_cast<std::ptrdiff_t>(i));
            return;
        }
    }
    throw FileNotFoundException("Target storage volume is not currently mounted.");
}

bool MountManager::isVolumeMounted(int volumeId) const noexcept {
    return std::any_of(mountedVolumes.begin(), mountedVolumes.end(),
        [volumeId](const StorageVolume* v) { return v->getVolumeId() == volumeId; });
}

size_t MountManager::getMountedCount() const noexcept { return mountedVolumes.size(); }