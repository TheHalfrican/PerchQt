

#ifndef THEMES_H
#define THEMES_H

#include <QString>
#include <QPalette>
#include <QColor>

class Themes {
public:
    enum class Theme {
        SystemDefault,
        Light,
        Dark,
        LavenderTeal,
        Xbox360,
        Custom
    };

    // Apply by enum
    static void applyTheme(Theme theme);
    // Apply by name (e.g. "Dark", "Custom")
    static void applyTheme(const QString& themeName);

private:
    // Builds the QPalette for a given theme
    static QPalette paletteForTheme(Theme theme);
    // Builds a custom palette given colors (hex or named)
    static QPalette customPalette(const QString& bgColor,
                                  const QString& textColor,
                                  const QString& accentColor);
};

#endif // THEMES_H