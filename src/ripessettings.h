#pragma once

#include <QColor>
#include <QSettings>

namespace Ripes {

// Definitions of the name of all settings within Ripes
#define RIPES_SETTING_REWINDSTACKSIZE ("simulator_rewindstacksize")
#define RIPES_SETTING_CCPATH ("compiler_path")
#define RIPES_SETTING_CCARGS ("compiler_args")
#define RIPES_SETTING_CONSOLEECHO ("console_echo")
#define RIPES_SETTING_CONSOLEBG ("console_bg_color")
#define RIPES_SETTING_CONSOLEFONT ("console_font_color")

// Definitions of all default settings within Ripes
const static std::map<QString, QVariant> s_defaultSettings = {
    {RIPES_SETTING_REWINDSTACKSIZE, 100},
    {RIPES_SETTING_CCPATH, ""},
    {RIPES_SETTING_CCARGS, "-O0"},
    {RIPES_SETTING_CONSOLEECHO, "true"},
    {RIPES_SETTING_CONSOLEBG, QColor(Qt::white)},
    {RIPES_SETTING_CONSOLEFONT, QColor(Qt::black)},
};

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
