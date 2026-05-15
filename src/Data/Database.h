#ifndef PERCHQT_DATABASE_H
#define PERCHQT_DATABASE_H

#include <QString>

class Database {
public:
    // Open (and migrate) the application's SQLite database.
    // Returns true on success. Sets up the default Qt SQL connection.
    static bool open();

    // Path the database lives at — for diagnostics.
    static QString path();

private:
    static bool runMigrations();
};

#endif // PERCHQT_DATABASE_H
