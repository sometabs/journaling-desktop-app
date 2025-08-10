#ifndef CRYPTOHELPER_H
#define CRYPTOHELPER_H

#include <QString>
#include <QByteArray>

class CryptoHelper
{
public:
    static void setPassword(const QString& password);
    static QString encrypt(const QString& plainText);
    static QString decrypt(const QString& base64EncryptedText);
    static void clearKey();

private:
    static QByteArray deriveKeyPBKDF2(const QString& password, const QByteArray& salt, int iterations, int dkLen);
    static QByteArray m_key;
};

#endif // CRYPTOHELPER_H
