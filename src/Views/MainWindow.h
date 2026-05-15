#ifndef PERCHQT_MAINWINDOW_H
#define PERCHQT_MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>
#include <QString>
#include <QVector>

#include "Models/Game.h"

class QTimer;
class QEvent;
class QShowEvent;
class QResizeEvent;
class GameListViewModel;
class GameWidgetView;
class GameListView;
class ControllerManager;
namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onAddGameClicked();
    void onGamesLoaded(const QVector<Game>& games);
    void onRemoveGame(int gameId);
    void onLaunchGame(int gameId);
    void onShowFile(int gameId);
    void onSetCoverImage(int gameId);
    void onRemoveCoverImage(int gameId);
    void onSettingsClicked();
    void onGridSizeChanged(int columns);
    void onListViewClicked();
    void onGridViewClicked();
    void onTitleToggleClicked();
    void onSearchTextChanged(const QString& text);
    void onControllerSettingsClicked();

private:
    void updateAppLogo();
    void rebuildGrid(const QVector<Game>& games);
    void applyCurrentFilter();
    void persistShowTitles() const;
    void selectIndex(int index);
    void moveSelection(int rowDelta, int colDelta);
    void launchSelected();

    Ui::MainWindow* ui{nullptr};
    GameListViewModel* m_viewModel{nullptr};
    ControllerManager* m_controllers{nullptr};
    QPointer<GameWidgetView> m_selectedView;
    QVector<Game> m_lastGames;       // currently displayed (post-filter)
    QVector<Game> m_allGames;        // unfiltered master list
    GameListView* m_listView{nullptr};
    QTimer* m_logoUpdateTimer{nullptr};
    int m_selectedIndex{-1};
    int m_gridColumns{1};
    bool m_showTitles{true};
};

#endif // PERCHQT_MAINWINDOW_H
