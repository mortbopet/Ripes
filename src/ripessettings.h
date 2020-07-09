#pragma once

#include <QColor>
#include <QFont>
#include <QSettings>

namespace Ripes {

// =========== Definitions of the name of all settings within Ripes ============
// User-modifyable settings
#define RIPES_SETTING_REWINDSTACKSIZE ("simulator_rewindstacksize")
#define RIPES_SETTING_CCPATH ("compiler_path")
#define RIPES_SETTING_CCARGS ("compiler_args")
#define RIPES_SETTING_LDARGS ("linker_args")
#define RIPES_SETTING_CONSOLEECHO ("console_echo")
#define RIPES_SETTING_CONSOLEBG ("console_bg_color")
#define RIPES_SETTING_CONSOLEFONTCOLOR ("console_font_color")
#define RIPES_SETTING_CONSOLEFONT ("console_font")

// Program state preserving settings
#define RIPES_SETTING_SETTING_TAB ("settings_tab")
#define RIPES_SETTING_VIEW_ZOOM ("view_zoom")
#define RIPES_SETTING_PROCESSOR_ID ("processor_id")
#define RIPES_SETTING_PROCESSOR_LAYOUT_ID ("processor_layout_id")
#define RIPES_SETTING_FOLLOW_EXEC ("follow_execution")

// ============= Definitions of all default settings within Ripes ==============
const static std::map<QString, QVariant> s_defaultSettings = {
    // User-modifyable settings
    {RIPES_SETTING_REWINDSTACKSIZE, 100},
    {RIPES_SETTING_CCPATH, ""},
    {RIPES_SETTING_CCARGS, "-O0"},
    {RIPES_SETTING_LDARGS, "-static-libgcc -lm"},  // Ensure statically linked executable + link with math library
    {RIPES_SETTING_CONSOLEECHO, "true"},
    {RIPES_SETTING_CONSOLEBG, QColor(Qt::white)},
    {RIPES_SETTING_CONSOLEFONTCOLOR, QColor(Qt::black)},
    {RIPES_SETTING_CONSOLEFONT, QVariant() /* Let Console define its own default font */},
    {RIPES_SETTING_CONSOLEFONT, QColor(Qt::black)},

    // Program state preserving settings
    {RIPES_SETTING_SETTING_TAB, 0},
    {RIPES_SETTING_VIEW_ZOOM, 250},
    {RIPES_SETTING_PROCESSOR_ID, QVariant() /* Let processorhandler define default */},
    {RIPES_SETTING_PROCESSOR_LAYOUT_ID, 0},
    {RIPES_SETTING_FOLLOW_EXEC, "true"}};

/**
 * @brief The SettingObserver class
 * A wrapper class around a single Ripes setting.
 * Provides a slot for setting the setting and a signal for broadcasting that the setting was modified. Useful for
 * propagating settings changes to all over the codebase.
 */
class SettingObserver : public QObject {
    Q_OBJECT
    friend class RipesSettings;

public:
    SettingObserver(const QString& key) : m_key(key) {}
    QVariant value() const;

signals:
    void modified(const QVariant& value);

public slots:
    void setValue(const QVariant& value);

private:
    QString m_key;
};

class RipesSettings : public QSettings {
public:
    static void setValue(const QString& key, const QVariant& value);
    static QVariant value(const QString& key);

    /**
     * @brief getObserver
     * returns the observer for the given setting key @p key
     */
    static SettingObserver* getObserver(const QString& key);

private:
    static RipesSettings& get() {
        static RipesSettings inst;
        return inst;
    }
    RipesSettings();
    ~RipesSettings();

    std::map<QString, SettingObserver> m_observers;
};

}  // namespace Ripes
