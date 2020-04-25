#pragma once

#include <QSettings>

namespace Ripes {

// Definitions of the name of all settings within Ripes
#define RIPES_SETTING_REWINDSTACKSIZE ("simulator_rewindstacksize")
#define RIPES_SETTING_CCPATH ("compiler_path")

// Definitions of all default settings within Ripes
const static std::map<QString, QVariant> s_defaultSettings = {{RIPES_SETTING_REWINDSTACKSIZE, 100},
                                                              {RIPES_SETTING_CCPATH, ""}};

/**
 * @brief The SettingObserver class
 * A wrapper class around a single Ripes setting.
 * Provides a slot for setting the setting and a signal for broadcasting that the setting was modified.
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
    static const SettingObserver* getObserver(const QString& key);

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
