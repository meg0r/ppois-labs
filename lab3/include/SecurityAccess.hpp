#pragma once

#include <string>
#include <vector>

class UserGroup;
class DiskQuota;
class StorageVolume;
class QuotaManager;
class AuditLogger;
class FileSystemNode;
class ByteBuffer;
class File;

class PermissionMask {
private:
    int octalMode;
    bool ownerRead;
    bool ownerWrite;
    bool ownerExecute;

public:
    PermissionMask(int mode, bool r, bool w, bool x);
    int getOctalMode() const noexcept;
    void grantWrite() noexcept;
    void revokeWrite() noexcept;
    bool canRead() const noexcept;
    bool canWrite() const noexcept;
    bool canExecute() const noexcept;
};

class UserAccount {
protected:
    int userId;
    std::string username;
    std::string passwordHash;
    bool isAccountLocked;
    UserGroup* primaryGroup;
    DiskQuota* assignedQuota;

public:
    static constexpr size_t MIN_PASSWORD_HASH_LEN = 6;

    UserAccount(int id, std::string uname, std::string passHash);
    virtual ~UserAccount() = default;

    int getUserId() const noexcept;
    const std::string& getUsername() const noexcept;
    bool verifyPassword(const std::string& candidateHash) const noexcept;
    void joinGroup(UserGroup* group);
    void assignQuota(DiskQuota* quota);
    void lockAccount() noexcept;
    bool isLocked() const noexcept;
    bool operator==(const UserAccount& other) const noexcept;
};

class AdminUser : public UserAccount {
private:
    int securityClearanceLevel;
    bool hasRootPrivileges;
    QuotaManager* managedQuotaSystem;
    AuditLogger* securityAuditLogger;

public:
    AdminUser(int id, std::string uname, std::string passHash, int clearanceLevel);
    void unlockUserAccount(UserAccount* targetUser, bool unlockFlag);
    void toggleVolumeReadOnly(StorageVolume* volume, bool readOnly) const;
    void bindAuditLogger(AuditLogger* logger);
    bool isRootAdmin() const noexcept;
};

class UserGroup {
private:
    int groupId;
    std::string groupName;
    bool isSystemGroup;
    std::vector<UserAccount*> groupMembers;

public:
    UserGroup(int id, std::string name, bool systemGroup = false);
    int getGroupId() const noexcept;
    void addMember(UserAccount* user);
    void removeMember(int userId);
    bool hasMember(int userId) const noexcept;
    size_t getMemberCount() const noexcept;
};

class AccessControlEntry {
private:
    int entryId;
    bool isAllowRule;
    UserAccount* targetUser;
    PermissionMask* entryPermissions;

public:
    AccessControlEntry(int id, bool allow, UserAccount* user, PermissionMask* mask);
    int getEntryId() const noexcept;
    bool appliesToUser(int userId) const noexcept;
    bool isAllowedToWrite() const noexcept;
    bool isAllowedToRead() const noexcept;
};

class AccessControlList {
private:
    int aclId;
    FileSystemNode* protectedNode;
    std::vector<AccessControlEntry*> aclEntries;

public:
    AccessControlList(int id, FileSystemNode* node);
    int getAclId() const noexcept;
    void addEntry(AccessControlEntry* entry);
    void removeEntry(int entryId);
    bool checkUserReadAccess(const UserAccount* user) const;
    bool checkUserWriteAccess(const UserAccount* user) const;
};

class AuthSession {
private:
    int sessionId;
    std::string clientIpAddress;
    std::string loginTimestamp;
    bool isSessionActive;
    UserAccount* authenticatedUser;

public:
    AuthSession(int id, std::string ip, std::string loginTime, UserAccount* user);
    int getSessionId() const noexcept;
    void terminateSession();
    bool isActive() const noexcept;
    UserAccount* getUser() const noexcept;
};

class AccessToken {
private:
    int tokenId;
    std::string tokenSecretString;
    int expirationSeconds;
    bool isRevokedStatus;
    AuthSession* boundSession;

public:
    AccessToken(int id, std::string secret, int ttlSeconds, AuthSession* session);
    int getTokenId() const noexcept;
    bool validateToken(const std::string& candidateSecret, int elapsedSeconds) const noexcept;
    void revokeToken() noexcept;
};

class CryptoKey {
private:
    int keyId;
    std::string keyHexFingerprint;
    int keyLengthBits;
    bool isExpiredKey;
    UserAccount* keyOwner;

public:
    CryptoKey(int id, std::string fingerprint, int bits, UserAccount* owner);
    int getKeyId() const noexcept;
    void markKeyExpired() noexcept;
    bool isKeyValid() const noexcept;
    bool operator==(const CryptoKey& other) const noexcept;
};

class EncryptionEngine {
private:
    int engineId;
    std::string cipherSuiteName;
    int processedBlocksCount;
    CryptoKey* activeSessionKey;

public:
    EncryptionEngine(int id, std::string suiteName, CryptoKey* key);
    int getEngineId() const noexcept;
    void rotateSessionKey(CryptoKey* newKey);
    void transformBuffer(ByteBuffer* buffer);
    int getProcessedBlocksCount() const noexcept;
};