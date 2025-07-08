

#include "GameListView.h"
#include "ui_GameListView.h"

#include <QTableWidgetItem>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QPixmap>
#include <QIcon>
#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>

GameListView::GameListView(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::GameListView)
{
    ui->setupUi(this);

    // Configure table
    ui->table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->table->verticalHeader()->setVisible(false);
    ui->table->setIconSize(QSize(64, 64)); // adjust as needed

    // Context menu and activation
    connect(ui->table, &QTableWidget::customContextMenuRequested,
            this, &GameListView::onCustomContextMenuRequested);
    connect(ui->table, &QTableWidget::cellActivated,
            this, &GameListView::onCellActivated);
}

GameListView::~GameListView()
{
    delete ui;
}

void GameListView::setGames(const QVector<Game>& games)
{
    m_games = games;
    ui->table->clearContents();
    ui->table->setRowCount(m_games.size());

    for (int row = 0; row < m_games.size(); ++row) {
        const Game& g = m_games[row];

        // Cover icon
        QTableWidgetItem* iconItem = new QTableWidgetItem();
        if (!g.coverPath.isEmpty()) {
            QPixmap pix(g.coverPath);
            iconItem->setIcon(QIcon(pix));
        }
        iconItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->table->setItem(row, 0, iconItem);

        // Title
        QTableWidgetItem* titleItem = new QTableWidgetItem(g.title);
        titleItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->table->setItem(row, 1, titleItem);

        // Last Played
        QTableWidgetItem* lastItem = new QTableWidgetItem(g.lastPlayed);
        lastItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->table->setItem(row, 2, lastItem);

        // Play Count
        QTableWidgetItem* countItem = new QTableWidgetItem(QString::number(g.playCount));
        countItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->table->setItem(row, 3, countItem);
    }
}

void GameListView::onCustomContextMenuRequested(const QPoint& pos)
{
    QModelIndex idx = ui->table->indexAt(pos);
    if (!idx.isValid()) return;
    int row = idx.row();
    int gameId = m_games[row].id;

    QMenu menu(this);
    QAction* launchAction = menu.addAction("Launch Game");
    QAction* showFileAction = menu.addAction("Show File in Browser");
    QAction* setCoverAction = menu.addAction("Set Cover Image...");
    QAction* removeCoverAction = menu.addAction("Remove Cover Image");
    QAction* removeGameAction = menu.addAction("Remove Game");

    QAction* act = menu.exec(ui->table->viewport()->mapToGlobal(pos));
    if      (act == launchAction)     emit launchRequested(gameId);
    else if (act == showFileAction)   emit showFileRequested(gameId);
    else if (act == setCoverAction)   emit setCoverRequested(gameId);
    else if (act == removeCoverAction)emit removeCoverRequested(gameId);
    else if (act == removeGameAction) emit removeRequested(gameId);
}

void GameListView::onCellActivated(int row, int /*column*/)
{
    if (row < 0 || row >= m_games.size()) return;
    emit launchRequested(m_games[row].id);
}