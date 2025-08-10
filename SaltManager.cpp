#include "SaltManager.h"

#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QRandomGenerator>

QByteArray SaltManager::getOrCreateSalt(int size)
{
    QString filePath = getSaltFilePath();
    QFile file(filePath);

    // If it already exists, read and return it
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray salt = file.readAll();
            file.close();
            return salt;
        }
    }

    // Otherwise, create a new one
    QByteArray salt(size, 0);
    QRandomGenerator::global()->fillRange(reinterpret_cast<quint32*>(salt.data()), size / sizeof(quint32));


    // Just in case
    int remainder = size % sizeof(quint32);
    if (remainder) {
        quint32 rnd = QRandomGenerator::global()->generate();
        memcpy(salt.data() + size - remainder, &rnd, remainder);
    }

    // Save to file
    if (file.open(QIODevice::WriteOnly)) {
        file.write(salt);
        file.close();
    }

    return salt;
}

QString SaltManager::getSaltFilePath()
{
    QString saltPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(saltPath);
    return saltPath + "/master_salt.bin";
}
