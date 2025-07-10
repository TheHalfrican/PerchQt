#include "Views/MainWindow.h"
#include "ui_MainWindow.h"
#include "SettingsDialog.h"
#include "ViewModels/GameListViewModel.h"

#include "Views/GameWidgetView.h"
#include "Views/GameListView.h"
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
#include <QDial>
#include <QScrollArea>
#include <QTimer>
#include <QLineEdit>
#include <QString>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_viewModel(new GameListViewModel(this))
    , m_showTitles(true)
{
    ui->setupUi(this);

    // Prepare list view (hidden by default)
    m_listView = ui->listView;
    ui->listView->setVisible(false);
    // Forward list-view actions to the same grid slots
    connect(m_listView, &GameListView::launchRequested,
            this, &MainWindow::onLaunchGame);
    connect(m_listView, &GameListView::removeRequested,
            this, &MainWindow::onRemoveGame);
    connect(m_listView, &GameListView::showFileRequested,
            this, &MainWindow::onShowFile);
    connect(m_listView, &GameListView::setCoverRequested,
            this, &MainWindow::onSetCoverImage);
    connect(m_listView, &GameListView::removeCoverRequested,
            this, &MainWindow::onRemoveCoverImage);

    // Open Settings dialog when the settings toolbar button is clicked
    connect(ui->settings_button, &QToolButton::clicked,
            this, &MainWindow::onSettingsClicked);

    // Show list view when the list toolbar button is clicked
    connect(ui->list_button, &QToolButton::clicked,
            this, &MainWindow::onListViewClicked);

    // Show grid view when the grid toolbar button is clicked
    connect(ui->grid_button, &QToolButton::clicked,
            this, &MainWindow::onGridViewClicked);

    // Toggle game title footers on grid view
    connect(ui->title_toggle_button, &QToolButton::clicked,
            this, &MainWindow::onTitleToggleClicked);

    // Filter grid as text is entered
    connect(ui->search_bar, &QLineEdit::textChanged,
            this, &MainWindow::onSearchTextChanged);

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
    connect(ui->actionAddGame, &QAction::triggered,
            this, &MainWindow::onAddGameClicked);

    connect(m_viewModel,
            &GameListViewModel::gamesChanged,
            this,
            &MainWindow::onGamesLoaded);

    connect(ui->actionSettings, &QAction::triggered,
            this, &MainWindow::onSettingsClicked);

    // Adjust grid columns when the dial value changes
    connect(ui->gridSizeDial, &QDial::valueChanged,
            this, &MainWindow::onGridSizeChanged);
    // Restore previously saved grid size
    {
        QSettings settings("PerchOrg", "PerchQt");
        int savedSize = settings.value("gridSize", ui->gridSizeDial->value()).toInt();
        ui->gridSizeDial->blockSignals(true);
        ui->gridSizeDial->setValue(savedSize);
        ui->gridSizeDial->blockSignals(false);
    }

    // Auto-scan saved folders on startup
    {
        QSettings settings("PerchOrg", "PerchQt");
        QStringList folders = settings.value("scanFolders").toStringList();
        for (const QString& folder : folders) {
            m_viewModel->scanFolder(folder);
        }
    }

    // Load games after the UI is laid out so columns compute correctly
    QTimer::singleShot(0, this, [this]() {
        m_viewModel->loadGames();
    });
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

    // Update master list only when no active filter
    if (ui->search_bar->text().isEmpty()) {
        m_allGames = games;
    }
    // Always update current displayed list
    m_lastGames = games;

    // Prevent crashes if scrollArea or viewport is unavailable
    if (!ui->scrollArea) {
        qWarning() << "onGamesLoaded: scrollArea is null!";
        return;
    }
    QWidget* vp = ui->scrollArea->viewport();
    if (!vp) {
        qWarning() << "onGamesLoaded: scrollArea viewport is null!";
        return;
    }
    if (!ui->gridLayout) {
        qWarning() << "onGamesLoaded: gridLayout is null!";
        return;
    }

    // Clear previous selection
    if (m_selectedView) {
        m_selectedView->setSelected(false);
        m_selectedView = nullptr;
    }

    // Clear existing items from the grid
    QLayoutItem* item;
    while ((item = ui->gridLayout->takeAt(0)) != nullptr) {
        if (QWidget* w = item->widget())
            w->deleteLater();
        delete item;
    }

    // Compute tile size from dial (range 100–350 px)
    int dialVal = ui->gridSizeDial->value();
    int tileSize = 50 + dialVal * 50;
    // Calculate how many columns fit in available width
    int available = ui->scrollArea->viewport()->width();
    int columns = qMax(1, available / tileSize);
    qDebug() << "Using tileSize =" << tileSize << "columns =" << columns;

    int row = 0;
    int col = 0;
    for (const Game& g : games) {
        auto* view = new GameWidgetView(this);
        // Honor current title-visibility setting
        view->setTitleVisible(m_showTitles);
        // Fix width only; height adjusts via internal layout
        view->setFixedWidth(tileSize);
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
    QString path = QFileDialog::getOpenFileName(this, "Select Cover Image");
    if (!path.isEmpty()) {
        m_viewModel->setCoverImage(gameId, path);
        // Refresh list view to pick up the new cover path
        m_listView->setGames(m_lastGames);
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

void MainWindow::onGridSizeChanged(int /*columns*/)
{
    // Save dial setting
    QSettings settings("PerchOrg", "PerchQt");
    settings.setValue("gridSize", ui->gridSizeDial->value());
    // Re-populate grid using the new column count
    onGamesLoaded(m_lastGames);
}

void MainWindow::onListViewClicked()
{
    // Switch to list view
    ui->scrollArea->setVisible(false);
    ui->listView->setVisible(true);
    // Hide the title toggle button in list view
    ui->title_toggle_button->setVisible(false);
    // Hide the grid-size dial in list view
    ui->gridSizeDial->setVisible(false);
    // Populate the list
    ui->listView->setGames(m_lastGames);
}

void MainWindow::onGridViewClicked()
{
    // Switch to grid view
    ui->listView->setVisible(false);
    ui->scrollArea->setVisible(true);
    // Show the title toggle button in grid view
    ui->title_toggle_button->setVisible(true);
    // Show the grid-size dial in grid view
    ui->gridSizeDial->setVisible(true);
    // Refresh grid content
    onGamesLoaded(m_lastGames);
}

void MainWindow::onTitleToggleClicked()
{
    // Flip visibility flag
    m_showTitles = !m_showTitles;
    // Update all existing grid items
    for (int i = 0; i < ui->gridLayout->count(); ++i) {
        if (auto* item = ui->gridLayout->itemAt(i)) {
            if (auto* w = qobject_cast<GameWidgetView*>(item->widget())) {
                w->setTitleVisible(m_showTitles);
            }
        }
    }
}

void MainWindow::onSearchTextChanged(const QString& text)
{
    if (text.isEmpty()) {
        // Show all games when filter cleared
        onGamesLoaded(m_allGames);
    } else {
        QVector<Game> filtered;
        filtered.reserve(m_allGames.size());
        for (const Game& g : m_allGames) {
            if (g.title.contains(text, Qt::CaseInsensitive))
                filtered.append(g);
        }
        onGamesLoaded(filtered);
    }
}