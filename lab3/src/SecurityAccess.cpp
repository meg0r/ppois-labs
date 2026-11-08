#include "SecurityAccess.hpp"
#include "Exceptions.hpp"
#include "FileHierarchy.hpp"
#include "IOOperations.hpp"
#include "LifecycleAndBackup.hpp"
#include "SearchAndMetadata.hpp"
#include "StorageHardware.hpp"

#include <algorithm>
#include <utility>

// ==========================================
// PermissionMask
// ==========================================

PermissionMask::PermissionMask(int mode, bool r, bool w, bool x)
    : octalMode(mode),
      ownerRead(r),
      ownerWrite(w),
      ownerExecute(x) {
    if (mode < 0 || mode > 777) {
        throw FileSystemException("Octal permission mode must be within [0, 777].");
    }
}

int PermissionMask::getOctalMode() const noexcept { return octalMode; }
void PermissionMask::grantWrite() noexcept { ownerWrite = true; }
void PermissionMask::revokeWrite() noexcept { ownerWrite = false; }
bool PermissionMask::canRead() const noexcept { return ownerRead; }
bool PermissionMask::canWrite() const noexcept { return ownerWrite; }
bool PermissionMask::canExecute() const noexcept { return ownerExecute; }

// ==========================================
// UserAccount
// ==========================================

UserAccount::UserAccount(int id, std::string uname, std::string passHash)
    : userId(id),
      username(std::move(uname)),
      passwordHash(std::move(passHash)),
      isAccountLocked(false),
      primaryGroup(nullptr),
      assignedQuota(nullptr) {
    if (id <= 0 || username.empty()) {
        throw FileSystemException("UserAccount requires positive ID and non-empty username.");
    }
    if (passwordHash.length() < MIN_PASSWORD_HASH_LEN) {
        throw AuthenticationFailedException("Password hash does not meet minimum security length.");
    }
}

int UserAccount::getUserId() const noexcept { return userId; }
const std::string& UserAccount::getUsername() const noexcept { return username; }

bool UserAccount::verifyPassword(const std::string& candidateHash) const noexcept {
    return !isAccountLocked && (passwordHash == candidateHash);
}

void UserAccount::joinGroup(UserGroup* group) {
    if (group == nullptr) {
        throw FileSystemException("UserGroup pointer cannot be null.");
    }
    primaryGroup = group;
}

void UserAccount::assignQuota(DiskQuota* quota) {
    if (quota == nullptr) {
        throw FileSystemException("DiskQuota pointer cannot be null.");
    }
    assignedQuota = quota;
}

void UserAccount::lockAccount() noexcept { isAccountLocked = true; }
bool UserAccount::isLocked() const noexcept { return isAccountLocked; }

bool UserAccount::operator==(const UserAccount& other) const noexcept {
    return userId == other.userId;
}

// ==========================================
// AdminUser
// ==========================================

AdminUser::AdminUser(int id, std::string uname, std::string passHash, int clearanceLevel)
    : UserAccount(id, std::move(uname), std::move(passHash)),
      securityClearanceLevel(clearanceLevel),
      hasRootPrivileges(true),
      managedQuotaSystem(nullptr),
      securityAuditLogger(nullptr) {
    if (clearanceLevel <= 0) {
        throw FileSystemException("Admin clearance level must be positive.");
    }
}

void AdminUser::unlockUserAccount(UserAccount* targetUser, bool unlockFlag) {
    if (targetUser == nullptr) {
        throw FileSystemException("Target user pointer cannot be null.");
    }
    if (!unlockFlag) {
        targetUser->lockAccount();
    }
}

void AdminUser::toggleVolumeReadOnly(StorageVolume* volume, bool readOnly) const {
    if (volume == nullptr) {
        throw FileSystemException("StorageVolume pointer cannot be null.");
    }
    if (!hasRootPrivileges) {
        throw PermissionDeniedException("Root privileges required to modify volume state.");
    }
    volume->setReadOnlyMode(readOnly);
}

void AdminUser::bindAuditLogger(AuditLogger* logger) {
    if (logger == nullptr) {
        throw FileSystemException("AuditLogger pointer cannot be null.");
    }
    securityAuditLogger = logger;
}

bool AdminUser::isRootAdmin() const noexcept { return hasRootPrivileges; }

// ==========================================
// UserGroup
// ==========================================

UserGroup::UserGroup(int id, std::string name, bool systemGroup)
    : groupId(id),
      groupName(std::move(name)),
      isSystemGroup(systemGroup) {
    if (id <= 0 || groupName.empty()) {
        throw FileSystemException("UserGroup requires positive ID and non-empty group name.");
    }
}

int UserGroup::getGroupId() const noexcept { return groupId; }

void UserGroup::addMember(UserAccount* user) {
    if (user == nullptr) {
        throw FileSystemException("Cannot add null user to group.");
    }
    if (!hasMember(user->getUserId())) {
        groupMembers.push_back(user);
    }
}

void UserGroup::removeMember(int userId) {
    auto it = std::find_if(groupMembers.begin(), groupMembers.end(),
        [userId](const UserAccount* u) { return u->getUserId() == userId; });
    if (it == groupMembers.end()) {
        throw FileSystemException("User is not a member of this group.");
    }
    groupMembers.erase(it);
}

bool UserGroup::hasMember(int userId) const noexcept {
    return std::any_of(groupMembers.begin(), groupMembers.end(),
        [userId](const UserAccount* u) { return u->getUserId() == userId; });
}

size_t UserGroup::getMemberCount() const noexcept { return groupMembers.size(); }

// ==========================================
// AccessControlEntry
// ==========================================

AccessControlEntry::AccessControlEntry(int id, bool allow, UserAccount* user, PermissionMask* mask)
    : entryId(id),
      isAllowRule(allow),
      targetUser(user),
      entryPermissions(mask) {
    if (id <= 0 || user == nullptr || mask == nullptr) {
        throw FileSystemException("AccessControlEntry requires valid user and permission mask.");
    }
}

int AccessControlEntry::getEntryId() const noexcept { return entryId; }

bool AccessControlEntry::appliesToUser(int userId) const noexcept {
    return targetUser->getUserId() == userId;
}

bool AccessControlEntry::isAllowedToWrite() const noexcept {
    return isAllowRule && entryPermissions->canWrite();
}

bool AccessControlEntry::isAllowedToRead() const noexcept {
    return isAllowRule && entryPermissions->canRead();
}

// ==========================================
// AccessControlList
// ==========================================

AccessControlList::AccessControlList(int id, FileSystemNode* node)
    : aclId(id),
      protectedNode(node) {
    if (id <= 0 || node == nullptr) {
        throw FileSystemException("AccessControlList requires positive ID and protected node.");
    }
}

int AccessControlList::getAclId() const noexcept { return aclId; }

void AccessControlList::addEntry(AccessControlEntry* entry) {
    if (entry == nullptr) {
        throw FileSystemException("ACL entry pointer cannot be null.");
    }
    aclEntries.push_back(entry);
}

void AccessControlList::removeEntry(int entryId) {
    auto it = std::find_if(aclEntries.begin(), aclEntries.end(),
        [entryId](const AccessControlEntry* e) { return e->getEntryId() == entryId; });
    if (it == aclEntries.end()) {
        throw FileNotFoundException("ACL entry not found.");
    }
    aclEntries.erase(it);
}

bool AccessControlList::checkUserReadAccess(const UserAccount* user) const {
    if (user == nullptr) {
        throw AuthenticationFailedException("User cannot be null during ACL check.");
    }
    for (const auto* entry : aclEntries) {
        if (entry->appliesToUser(user->getUserId())) {
            return entry->isAllowedToRead();
        }
    }
    return false;
}

bool AccessControlList::checkUserWriteAccess(const UserAccount* user) const {
    if (user == nullptr) {
        throw AuthenticationFailedException("User cannot be null during ACL check.");
    }
    for (const auto* entry : aclEntries) {
        if (entry->appliesToUser(user->getUserId())) {
            return entry->isAllowedToWrite();
        }
    }
    return false;
}

// ==========================================
// AuthSession
// ==========================================

AuthSession::AuthSession(int id, std::string ip, std::string loginTime, UserAccount* user)
    : sessionId(id),
      clientIpAddress(std::move(ip)),
      loginTimestamp(std::move(loginTime)),
      isSessionActive(true),
      authenticatedUser(user) {
    if (id <= 0 || clientIpAddress.empty() || loginTimestamp.empty() || user == nullptr) {
        throw AuthenticationFailedException("AuthSession requires valid IP, timestamp, and user.");
    }
    if (user->isLocked()) {
        throw AuthenticationFailedException("Cannot open session for a locked user account.");
    }
}

int AuthSession::getSessionId() const noexcept { return sessionId; }

void AuthSession::terminateSession() {
    if (!isSessionActive) {
        throw FileSystemException("Auth session is already terminated.");
    }
    isSessionActive = false;
}

bool AuthSession::isActive() const noexcept { return isSessionActive; }
UserAccount* AuthSession::getUser() const noexcept { return authenticatedUser; }

// ==========================================
// AccessToken
// ==========================================

AccessToken::AccessToken(int id, std::string secret, int ttlSeconds, AuthSession* session)
    : tokenId(id),
      tokenSecretString(std::move(secret)),
      expirationSeconds(ttlSeconds),
      isRevokedStatus(false),
      boundSession(session) {
    if (id <= 0 || tokenSecretString.empty() || ttlSeconds <= 0 || session == nullptr) {
        throw AuthenticationFailedException("Invalid AccessToken initialization parameters.");
    }
}

int AccessToken::getTokenId() const noexcept { return tokenId; }

bool AccessToken::validateToken(const std::string& candidateSecret, int elapsedSeconds) const noexcept {
    return !isRevokedStatus &&
           boundSession->isActive() &&
           (tokenSecretString == candidateSecret) &&
           (elapsedSeconds <= expirationSeconds);
}

void AccessToken::revokeToken() noexcept { isRevokedStatus = true; }

// ==========================================
// CryptoKey
// ==========================================

CryptoKey::CryptoKey(int id, std::string fingerprint, int bits, UserAccount* owner)
    : keyId(id),
      keyHexFingerprint(std::move(fingerprint)),
      keyLengthBits(bits),
      isExpiredKey(false),
      keyOwner(owner) {
    if (id <= 0 || keyHexFingerprint.empty() || (bits != 128 && bits != 256 && bits != 512) || owner == nullptr) {
        throw FileSystemException("CryptoKey requires valid ID, fingerprint, key bit length (128/256/512), and owner.");
    }
}

int CryptoKey::getKeyId() const noexcept { return keyId; }
void CryptoKey::markKeyExpired() noexcept { isExpiredKey = true; }
bool CryptoKey::isKeyValid() const noexcept { return !isExpiredKey; }

bool CryptoKey::operator==(const CryptoKey& other) const noexcept {
    return keyId == other.keyId && keyHexFingerprint == other.keyHexFingerprint;
}

// ==========================================
// EncryptionEngine
// ==========================================

EncryptionEngine::EncryptionEngine(int id, std::string suiteName, CryptoKey* key)
    : engineId(id),
      cipherSuiteName(std::move(suiteName)),
      processedBlocksCount(0),
      activeSessionKey(key) {
    if (id <= 0 || cipherSuiteName.empty() || key == nullptr || !key->isKeyValid()) {
        throw FileSystemException("EncryptionEngine requires valid cipher suite and active key.");
    }
}

int EncryptionEngine::getEngineId() const noexcept { return engineId; }

void EncryptionEngine::rotateSessionKey(CryptoKey* newKey) {
    if (newKey == nullptr || !newKey->isKeyValid()) {
        throw PermissionDeniedException("Replacement cryptographic key is null or expired.");
    }
    activeSessionKey = newKey;
}

void EncryptionEngine::transformBuffer(ByteBuffer* buffer) {
    if (buffer == nullptr) {
        throw FileSystemException("ByteBuffer for encryption transform cannot be null.");
    }
    if (!activeSessionKey->isKeyValid()) {
        throw PermissionDeniedException("Active cryptographic key has expired.");
    }
    ++processedBlocksCount;
}

int EncryptionEngine::getProcessedBlocksCount() const noexcept { return processedBlocksCount; }