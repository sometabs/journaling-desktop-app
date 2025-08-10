#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "DatabaseManager.h"
#include "PasswordDialog.h"
#include "CryptoHelper.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <qdatetime.h>
#include <qmessagebox.h>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("My Journal App");

    // Linking the search box to the noteModel (list of notes)
    noteModel = new QStandardItemModel(this);
    //ui->listView->setModel(noteModel);
    noteFilterModel = new QSortFilterProxyModel(this);
    noteFilterModel->setSourceModel(noteModel);
    noteFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    noteFilterModel->setFilterKeyColumn(0); // Title is column 0
    ui->listView->setModel(noteFilterModel);


    // Load notes
    loadNotes(-1); // Don't select any note by default (-1)


    // All the connect functions
    connect(ui->listView, &QListView::clicked,this, &MainWindow::onNoteSelected);
    connect(ui->saveButton, &QPushButton::clicked,this, &MainWindow::onSaveButtonClicked);
    connect(ui->newEntryButton, &QPushButton::clicked,this, &MainWindow::onNewEntryClicked);
    connect(ui->lockButton, &QPushButton::clicked, this, &MainWindow::handleLock);
    connect(ui->searchBox, &QLineEdit::textChanged, this, [=](const QString& text) {
        noteFilterModel->setFilterFixedString(text);
        // Deselect current item (prevents broken selection)
        selectedNoteId = -1;
        ui->listView->clearSelection();
        ui->listView->setCurrentIndex(QModelIndex());
        ui->titleView->clear();
        ui->contentView->clear();
    });


    // Default note section UI
    QDateTime now = QDateTime::currentDateTime();
    QString formatted = now.toString("dd/MM/yy HH:mm");
    ui->dateView->setText(formatted);


}

MainWindow::~MainWindow()
{
    delete ui;
}

QModelIndex MainWindow::loadNotes(int highlightNoteId) {
    auto notes = DatabaseManager::instance().getAllNotes();
    noteModel->clear();
    QModelIndex targetIndex;

    for (int i = 0; i < notes.size(); ++i) {
        QString label = notes.at(i).title + " [" + notes.at(i).mood + "]";
        QStandardItem* item = new QStandardItem(label);
        item->setData(notes.at(i).id, Qt::UserRole + 1);
        noteModel->appendRow(item);
        if (notes.at(i).id == highlightNoteId) {
            targetIndex = noteModel->index(i, 0);  // save index to reselect
        }
    }
    return targetIndex;
}


void MainWindow::onNoteSelected(const QModelIndex& index) {
    if (!index.isValid()) return;

    // Get the selected item's stored note ID
    int noteId = index.data(Qt::UserRole + 1).toInt();
    selectedNoteId = noteId; // Save the selected note

    // Fetch note from DB
    Note note = DatabaseManager::instance().getNoteById(noteId);

    // Parse and format the date
    QDateTime created = QDateTime::fromString(note.createdAt, "yyyy-MM-dd HH:mm:ss");
    QString formattedDate = created.toString("dd/MM/yy HH:mm");

    // Find mood and display
    int moodIndex = ui->moodSelection->findText(note.mood, Qt::MatchFixedString);
    if (moodIndex != -1)ui->moodSelection->setCurrentIndex(moodIndex);


    // Display all informations related to the note
    ui->contentView->setPlainText(note.content);
    ui->titleView->setText(note.title);
    ui->dateView->setText(formattedDate);
}


void MainWindow::onNewEntryClicked(){
    ui->contentView->clear();
    ui->titleView->clear();

    // Get current datetime
    QDateTime now = QDateTime::currentDateTime();

    // Format it as "dd/MM/yy HH:mm"
    QString formatted = now.toString("dd/MM/yy HH:mm");

    ui->dateView->setText(formatted);

    selectedNoteId = -1; // Reset the NoteId
    ui->listView->clearSelection();
}


void MainWindow::onSaveButtonClicked()
{

    QString content = ui->contentView->toPlainText().trimmed();
    QString Title = ui->titleView->text().trimmed();

    if (content.isEmpty() || Title.isEmpty()) {
        QMessageBox::warning(this, "Missing", "Title/content empty.");
        return;
    }

    QString title = ui->titleView->text();
    QString mood = ui->moodSelection->currentText().toLower();

    QString dateStr = ui->dateView->text().trimmed();
    QDateTime customDate = QDateTime::fromString(dateStr, "dd/MM/yy HH:mm"); // Convert to Qdate
    QString createdAtStr = customDate.toString("yyyy-MM-dd HH:mm:ss"); // format for SQLite

    if (selectedNoteId == -1) {
        // NEW
        selectedNoteId = DatabaseManager::instance().addNote(title, content, mood, createdAtStr);
    } else {
        // UPDATE
        QDateTime now = QDateTime::currentDateTime();
        createdAtStr = now.toString("yyyy-MM-dd HH:mm:ss");
        DatabaseManager::instance().updateNote(selectedNoteId, title, content, mood, createdAtStr);
    }



    // Reload the list
    QModelIndex indexToSelect = loadNotes(selectedNoteId); // Source model
    QModelIndex proxyIndex = noteFilterModel->mapFromSource(indexToSelect);
    if (indexToSelect.isValid()) {
        ui->listView->setCurrentIndex(proxyIndex);
    }

}


void MainWindow::handleLock()
{
    // Clear sensitive state
    ui->contentView->clear();
    ui->titleView->clear();
    selectedNoteId = -1;
    CryptoHelper::clearKey();

    // Hide the main window
    this->hide();

    // Show password dialog again
    PasswordDialog dlg(PasswordDialog::Login);
    if (dlg.exec() == QDialog::Accepted) {
        /*QString password = dlg.getPassword();
        CryptoHelper::setPassword(password);*/

        // Re-show the main window
        this->show();
        loadNotes(-1); // re-fetch notes (optional)
    } else {
        qApp->quit(); // User canceled → exit app
    }
}


