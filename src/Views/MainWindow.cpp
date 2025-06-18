#include "Views/MainWindow.h"
#include "ui_MainWindow.h"
#include "ViewModels/GameListViewModel.h"

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