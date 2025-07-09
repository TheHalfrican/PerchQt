#include <QApplication>
#include "Views/MainWindow.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    // --- Database initialization ---
    {
        QString dbPath = QCoreApplication::applicationDirPath()
                         + QDir::separator()
                         + "perch.db";
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(dbPath);
        if (!db.open()) {
            qCritical() << "Failed to open database at" << dbPath << ":" << db.lastError().text();
            return -1;
        }
        qDebug() << "Database opened at" << dbPath;
        // Ensure the games table exists
        {
            QSqlQuery q;
            const QString createSql = R"(
                CREATE TABLE IF NOT EXISTS games (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    title TEXT,
                    file_path TEXT UNIQUE,
                    cover_path TEXT,
                    last_played TEXT,
                    play_count INTEGER
                )
            )";
            if (!q.exec(createSql)) {
                qWarning() << "Failed to create games table:" << q.lastError().text();
            }
        }
    }
    // --- End database initialization ---
    MainWindow w;
    w.showMaximized();
    return app.exec();
}
