#include "Views/MainWindow.h"
#include "ui_MainWindow.h"
#include "SettingsDialog.h"
#include "ViewModels/GameListViewModel.h"

#include "Views/GameWidgetView.h"
#include <QLayoutItem>

#include <QFileDialog>
#include <QFileInfo>

#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QStringList>
#include <QSettings>
#include <QToolButton>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_viewModel(new GameListViewModel(this))
{
    ui->setupUi(this);

    // Open Settings dialog when the settings toolbar button is clicked
    connect(ui->settings_button, &QToolButton::clicked,
            this, &MainWindow::onSettingsClicked);

    // Verify default database connection
    {
        QSqlDatabase db = QSqlDatabase::database();
        if (!db.isValid() || !db.isOpen()) {
            qCritical() << "Default DB connection is not open or invalid:"
                        << (db.isValid() ? db.lastError().text() : "invalid connection");
        } else {
            qDebug() << "Using existing DB connection.";
        }
    }

    // Hook up the model
    // ui->listView->setModel(m_viewModel->gameListModel());

    // Connect signals
    connect(ui->listView, &QListView::clicked,
            m_viewModel, &GameListViewModel::onGameSelected);
    connect(ui->actionAddGame, &QAction::triggered,
            this, &MainWindow::onAddGameClicked);

    connect(m_viewModel,
            &GameListViewModel::gamesChanged,
            this,
            &MainWindow::onGamesLoaded);

    connect(ui->actionSettings, &QAction::triggered,
            this, &MainWindow::onSettingsClicked);

    // Auto-scan saved folders on startup
    {
        QSettings settings("PerchOrg", "PerchQt");
        QStringList folders = settings.value("scanFolders").toStringList();
        for (const QString& folder : folders) {
            m_viewModel->scanFolder(folder);
        }
    }

    // Load games
    m_viewModel->loadGames();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onAddGameClicked()
{
    // Let user pick the executable file
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    "Select Executable");
    if (filePath.isEmpty())
        return;

    // Derive title from filename without extension
    QFileInfo fi(filePath);
    QString title = fi.baseName();

    // No cover image prompt; leave coverPath empty
    QString coverPath;

    // Delegate to ViewModel
    m_viewModel->addGame(title, filePath, coverPath);
}

void MainWindow::onGamesLoaded(const QVector<Game>& games)
{
    qDebug() << "onGamesLoaded: received" << games.size() << "games";

    // Clear existing items from the grid
    QLayoutItem* item;
    while ((item = ui->gridLayout->takeAt(0)) != nullptr) {
        if (QWidget* w = item->widget())
            w->deleteLater();
        delete item;
    }

    // Populate grid with new GameWidgetViews
    const int columns = 3;
    int row = 0;
    int col = 0;
    for (const Game& g : games) {
        auto* view = new GameWidgetView(this);
        view->setGame(g);
        connect(view, &GameWidgetView::removeRequested,
                this, &MainWindow::onRemoveGame);
        connect(view, &GameWidgetView::launchRequested,
                this, &MainWindow::onLaunchGame);
        connect(view, &GameWidgetView::showFileRequested,
                this, &MainWindow::onShowFile);
        connect(view, &GameWidgetView::setCoverRequested,
                this, &MainWindow::onSetCoverImage);
        connect(view, &GameWidgetView::removeCoverRequested,
                this, &MainWindow::onRemoveCoverImage);
        // Ensure only one selected at a time
        connect(view, &GameWidgetView::clicked, this, [this, view]() {
            if (m_selectedView && m_selectedView != view) {
                m_selectedView->setSelected(false);
            }
            m_selectedView = view;
            view->setSelected(true);
        });
        ui->gridLayout->addWidget(view, row, col);
        if (++col >= columns) {
            col = 0;
            ++row;
        }
    }
}

void MainWindow::onRemoveGame(int gameId)
{
    m_viewModel->removeGame(gameId);
}

void MainWindow::onLaunchGame(int gameId)
{
    m_viewModel->launchGame(gameId);
}

void MainWindow::onShowFile(int gameId)
{
    m_viewModel->showGameFile(gameId);
}

void MainWindow::onSetCoverImage(int gameId)
{
    {
        QString path = QFileDialog::getOpenFileName(this, "Select Cover Image");
        if (!path.isEmpty()) {
            m_viewModel->setCoverImage(gameId, path);
        }
    }
}

void MainWindow::onRemoveCoverImage(int gameId)
{
    m_viewModel->removeCoverImage(gameId);
}

void MainWindow::onSettingsClicked()
{
    SettingsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        QStringList folders = dlg.scanFolders();
        for (const QString& folder : folders) {
            m_viewModel->scanFolder(folder);
        }
        // Save for next launch
        QSettings settings("PerchOrg", "PerchQt");
        settings.setValue("scanFolders", folders);
    }
}