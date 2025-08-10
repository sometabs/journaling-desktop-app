#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNoteSelected(const QModelIndex& index);
    void onSaveButtonClicked();
    void onNewEntryClicked();
    void handleLock();

private:
    Ui::MainWindow *ui;
    QStandardItemModel* noteModel;
    QSortFilterProxyModel* noteFilterModel;
    QModelIndex loadNotes(int highlightNoteId);
    int selectedNoteId = -1;

};
#endif // MAINWINDOW_H
