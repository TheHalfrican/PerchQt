# PerchQt

Qt6/SDL2 desktop game launcher targeting Xbox 360 / Xenia-Canary ROM libraries on Windows. Single-binary app with SQLite persistence, theme support, controller-driven navigation.

## Build

The project supports two toolchains; `build.ps1` auto-detects which is available.

```powershell
# Incremental build, auto-detect toolchain
.\build.ps1

# Force a specific toolchain
.\build.ps1 -Toolchain MinGW   # uses Qt's bundled MinGW 13.1.0 + Ninja, builds to build-mingw/
.\build.ps1 -Toolchain MSVC    # uses VS 2022 Community + vcpkg, builds to build/

# Wipe and reconfigure
.\build.ps1 -Clean

# Debug instead of Release
.\build.ps1 -Config Debug

# Run the executable (builds first if missing)
.\run.ps1
```

CMake's POST_BUILD steps automatically run `windeployqt` and copy `SDL2.dll` next to the exe — no manual deployment needed.

### Toolchain assumptions

- **MSVC path** expects `vcvars64.bat` at the standard VS 2022 Community location and a vcpkg checkout at `C:\Users\NoahM\DevPkgs\vcpkg\` providing Qt6 + SDL2 in the default `x64-windows` triplet.
- **MinGW path** expects Qt 6.9.1 installed at `C:\Qt\6.9.1\mingw_64\` (with `Tools\mingw1310_64\` and `Tools\Ninja\`) and SDL2 MinGW dev release at `C:\Users\NoahM\DevPkgs\SDL2-mingw\SDL2-2.32.8\`.

If either toolchain is missing on a new machine, edit the paths in `build.ps1`.

### Direct CMake (without build.ps1)

```powershell
# MinGW + Ninja
$env:PATH = 'C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;' + $env:PATH
cmake -S . -B build-mingw -G Ninja `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_PREFIX_PATH='C:/Qt/6.9.1/mingw_64;C:/Users/NoahM/DevPkgs/SDL2-mingw/SDL2-2.32.8/x86_64-w64-mingw32' `
    -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw1310_64/bin/g++.exe
cmake --build build-mingw --parallel
```

## Architecture

Layered MVVM-ish, with a single ViewModel mediating between the SQLite-backed model and the widget views.

```
src/
├── main.cpp                    # App entry, sets org/app names, opens DB, applies theme, shows MainWindow
├── Models/
│   └── Game.h                  # Plain data struct (no Qt model class — see "Why no QAbstractListModel" below)
├── Data/
│   ├── Database.h
│   └── Database.cpp            # Opens DB at QStandardPaths::AppDataLocation, runs schema migrations via PRAGMA user_version
├── ViewModels/
│   ├── GameListViewModel.h
│   └── GameListViewModel.cpp   # Single source of mutations: addGame, removeGame, launchGame, scanFolder, setCoverImage, etc.
│                               # Emits gamesChanged(QVector<Game>) and statusMessage(QString)
├── Views/
│   ├── MainWindow.{h,cpp}      # Toolbar, search, grid/list toggle, status bar, controller routing
│   ├── GameWidgetView.{h,cpp}  # A single tile in the grid (cover + title + context menu)
│   ├── GameListView.{h,cpp}    # Table-style alternative view (QTableWidget-based)
│   ├── SettingsDialog.{h,cpp}  # Scan folders, emulator path, theme picker
│   └── ControllerConfigView.{h,cpp}  # Lists connected controllers, opens system Bluetooth pane
├── Input/
│   ├── ControllerManager.h
│   └── ControllerManager.cpp   # Owns SDL2 game-controller subsystem, polls events on a 16ms timer,
│                               # emits Qt signals (moveLeft/Right/Up/Down, confirmPressed, backPressed, menuPressed)
├── Utils/
│   ├── Themes.{h,cpp}          # System/Light/Dark/LavenderTeal/Xbox360/Custom palettes via Fusion style
│   └── PlaceholderImage.h      # Header-only generator for the "(Right-click to set Cover Art)" 2:3 placeholder
├── Widgets/
│   └── MarqueeLabel.{h,cpp}    # QLabel subclass that horizontally scrolls overflow text
├── UI/                         # .ui files for Qt Designer (AUTOUIC processes them)
└── Assets/
    ├── resources.qrc
    └── *.png/*.ico/*.icns      # App icons + toolbar icons
```

### Why no `QAbstractListModel`

There used to be one (`GameListModel`) but neither view consumed it — the grid is widget-based and rebuilt on every refresh, and the list view uses `QTableWidget` with manually-populated rows. Adding it back is fine, but if you do, commit to it: have one of the views actually drive off the model with a `QListView` + delegate, and replace the full-rebuild path in `MainWindow::rebuildGrid` with `dataChanged()` notifications.

### Threading

- **UI thread** owns the default Qt SQL connection. All read paths and most writes go through it.
- **`scanFolder` runs on `QThreadPool` via `QtConcurrent::run`.** The worker thread opens its own named SQLite connection (`scan-<tid>`), wraps inserts in a transaction, then posts results back to the UI thread via `QMetaObject::invokeMethod` with `Qt::QueuedConnection`. SQLite is configured for WAL journaling so concurrent readers don't block the writer.
- **SDL2 controller polling** runs on the UI thread via a `QTimer` inside `ControllerManager`. SDL events drain into Qt signals.

If you add another long-running operation, follow the same pattern: per-thread DB connection, `removeDatabase` when done, queued invoke back to the UI.

### Settings & data locations

- **Settings:** `QSettings` reads/writes `HKCU\Software\PerchOrg\PerchQt` on Windows. Org/app names are set once in `main()`, so all `QSettings()` calls are default-constructed.
- **Database:** `%APPDATA%\PerchOrg\PerchQt\perch.db` (via `QStandardPaths::AppDataLocation`). On first run, if a `perch.db` ships next to the executable (`applicationDirPath()/perch.db`), it's seeded into AppData.
- **Schema:** Tracked via SQLite's `PRAGMA user_version`. `Database::runMigrations` is the single place to add a new schema version. Each migration is a guarded `if (version < N) { ... setSchemaVersion(N); }` block.

### Settings keys in use

| Key | Type | Purpose |
|---|---|---|
| `Theme/CurrentTheme` | string | "System Default" / "Light" / "Dark" / "LavenderTeal" / "Xbox 360" / "Custom" |
| `Theme/CustomBgColor` / `CustomTextColor` / `CustomAccentColor` | string (#hex) | Custom palette colors |
| `gridSize` | int | Position of the grid-size dial |
| `showTitles` | bool | Whether titles appear under grid tiles |
| `scanFolders` | QStringList | Auto-scanned on startup |
| `emulatorPath` | string | Optional emulator (e.g. Xenia) — if set, games are passed to it as args |

## Conventions

- **Default to no comments.** Names should carry the meaning. Add a comment only when the *why* would surprise the next reader (a workaround, a hidden invariant, a non-obvious order-of-operations).
- **Forward-declare in headers, include in .cpp.** Especially for generated `ui_*.h` files.
- **Prefer signals over direct calls** when crossing layer boundaries (ViewModel → View). The ViewModel never knows about widgets.
- **The ViewModel is the only place that mutates the database.** Views call `m_viewModel->doX()` and react to `gamesChanged` / `statusMessage`.
- **Status feedback goes through `GameListViewModel::statusMessage(QString)`,** which `MainWindow` shows in the status bar for ~4s. Don't pop dialogs for transient outcomes.
- **Selection styling is palette-derived.** `GameWidgetView::setSelected` reads `QPalette::Highlight` so it tracks the active theme. Don't hardcode colors in stylesheets.

## Adding a new ROM extension to scan

1. `src/ViewModels/GameListViewModel.cpp` — the `allowedExt` static list inside the `QtConcurrent::run` lambda in `scanFolder`. Add the new extension (lowercase, no dot).
2. There's no UI for this yet; it's a known limitation. A future improvement would be to read the list from `QSettings`.

## Adding a schema migration

In `src/Data/Database.cpp`, `runMigrations()`:

```cpp
if (version < 2) {
    if (!execOrWarn(db, "ALTER TABLE games ADD COLUMN genre TEXT")) return false;
    if (!setSchemaVersion(db, 2)) return false;
    version = 2;
}
```

Each version block must be idempotent and gated on the current `version`. Don't reorder existing blocks.

## Controller mapping

| SDL button / axis | Action |
|---|---|
| D-pad / left stick | Move grid selection (auto-repeat at 110ms after 350ms initial delay) |
| A | Launch selected game |
| B | (reserved — not currently bound) |
| Start | Open Settings dialog |

Hot-plug works: `ControllerManager` opens new controllers on `SDL_CONTROLLERDEVICEADDED` and emits `controllersChanged` so the Controller Config dialog can update its display live.

## Things deliberately not done

These were considered but skipped:

- **Tests / CI** — no test runner is wired up. Worth adding if the data layer grows.
- **Manual game rename** — currently the title is locked to the imported filename's `baseName`.
- **Configurable scan extensions UI** — see "Adding a new ROM extension."
- **Real `QListView` + delegate for the grid** — the current approach rebuilds widget tiles on every reload. Fine for libraries up to a few hundred games; would need attention beyond that.
- **`onTitleToggleClicked` triggers a full grid rebuild** through `applyCurrentFilter()` — small enough that a per-widget `setTitleVisible` loop would also be fine, but the rebuild is what currently happens.
