#include "UnlockTokenManager.h"
#include <QStandardPaths>
#include <QFile>
#include <QDir>

QString UnlockTokenManager::getTokenFilePath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(path);
    return path + "/unlock_token.dat";
}

void UnlockTokenManager::writeEncryptedToken(const QString& encryptedToken)
{
    QFile file(getTokenFilePath());
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(encryptedToken.toUtf8());
        file.close();
    }
}

QString UnlockTokenManager::readEncryptedToken()
{
    QFile file(getTokenFilePath());
    if (file.open(QIODevice::ReadOnly)) {
        QString result = QString::fromUtf8(file.readAll());
        file.close();
        return result;
    }
    return "";
}

bool UnlockTokenManager::exists()
{
    return QFile::exists(getTokenFilePath());
}
