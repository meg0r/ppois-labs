# Лабораторная работа №3. Система управления файлами (File Management System)

## Описание предметной области

Объектная модель реализует архитектуру современной системы управления файлами и корпоративного файлового хранилища. Система охватывает физический и логический уровни хранения данных (диски, разделы, тома, индексные дескрипторы `Inode`), иерархию файловых объектов (директории, текстовые, бинарные, медиа-, архивные и исполняемые файлы, символьные и жёсткие ссылки), подсистему контроля доступа и криптографии (пользователи, группы, права POSIX/ACL, токены, шифрование), операции ввода-вывода и буферизации, полнотекстовую индексацию и поиск, а также версионирование, резервное копирование и квотирование.

Формат строки: `Класс | число полей | число методов | связанные классы`

| Класс | Поля | Методы | Ассоциации (связанные классы) |
| :--- | :--- | :--- | :--- |
| AccessControlEntry | 4 | 4 | UserAccount, UserGroup, PermissionMask |
| AccessControlList | 3 | 5 | AccessControlEntry, UserAccount, FileSystemNode |
|AccessToken | 5 | 4 | UserAccount, AuthSession |
| AdminUser | 4 | 5 | UserAccount, StorageVolume, QuotaManager, AuditLogger |
| ArchiveFile | 6 | 5 | File, CompressionCodec, FileSystemNode |
| AuditEvent | 6 | 3 | UserAccount, FileSystemNode |
| AuditLogger | 4 | 5 | AuditEvent, LogFile |
| AuthSession | 6 | 4 | UserAccount, AccessToken |
| BackupJob | 7 | 6 | Directory, StorageVolume, BackupSnapshot,BackupScheduler |
| BackupScheduler | 4 | 4 | BackupJob |
| BackupSnapshot | 6 | 4 | StorageVolume, ChecksumValidator |
| BinaryFile | 5 | 4 | File, ByteBuffer |
| BlockAllocator | 5 | 5 | StorageBlock, Partition, DiskQuota |
| ByteBuffer | 4 | 5 | — |
| ChecksumValidator | 3 | 4 | File, ByteBuffer |
| CompressionCodec | 4 | 4 | File, ArchiveFile, ByteBuffer |
| ConfigFile | 5 | 5 | TextFile |
| Directory | 6 | 7 | FileSystemNode, File, Inode, DirectoryWatcher |
| DirectoryWatcher | 4 | 4 | Directory, FileEvent |
| DiskDrive | 7 | 5 | Partition, SMARTMonitor |
| DiskQuota | 5 | 4 | UserAccount, StorageVolume |
|DuplicateFinder | 3 | 4 | Directory, File, ChecksumValidator |
| EncryptedFile | 6 | 5 | File, EncryptionEngine, CryptoKey |
| EncryptionEngine | 4 | 4 | CryptoKey, ByteBuffer, File |
| CryptoKey | 5 | 4 | UserAccount |
| Exceptions | 1 | 13 | FileSystemException, FileNotFoundException, PermissionDeniedException и др. |
| ExecutableFile | 5 | 4 | BinaryFile, UserAccount, PermissionMask |
| File | 8 | 7 | FileSystemNode, FileMetadata, Inode, FileLock |
| FileCopyTask | 6 | 4 | File, Directory, ByteBuffer |
| FileDescriptor | 5 | 4 | File, UserAccount, FileStream |
| FileEvent | 5 | 3 | FileSystemNode, UserAccount |
| FileIndexer | 4 | 5 | Directory, File, SearchIndex |
| FileLock | 5 | 4 | File, UserAccount, AuthSession |
| FileManagerCLI | 8 | 12 | VirtualFileSystem, UserAccount, SearchEngine, BackupScheduler, QuotaManager |
| FileMetadata | 7 | 5 | FileTag |
| FileStream | 6 | 6 | File, FileDescriptor, ByteBuffer |
| FileSystemNode | 7 | 6 | Directory, Inode, PermissionMask, UserAccount |
| FileTag | 3 | 3 | — |
| FileVersion | 6 | 4 | File, UserAccount, ChecksumValidator |
| HardLink | 4 | 3 | FileSystemNode, Inode |
| Inode | 8 | 5 | StorageBlock, PermissionMask |
| InodeTable | 4 | 5 | Inode, Partition |
| LogFile | 5 | 4 | TextFile |
| MediaFile | 7 | 4 | BinaryFile, FileMetadata |
| MountManager | 4 | 5 | StorageVolume, Directory, Partition |
| Partition | 6 | 5 | DiskDrive, StorageBlock, InodeTable |
| PathResolver | 3 | 5 | Directory, FileSystemNode, SymbolicLink |
| PermissionMask | 4 | 5 | — |
| QuotaManager | 4 | 5 | DiskQuota, UserAccount, StorageVolume |
| RecycleBin | 5 | 5 | FileSystemNode, Directory, TrashItem |
| SearchEngine | 4 | 5 | SearchQuery, SearchIndex, FileSystemNode |
| SearchIndex | 4 | 4 | File, FileTag |
| SearchQuery | 5 | 4 | FileTag |
| SMARTMonitor | 5 | 4 | DiskDrive |
| StorageBlock | 4 | 4 | — |
| StorageVolume | 7 | 6 | Partition, Directory, BlockAllocator |
| SymbolicLink | 5 | 4 | FileSystemNode |
| TextFile | 5 | 5 | File |
| TrashItem | 5 | 3 | FileSystemNode, Directory |
| UserAccount | 6 | 5 | UserGroup, DiskQuota |
| UserGroup | 4 | 4 | UserAccount |
| VersionHistory | 4 | 5 | File, FileVersion |
| VirtualFileSystem | 7 | 8 | StorageVolume, MountManager, PathResolver, AccessControlList, RecycleBin |

## Исключения (13)

* `FileSystemException` (базовый класс исключений файловой системы)
* `FileNotFoundException`
* `DirectoryNotFoundException`
* `DirectoryNotEmptyException`
* `FileAlreadyExistsException`
* `PermissionDeniedException`
* `DiskQuotaExceededException`
* `OutOfDiskSpaceException`
* `FileLockedException`
* `CorruptedFileException`
* `InvalidPathException`
* `CyclicSymlinkException`
* `AuthenticationFailedException`

## Итоговая статистика

| Показатель | Значение |
| :--- | :--- |
| Классы | 63 (включая иерархию исключений) |
| Поля | 312 |
| Поведения | 289 |
| Ассоциации | 114 |
| Исключения | 13 |