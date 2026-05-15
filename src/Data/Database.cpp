#include "Data/Database.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QDebug>

namespace {

QString resolveDbPath()
{
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    return dataDir + QDir::separator() + "perch.db";
}

void seedFromBundleIfMissing(const QString& target)
{
    if (QFile::exists(target)) return;
    const QString bundled = QCoreApplication::applicationDirPath()
                          + QDir::separator() + "perch.db";
    if (QFile::exists(bundled) && QFile::copy(bundled, target)) {
        qInfo() << "Seeded database from" << bundled << "to" << target;
    }
}

int currentSchemaVersion(QSqlDatabase& db)
{
    QSqlQuery q(db);
    if (!q.exec("PRAGMA user_version")) return 0;
    if (q.next()) return q.value(0).toInt();
    return 0;
}

bool setSchemaVersion(QSqlDatabase& db, int version)
{
    QSqlQuery q(db);
    // PRAGMA doesn't take bound parameters; version is an int we control.
    return q.exec(QString("PRAGMA user_version = %1").arg(version));
}

bool execOrWarn(QSqlDatabase& db, const QString& sql)
{
    QSqlQuery q(db);
    if (!q.exec(sql)) {
        qWarning() << "Migration failed:" << q.lastError().text() << "\n  SQL:" << sql;
        return false;
    }
    return true;
}

} // namespace

QString Database::path()
{
    return resolveDbPath();
}

bool Database::open()
{
    const QString target = resolveDbPath();
    seedFromBundleIfMissing(target);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(target);
    if (!db.open()) {
        qCritical() << "Failed to open database at" << target << ":" << db.lastError().text();
        return false;
    }
    qInfo() << "Database opened at" << target;

    // Concurrency-friendly defaults for SQLite.
    QSqlQuery pragma(db);
    pragma.exec("PRAGMA journal_mode = WAL");
    pragma.exec("PRAGMA foreign_keys = ON");

    return runMigrations();
}

bool Database::runMigrations()
{
    QSqlDatabase db = QSqlDatabase::database();
    int version = currentSchemaVersion(db);

    // v1 — initial schema.
    if (version < 1) {
        if (!execOrWarn(db, R"(
            CREATE TABLE IF NOT EXISTS games (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                title TEXT,
                file_path TEXT UNIQUE,
                cover_path TEXT,
                last_played TEXT,
                play_count INTEGER
            )
        )")) return false;
        if (!setSchemaVersion(db, 1)) return false;
        version = 1;
    }

    // Future migrations go here, each guarded by `if (version < N)`.

    return true;
}
