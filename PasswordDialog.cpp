#include "PasswordDialog.h"
#include "ui_PasswordDialog.h"
#include "DatabaseManager.h"
#include "CryptoHelper.h"

#include <qmessagebox.h>
#include <QPushButton>


PasswordDialog::PasswordDialog(Mode mode, QWidget *parent)
    : QDialog(parent), ui(new Ui::PasswordDialog), currentMode(mode)
{
    ui->setupUi(this);
    setWindowTitle("Unlock the notes");

    if (mode == Create) {
        ui->label->setText("Create a new password");
        ui->confirmEdit->setVisible(true);
    } else {
        ui->label->setText("Enter your password");
        ui->confirmEdit->setVisible(false);
    }

    // Override the default OK button behavior to prevent redirection in case of
    // a wrong/empty confirmation password
    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [=]() {
        QString password = ui->passwordEdit->text();

        if (currentMode == Create) {
            QString confirm = ui->confirmEdit->text();

            if (password.isEmpty() || confirm.isEmpty()) {
                QMessageBox::warning(this, "Empty Fields", "Please enter and confirm your password.");
                return;
            }

            if (password != confirm) {
                QMessageBox::warning(this, "Mismatch", "Passwords do not match.");
                return;
            }


            CryptoHelper::setPassword(password);
            // Store the Auth token
            if (!DatabaseManager::instance().storeAuthToken()) {
                QMessageBox::critical(this, "Error", "Failed to initialize secure environment.");
                return;
            }

        }else{

            if (password.isEmpty()) {
                QMessageBox::warning(this, "Empty Password", "Please enter your password.");
                return;
            }

            CryptoHelper::setPassword(password);
            // Validate the Auth token
            if (!DatabaseManager::instance().validateAuthToken()) {
                QMessageBox::critical(this, "Incorrect Password", "The password you entered is incorrect.");
                return;
            }
        }

        // If checks pass, close dialog with success
        this->accept();
    });
}


PasswordDialog::~PasswordDialog()
{
    delete ui;
}

