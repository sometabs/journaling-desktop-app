#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>

namespace Ui {
class PasswordDialog;
}

class PasswordDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode { Create, Login };
    explicit PasswordDialog(Mode mode, QWidget *parent = nullptr);
    ~PasswordDialog();

private:
    Ui::PasswordDialog *ui;
    Mode currentMode;
};

#endif // PASSWORDDIALOG_H
