#include "DatabaseManager.h"
#include "CryptoHelper.h"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>


DatabaseManager::DatabaseManager() {}

DatabaseManager::~DatabaseManager() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::openDatabase() {

    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);

    QString dbPath = configDir + "/my_journal.db";

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);


    if (!m_db.open()) {
        qDebug() << "Database open failed:" << m_db.lastError().text();
        return false;
    }

    qDebug() << "Database opened successfully.";
    return true;
}

bool DatabaseManager::createTables() {
    QSqlQuery query;
    QString createStmt =
        "CREATE TABLE IF NOT EXISTS notes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "title TEXT NOT NULL, "
        "content TEXT, "
        "mood TEXT, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ")";

    if (!query.exec(createStmt)) {
        qDebug() << "Create table failed:" << query.lastError().text();
        return false;
    }


    QString authStmt =
        "CREATE TABLE IF NOT EXISTS auth ("
        "id INTEGER PRIMARY KEY, "
        "token TEXT NOT NULL"
        ")";

    if (!query.exec(authStmt)) {
        qDebug() << "Create auth table failed:" << query.lastError().text();
        return false;
    }


    return true;
}



int DatabaseManager::addNote(const QString& title, const QString& content, const QString& mood, const QString& createdAt) {

    QString encrypted_title = CryptoHelper::encrypt(title);
    QString encrypted_content = CryptoHelper::encrypt(content);
    QString encrypted_mood = CryptoHelper::encrypt(mood);

    QSqlQuery query;
    query.prepare("INSERT INTO notes (title, content, mood, created_at) "
                  "VALUES (:title, :content, :mood, :created_at)");
    query.bindValue(":title", encrypted_title);
    query.bindValue(":content", encrypted_content);
    query.bindValue(":mood", encrypted_mood);
    query.bindValue(":created_at", createdAt);

    if (!query.exec()) {
        qDebug() << "Insert failed:" << query.lastError().text();
        return false;
    }

    return query.lastInsertId().toInt(); // Return the last note's id for UI selection
}


bool DatabaseManager::updateNote(int id, const QString& title, const QString& content, const QString& mood, const QString& createdAt) {


    QString encrypted_title = CryptoHelper::encrypt(title);
    QString encrypted_content = CryptoHelper::encrypt(content);
    QString encrypted_mood = CryptoHelper::encrypt(mood);

    QSqlQuery query;
    query.prepare("UPDATE notes SET title = :title, content = :content, mood = :mood, created_at = :created_at "
                  "WHERE id = :id");
    query.bindValue(":title", encrypted_title);
    query.bindValue(":content", encrypted_content);
    query.bindValue(":mood", encrypted_mood);
    query.bindValue(":created_at", createdAt);
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Update failed:" << query.lastError().text();
        return false;
    }

    return true;
}



QList<Note> DatabaseManager::getAllNotes() {
    QList<Note> notes;
    QSqlQuery query("SELECT id, title, mood, created_at FROM notes ORDER BY id DESC");

    while (query.next()) {
        Note note;
        note.id = query.value(0).toInt();
        note.title = CryptoHelper::decrypt(query.value(1).toString());
        note.mood = CryptoHelper::decrypt(query.value(2).toString());
        note.createdAt = query.value(3).toString();
        notes.append(note);
    }

    return notes;
}


Note DatabaseManager::getNoteById(int id) {
    QSqlQuery query;
    query.prepare("SELECT id, title, content, mood, created_at FROM notes WHERE id = :id");
    query.bindValue(":id", id);
    Note note;

    if (query.exec() && query.next()) {
        note.id = query.value(0).toInt();
        note.title = CryptoHelper::decrypt(query.value(1).toString());
        note.content = CryptoHelper::decrypt(query.value(2).toString());
        note.mood = CryptoHelper::decrypt(query.value(3).toString());
        note.createdAt = query.value(4).toString();
    }

    return note;
}


bool DatabaseManager::storeAuthToken()
{
    QString encrypted = CryptoHelper::encrypt("auth_token");
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO auth (id, token) VALUES (1, :token)");
    query.bindValue(":token", encrypted);
    if (!query.exec()) {
        qDebug() << "Failed to store auth token:" << query.lastError().text();
        return false;
    }
    return true;
}


bool DatabaseManager::validateAuthToken()
{
    QSqlQuery query("SELECT token FROM auth WHERE id = 1");
    if (!query.exec() || !query.next()) return false;

    QString decrypted = CryptoHelper::decrypt(query.value(0).toString());
    return decrypted == "auth_token";
}


