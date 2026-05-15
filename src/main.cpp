#include <QApplication>
#include <QCoreApplication>
#include <QSettings>

#include "Views/MainWindow.h"
#include "Utils/Themes.h"
#include "Data/Database.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("PerchOrg");
    QCoreApplication::setApplicationName("PerchQt");
    QCoreApplication::setApplicationVersion("0.1");

    Themes::applyTheme(QSettings().value("Theme/CurrentTheme", "System Default").toString());

    if (!Database::open())
        return 1;

    MainWindow w;
    w.showMaximized();
    return app.exec();
}
