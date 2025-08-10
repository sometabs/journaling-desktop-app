#include "MainWindow.h"
#include "SaltManager.h"
#include "PasswordDialog.h"
#include "DatabaseManager.h"

#include <QApplication>
#include <qmessagebox.h>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Initialize the DB
    DatabaseManager& db = DatabaseManager::instance();
    db.openDatabase();
    db.createTables();

    // Check if the salt exists
    bool firstLaunch = !QFile::exists(SaltManager::getSaltFilePath());

    // Create the password dialog and decide what's the mode .
    PasswordDialog::Mode mode = firstLaunch ? PasswordDialog::Create : PasswordDialog::Login;
    PasswordDialog dlg(mode);

    // User canceled
    if (dlg.exec() != QDialog::Accepted) {
        return 0;
    }

    // Show main window
    MainWindow w;
    w.show();
    return a.exec();
}
