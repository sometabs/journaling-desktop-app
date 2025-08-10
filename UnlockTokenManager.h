#ifndef UNLOCKTOKENMANAGER_H
#define UNLOCKTOKENMANAGER_H

#include <QString>

class UnlockTokenManager
{
public:
    static QString getTokenFilePath();
    static void writeEncryptedToken(const QString& encryptedToken);
    static QString readEncryptedToken();
    static bool exists();
};

#endif // UNLOCKTOKENMANAGER_H
