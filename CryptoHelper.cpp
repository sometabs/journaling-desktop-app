#include "CryptoHelper.h"
#include "QAESEncryption.h"
#include "SaltManager.h"

#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>
#include <QtMath>

QByteArray CryptoHelper::m_key;

void CryptoHelper::setPassword(const QString& password)
{
    // Load or create master salt from file
    QByteArray masterSalt = SaltManager::getOrCreateSalt(32);

    // Derive key using PBKDF2-HMAC-SHA256
    m_key = deriveKeyPBKDF2(password, masterSalt, 20000, 32); // 20K iterations / AES-256 key size
}

QByteArray CryptoHelper::deriveKeyPBKDF2(const QString& password, const QByteArray& salt, int iterations, int dkLen)
{
    QByteArray key;
    QByteArray passwordBytes = password.toUtf8();
    int hashLen = QCryptographicHash::hashLength(QCryptographicHash::Sha256);
    int blocksNeeded = qCeil((double)dkLen / hashLen);

    for (int block = 1; block <= blocksNeeded; ++block) {
        QByteArray blockSalt = salt;
        blockSalt.append(char(block >> 24));
        blockSalt.append(char(block >> 16));
        blockSalt.append(char(block >> 8));
        blockSalt.append(char(block));

        QByteArray u = QMessageAuthenticationCode::hash(blockSalt, passwordBytes, QCryptographicHash::Sha256);
        QByteArray result = u;

        for (int i = 1; i < iterations; ++i) {
            u = QMessageAuthenticationCode::hash(u, passwordBytes, QCryptographicHash::Sha256);
            for (int j = 0; j < result.size(); ++j)
                result[j] ^= u[j];
        }

        key.append(result);
    }

    return key.left(dkLen);
}

QString CryptoHelper::encrypt(const QString& plainText)
{
    QAESEncryption encryption(QAESEncryption::AES_256, QAESEncryption::CBC);

    // Generate per-entry salt
    QByteArray entrySalt(16, 0);
    QRandomGenerator::global()->fillRange(reinterpret_cast<quint32*>(entrySalt.data()), entrySalt.size() / sizeof(quint32));

    // Derive per-entry key
    QByteArray derivedKey = QCryptographicHash::hash(m_key + entrySalt, QCryptographicHash::Sha256).left(32);

    // Generate IV
    QByteArray iv(16, 0);
    QRandomGenerator::global()->fillRange(reinterpret_cast<quint32*>(iv.data()), iv.size() / sizeof(quint32));

    // Encrypt
    QByteArray encrypted = encryption.encode(plainText.toUtf8(), derivedKey, iv);

    // Store [entrySalt | IV | ciphertext]
    QByteArray result = entrySalt + iv + encrypted;
    return QString::fromUtf8(result.toBase64());
}

QString CryptoHelper::decrypt(const QString& base64EncryptedText)
{
    QByteArray full = QByteArray::fromBase64(base64EncryptedText.toUtf8());
    if (full.size() < 32) return QString(); // invalid: must contain salt + IV

    QByteArray entrySalt = full.left(16);
    QByteArray iv = full.mid(16, 16);
    QByteArray encrypted = full.mid(32);

    // Derive per-entry key
    QByteArray derivedKey = QCryptographicHash::hash(m_key + entrySalt, QCryptographicHash::Sha256).left(32);

    QAESEncryption encryption(QAESEncryption::AES_256, QAESEncryption::CBC);
    QByteArray decrypted = encryption.decode(encrypted, derivedKey, iv);

    return QString::fromUtf8(QAESEncryption::RemovePadding(decrypted));
}

void CryptoHelper::clearKey()
{
    m_key.fill(0);   // wipe sensitive memory
    m_key.clear();
}
