#ifndef PERCHQT_MAINWINDOW_H
#define PERCHQT_MAINWINDOW_H

#include <QMainWindow>
#include "ui_MainWindow.h"
#include <QVector>
#include "Models/Game.h"
#include <QString>
#include <QShowEvent>
#include <QEvent>
#include <QResizeEvent>
#include <QPointer>

// Forward declarations
class GameListViewModel;
class GameWidgetView;
class GameListView;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    // Constructor
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    // ViewModel driving this window
    GameListViewModel* m_viewModel{nullptr};
    Ui::MainWindow*    ui{nullptr};
    // Currently selected game widget for single selection
    QPointer<GameWidgetView> m_selectedView;
    // Stored games for redisplay when grid size changes
    QVector<Game> m_lastGames;
    // Unfiltered full list of games for search/filtering
    QVector<Game> m_allGames;
    // The alternate list view widget
    GameListView* m_listView{nullptr};
    // Whether game titles are currently shown in the grid
    bool m_showTitles{true};

    // Reload and rescale the app logo when theme or DPI changes
    void updateAppLogo();

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

protected:
    // Handle palette or style changes to update logo dynamically
    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
};

#endif // PERCHQT_MAINWINDOW_H