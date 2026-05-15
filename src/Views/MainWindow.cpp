#include "Views/MainWindow.h"
#include "ui_MainWindow.h"

#include "Input/ControllerManager.h"
#include "ViewModels/GameListViewModel.h"
#include "Views/SettingsDialog.h"
#include "Views/GameWidgetView.h"
#include "Views/GameListView.h"
#include "Views/ControllerConfigView.h"

#include <QDial>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGuiApplication>
#include <QLineEdit>
#include <QPalette>
#include <QScreen>
#include <QScrollArea>
#include <QSettings>
#include <QStatusBar>
#include <QTimer>
#include <QToolButton>

namespace {
constexpr int kLogoUpdateDebounceMs = 80;
constexpr int kStatusMessageMs = 4000;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_viewModel(new GameListViewModel(this))
    , m_controllers(new ControllerManager(this))
    , m_logoUpdateTimer(new QTimer(this))
{
    ui->setupUi(this);

    {
        QSettings settings;
        m_showTitles = settings.value("showTitles", true).toBool();
        int savedSize = settings.value("gridSize", ui->gridSizeDial->value()).toInt();
        ui->gridSizeDial->blockSignals(true);
        ui->gridSizeDial->setValue(savedSize);
        ui->gridSizeDial->blockSignals(false);
    }

    m_logoUpdateTimer->setSingleShot(true);
    m_logoUpdateTimer->setInterval(kLogoUpdateDebounceMs);
    connect(m_logoUpdateTimer, &QTimer::timeout, this, &MainWindow::updateAppLogo);
    updateAppLogo();

    m_listView = ui->listView;
    ui->listView->setVisible(false);
    connect(m_listView, &GameListView::launchRequested,    this, &MainWindow::onLaunchGame);
    connect(m_listView, &GameListView::removeRequested,    this, &MainWindow::onRemoveGame);
    connect(m_listView, &GameListView::showFileRequested,  this, &MainWindow::onShowFile);
    connect(m_listView, &GameListView::setCoverRequested,  this, &MainWindow::onSetCoverImage);
    connect(m_listView, &GameListView::removeCoverRequested, this, &MainWindow::onRemoveCoverImage);

    connect(ui->settings_button,          &QToolButton::clicked, this, &MainWindow::onSettingsClicked);
    connect(ui->controller_config_button, &QToolButton::clicked, this, &MainWindow::onControllerSettingsClicked);
    connect(ui->list_button,              &QToolButton::clicked, this, &MainWindow::onListViewClicked);
    connect(ui->grid_button,              &QToolButton::clicked, this, &MainWindow::onGridViewClicked);
    connect(ui->title_toggle_button,      &QToolButton::clicked, this, &MainWindow::onTitleToggleClicked);
    connect(ui->search_bar,               &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);

    connect(ui->actionAddGame,            &QAction::triggered, this, &MainWindow::onAddGameClicked);
    connect(ui->actionSettings,           &QAction::triggered, this, &MainWindow::onSettingsClicked);
    connect(ui->actionControllerSettings, &QAction::triggered, this, &MainWindow::onControllerSettingsClicked);
    connect(ui->actionExit,               &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionToggleTitles,       &QAction::triggered, this, &MainWindow::onTitleToggleClicked);
    connect(ui->actionGridView,           &QAction::triggered, this, &MainWindow::onGridViewClicked);
    connect(ui->actionListView,           &QAction::triggered, this, &MainWindow::onListViewClicked);

    connect(ui->gridSizeDial, &QDial::valueChanged, this, &MainWindow::onGridSizeChanged);

    connect(m_viewModel, &GameListViewModel::gamesChanged,
            this, &MainWindow::onGamesLoaded);
    connect(m_viewModel, &GameListViewModel::statusMessage, this,
            [this](const QString& msg) { statusBar()->showMessage(msg, kStatusMessageMs); });

    connect(m_controllers, &ControllerManager::moveLeft,        this, [this]{ moveSelection(0, -1); });
    connect(m_controllers, &ControllerManager::moveRight,       this, [this]{ moveSelection(0, +1); });
    connect(m_controllers, &ControllerManager::moveUp,          this, [this]{ moveSelection(-1, 0); });
    connect(m_controllers, &ControllerManager::moveDown,        this, [this]{ moveSelection(+1, 0); });
    connect(m_controllers, &ControllerManager::confirmPressed,  this, &MainWindow::launchSelected);
    connect(m_controllers, &ControllerManager::menuPressed,     this, &MainWindow::onSettingsClicked);

    // Auto-scan saved folders on startup.
    {
        QSettings settings;
        for (const QString& folder : settings.value("scanFolders").toStringList())
            m_viewModel->scanFolder(folder);
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::updateAppLogo()
{
    qreal dpr = 1.0;
    if (auto* screen = QGuiApplication::primaryScreen())
        dpr = screen->devicePixelRatio();

    const QColor bg = palette().color(QPalette::Window);
    const QString logoPath = (bg.lightness() < 128)
        ? ":/assets/app_icon_alt.png"
        : ":/assets/app_icon.png";

    QPixmap logo(logoPath);
    QSize target = ui->logoLabel->size() * dpr;
    QPixmap scaled = logo.scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaled.setDevicePixelRatio(dpr);
    ui->logoLabel->setPixmap(scaled);
    ui->logoLabel->setAlignment(Qt::AlignCenter);
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    static bool firstShow = true;
    if (firstShow) {
        firstShow = false;
        QTimer::singleShot(0, this, [this]() { m_viewModel->loadGames(); });
    }
}

void MainWindow::changeEvent(QEvent* event)
{
    QMainWindow::changeEvent(event);
    switch (event->type()) {
    case QEvent::ApplicationPaletteChange:
    case QEvent::PaletteChange:
    case QEvent::ThemeChange:
        m_logoUpdateTimer->start();
        break;
    default:
        break;
    }
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    m_logoUpdateTimer->start();
}

void MainWindow::onAddGameClicked()
{
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Select Executable"));
    if (filePath.isEmpty()) return;
    const QFileInfo fi(filePath);
    m_viewModel->addGame(fi.baseName(), filePath, QString());
}

void MainWindow::rebuildGrid(const QVector<Game>& games)
{
    QWidget* oldContainer = ui->scrollArea->takeWidget();
    if (oldContainer) oldContainer->deleteLater();

    QWidget* container = new QWidget;
    QGridLayout* layout = new QGridLayout(container);
    ui->scrollArea->setWidget(container);
    ui->gridLayout = layout;
    m_selectedView = nullptr;

    const int tileSize = 50 + ui->gridSizeDial->value() * 50;
    const int available = ui->scrollArea->viewport()->width();
    const int columns = qMax(1, available / tileSize);
    m_gridColumns = columns;

    int row = 0, col = 0;
    for (const Game& g : games) {
        auto* view = new GameWidgetView(this);
        view->setTitleVisible(m_showTitles);
        view->setFixedWidth(tileSize);
        view->setGame(g);
        connect(view, &GameWidgetView::removeRequested,      this, &MainWindow::onRemoveGame);
        connect(view, &GameWidgetView::launchRequested,      this, &MainWindow::onLaunchGame);
        connect(view, &GameWidgetView::showFileRequested,    this, &MainWindow::onShowFile);
        connect(view, &GameWidgetView::setCoverRequested,    this, &MainWindow::onSetCoverImage);
        connect(view, &GameWidgetView::removeCoverRequested, this, &MainWindow::onRemoveCoverImage);
        const int idx = row * columns + col;
        connect(view, &GameWidgetView::clicked, this, [this, idx]() {
            selectIndex(idx);
        });
        layout->addWidget(view, row, col);
        if (++col >= columns) { col = 0; ++row; }
    }

    // Restore selection if still valid; otherwise default to first item.
    if (!games.isEmpty()) {
        const int target = (m_selectedIndex >= 0 && m_selectedIndex < games.size())
            ? m_selectedIndex : 0;
        selectIndex(target);
    } else {
        m_selectedIndex = -1;
    }
}

void MainWindow::selectIndex(int index)
{
    if (index < 0 || index >= m_lastGames.size()) return;
    if (m_selectedView) m_selectedView->setSelected(false);

    QLayoutItem* item = ui->gridLayout ? ui->gridLayout->itemAt(index) : nullptr;
    auto* view = item ? qobject_cast<GameWidgetView*>(item->widget()) : nullptr;
    if (!view) return;

    m_selectedView = view;
    m_selectedIndex = index;
    view->setSelected(true);
    ui->scrollArea->ensureWidgetVisible(view);
}

void MainWindow::moveSelection(int rowDelta, int colDelta)
{
    if (m_lastGames.isEmpty()) return;
    if (!ui->scrollArea->isVisible()) return; // grid-only for now

    const int columns = qMax(1, m_gridColumns);
    int idx = (m_selectedIndex < 0) ? 0 : m_selectedIndex;
    int next = idx + rowDelta * columns + colDelta;
    next = qBound(0, next, m_lastGames.size() - 1);
    selectIndex(next);
}

void MainWindow::launchSelected()
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_lastGames.size()) return;
    onLaunchGame(m_lastGames.at(m_selectedIndex).id);
}

void MainWindow::applyCurrentFilter()
{
    const QString text = ui->search_bar->text();
    QVector<Game> filtered;
    if (text.isEmpty()) {
        filtered = m_allGames;
    } else {
        filtered.reserve(m_allGames.size());
        for (const Game& g : m_allGames) {
            if (g.title.contains(text, Qt::CaseInsensitive))
                filtered.append(g);
        }
    }
    m_lastGames = filtered;

    if (ui->listView->isVisible()) {
        m_listView->setGames(filtered);
    } else {
        rebuildGrid(filtered);
    }
}

void MainWindow::onGamesLoaded(const QVector<Game>& games)
{
    m_allGames = games;
    applyCurrentFilter();
}

void MainWindow::onRemoveGame(int gameId)        { m_viewModel->removeGame(gameId); }
void MainWindow::onLaunchGame(int gameId)        { m_viewModel->launchGame(gameId); }
void MainWindow::onShowFile(int gameId)          { m_viewModel->showGameFile(gameId); }
void MainWindow::onRemoveCoverImage(int gameId)  { m_viewModel->removeCoverImage(gameId); }

void MainWindow::onSetCoverImage(int gameId)
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Select Cover Image"));
    if (path.isEmpty()) return;
    m_viewModel->setCoverImage(gameId, path);
    // List view's setGames is called below via the gamesChanged reload.
}

void MainWindow::onSettingsClicked()
{
    SettingsDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    const QStringList folders = dlg.scanFolders();
    QSettings().setValue("scanFolders", folders);
    for (const QString& folder : folders)
        m_viewModel->scanFolder(folder);
}

void MainWindow::onGridSizeChanged(int /*value*/)
{
    QSettings().setValue("gridSize", ui->gridSizeDial->value());
    applyCurrentFilter();
}

void MainWindow::onListViewClicked()
{
    ui->scrollArea->setVisible(false);
    ui->listView->setVisible(true);
    ui->title_toggle_button->setVisible(false);
    ui->gridSizeDial->setVisible(false);
    m_listView->setGames(m_lastGames);
}

void MainWindow::onGridViewClicked()
{
    ui->listView->setVisible(false);
    ui->scrollArea->setVisible(true);
    ui->title_toggle_button->setVisible(true);
    ui->gridSizeDial->setVisible(true);
    applyCurrentFilter();
}

void MainWindow::onTitleToggleClicked()
{
    m_showTitles = !m_showTitles;
    persistShowTitles();
    for (int i = 0; i < ui->gridLayout->count(); ++i) {
        if (auto* item = ui->gridLayout->itemAt(i)) {
            if (auto* w = qobject_cast<GameWidgetView*>(item->widget()))
                w->setTitleVisible(m_showTitles);
        }
    }
}

void MainWindow::persistShowTitles() const
{
    QSettings().setValue("showTitles", m_showTitles);
}

void MainWindow::onSearchTextChanged(const QString& /*text*/)
{
    applyCurrentFilter();
}

void MainWindow::onControllerSettingsClicked()
{
    auto* view = new ControllerConfigView(m_controllers, this);
    view->setAttribute(Qt::WA_DeleteOnClose);
    view->show();
}
