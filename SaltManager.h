#ifndef SALTMANAGER_H
#define SALTMANAGER_H

#include <QByteArray>
#include <QString>

class SaltManager
{
public:
    static QByteArray getOrCreateSalt(int size = 32); // 32 bytes = 256-bit
    static QString getSaltFilePath();

};

#endif // SALTMANAGER_H
