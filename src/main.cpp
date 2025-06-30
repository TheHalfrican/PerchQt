#include <QApplication>
#include "Views/MainWindow.h"
#include <QSqlDatabase>
#include <QSqlError>
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
    }
    // --- End database initialization ---
    MainWindow w;
    w.show();
    return app.exec();
}
