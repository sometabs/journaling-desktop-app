#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H


#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>


// Definition of a single note
struct Note {
    int id;
    QString title;
    QString content;
    QString mood;
    QString createdAt;
};


class DatabaseManager {
public:
    static DatabaseManager& instance();

    bool openDatabase();
    bool createTables();
    int addNote(const QString& title, const QString& content, const QString& mood, const QString& createdAt);
    bool updateNote(int id, const QString& title, const QString& content, const QString& mood, const QString& createdAt);
    QList<Note> getAllNotes();
    Note getNoteById(int id);
    bool storeAuthToken();
    bool validateAuthToken();



private:
    DatabaseManager(); // Private constructor for singleton
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
