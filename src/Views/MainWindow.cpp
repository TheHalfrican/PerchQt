

#include "Views/MainWindow.h"
#include "ViewModels/GameListViewModel.h"

#include <QListView>
#include <QWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_viewModel(new GameListViewModel(this))
    , m_listView(new QListView(this))
{
    setupUi();
    setupConnections();
    // Load games (adjust config path logic in ViewModel as needed)
    m_viewModel->loadGames();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    setWindowTitle("PerchQt");

    // Central widget and layout
    auto* central = new QWidget(this);
    auto* layout  = new QVBoxLayout(central);
    layout->addWidget(m_listView);
    setCentralWidget(central);

    // Set the model on the list view
    m_listView->setModel(m_viewModel->gameListModel());
}

void MainWindow::setupConnections()
{
    // When an item is clicked, notify the ViewModel
    connect(m_listView, &QListView::clicked,
            m_viewModel, &GameListViewModel::onGameSelected);

    // Example: handle gameLaunched signal
    // connect(m_viewModel, &GameListViewModel::gameLaunched,
    //         this, [](const Game& game) {
    //             // TODO: implement game launch handling
    //         });
}