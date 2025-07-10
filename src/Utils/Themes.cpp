#include "Utils/Themes.h"
#include <QApplication>
#include <QStyleFactory>
#include <QSettings>
#include <QObject>
#include <QStyle>
#include <QPalette>
#include <QColor>

void Themes::applyTheme(Theme theme) {
    if (auto app = qobject_cast<QApplication*>(QApplication::instance())) {
        // Use Fusion style for consistent palette support
        app->setStyle(QStyleFactory::create("Fusion"));
        app->setPalette(paletteForTheme(theme));
    }
}

void Themes::applyTheme(const QString& themeName) {
    if (themeName.compare("Light", Qt::CaseInsensitive) == 0) {
        applyTheme(Theme::Light);
    } else if (themeName.compare("Dark", Qt::CaseInsensitive) == 0) {
        applyTheme(Theme::Dark);
    } else if (themeName.compare("Lavender Teal", Qt::CaseInsensitive) == 0) {
        applyTheme(Theme::LavenderTeal);
    } else if (themeName.compare("Xbox 360", Qt::CaseInsensitive) == 0 ||
               themeName.compare("Xbox360", Qt::CaseInsensitive) == 0) {
        applyTheme(Theme::Xbox360);
    } else if (themeName.compare("Custom", Qt::CaseInsensitive) == 0) {
        applyTheme(Theme::Custom);
    } else {
        applyTheme(Theme::SystemDefault);
    }
}

QPalette Themes::paletteForTheme(Theme theme) {
    QPalette pal;
    switch (theme) {
        case Theme::SystemDefault:
            if (auto app = qobject_cast<QApplication*>(QApplication::instance())) {
                return app->style()->standardPalette();
            }
            break;
        case Theme::Light: {
            QPalette light;
            light.setColor(QPalette::Window, QColor(245, 245, 245));
            light.setColor(QPalette::WindowText, Qt::black);
            light.setColor(QPalette::Base, QColor(255, 255, 255));
            light.setColor(QPalette::Text, Qt::black);
            light.setColor(QPalette::Button, QColor(245, 245, 245));
            light.setColor(QPalette::ButtonText, Qt::black);
            light.setColor(QPalette::Highlight, QColor(128, 128, 128));
            light.setColor(QPalette::HighlightedText, Qt::white);
            // Placeholder text color for light theme
            light.setColor(QPalette::PlaceholderText, QColor(128, 128, 128));
            return light;
        }
        case Theme::Dark: {
            QPalette dark;
            dark.setColor(QPalette::Window, QColor(0, 0, 0));
            dark.setColor(QPalette::WindowText, Qt::white);
            dark.setColor(QPalette::Base, QColor(18, 18, 18));
            dark.setColor(QPalette::Text, Qt::white);
            dark.setColor(QPalette::Button, QColor(0, 0, 0));
            dark.setColor(QPalette::ButtonText, Qt::white);
            dark.setColor(QPalette::Highlight, QColor(64, 64, 64));
            dark.setColor(QPalette::HighlightedText, Qt::black);
            return dark;
        }
        case Theme::Xbox360: {
            QPalette xbox;
            xbox.setColor(QPalette::Window, QColor(32, 32, 32));
            xbox.setColor(QPalette::WindowText, QColor(200, 200, 200));
            xbox.setColor(QPalette::Base, QColor(24, 24, 24));
            xbox.setColor(QPalette::Text, QColor(200, 200, 200));
            xbox.setColor(QPalette::Button, QColor(48, 48, 48));
            xbox.setColor(QPalette::ButtonText, QColor(200, 200, 200));
            xbox.setColor(QPalette::Highlight, QColor(0, 200, 0));
            xbox.setColor(QPalette::HighlightedText, Qt::black);
            return xbox;
        }
        case Theme::LavenderTeal: {
            QPalette lt;
            lt.setColor(QPalette::Window, QColor(220, 208, 255));
            lt.setColor(QPalette::WindowText, QColor(0, 51, 51));
            lt.setColor(QPalette::Base, QColor(230, 230, 250));
            lt.setColor(QPalette::Text, QColor(0, 51, 51));
            lt.setColor(QPalette::Button, QColor(200, 200, 235));
            lt.setColor(QPalette::ButtonText, QColor(0, 51, 51));
            lt.setColor(QPalette::Highlight, QColor(0, 128, 128));
            lt.setColor(QPalette::HighlightedText, Qt::white);
            // Placeholder text color for lavender teal theme
            lt.setColor(QPalette::PlaceholderText, QColor(80, 80, 80));
            return lt;
        }
        case Theme::Custom: {
            // Load custom colors from QSettings
            QSettings settings("PerchOrg", "PerchQt");
            QString bg = settings.value("Theme/CustomBgColor", "#ffffff").toString();
            QString text = settings.value("Theme/CustomTextColor", "#000000").toString();
            QString accent = settings.value("Theme/CustomAccentColor", "#0078d7").toString();
            return customPalette(bg, text, accent);
        }
    }
    // Fallback to system default
    if (auto app = qobject_cast<QApplication*>(QApplication::instance()))
        return app->style()->standardPalette();
    return pal;
}

QPalette Themes::customPalette(const QString& bgColor,
                               const QString& textColor,
                               const QString& accentColor) {
    QPalette custom;
    custom.setColor(QPalette::Window, QColor(bgColor));
    custom.setColor(QPalette::WindowText, QColor(textColor));
    custom.setColor(QPalette::Base, QColor(bgColor));
    custom.setColor(QPalette::Text, QColor(textColor));
    custom.setColor(QPalette::Button, QColor(bgColor));
    custom.setColor(QPalette::ButtonText, QColor(textColor));
    custom.setColor(QPalette::Highlight, QColor(accentColor));
    custom.setColor(QPalette::HighlightedText, QColor(textColor));
    // Placeholder text color for custom theme
    custom.setColor(QPalette::PlaceholderText, QColor(textColor).darker(150));
    return custom;
}