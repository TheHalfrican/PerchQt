#include "Views/MainWindow.h"
#include "ui_MainWindow.h"
#include "ViewModels/GameListViewModel.h"

#include "Views/GameWidgetView.h"
#include "Models/Game.h"
#include <QLayoutItem>

#include <QFileDialog>
#include <QFileInfo>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_viewModel(new GameListViewModel(this))
{
    ui->setupUi(this);

    // Hook up the model
    ui->listView->setModel(m_viewModel->gameListModel());

    // Connect signals
    connect(ui->listView, &QListView::clicked,
            m_viewModel, &GameListViewModel::onGameSelected);
    connect(ui->actionAddGame, &QAction::triggered,
            this, &MainWindow::onAddGameClicked);

    connect(m_viewModel,
            &GameListViewModel::gamesChanged,
            this,
            &MainWindow::onGamesLoaded);

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

    // Let user pick an optional cover image
    QString coverPath = QFileDialog::getOpenFileName(this,
                                                     "Select Cover Image");

    // Delegate to ViewModel
    m_viewModel->addGame(title, filePath, coverPath);
}

void MainWindow::onGamesLoaded(const QVector<Game>& games)
{
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
        view->setTitle(g.title);
        view->setCoverArt(g.coverPath);
        ui->gridLayout->addWidget(view, row, col);
        if (++col >= columns) {
            col = 0;
            ++row;
        }
    }
}