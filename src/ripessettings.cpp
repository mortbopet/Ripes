#include "ripessettings.h"

#include <QCoreApplication>
#include <map>

namespace Ripes {

void SettingObserver::setValue(const QVariant& v) {
    QSettings settings;
    Q_ASSERT(settings.contains(m_key));
    settings.setValue(m_key, v);

    emit modified(value());
}

QVariant SettingObserver::value() const {
    QSettings settings;
    Q_ASSERT(settings.contains(m_key));
    return settings.value(m_key);
}

RipesSettings::~RipesSettings() {}

RipesSettings::RipesSettings() {
    // Create a global organization & application name for QSettings to refer to
    QCoreApplication::setOrganizationName("Ripes");
    QCoreApplication::setOrganizationDomain("https://github.com/mortbopet/Ripes");
    QCoreApplication::setApplicationName("Ripes");

    // Populate settings with default values if settings value is not found
    QSettings settings;
    for (const auto& setting : s_defaultSettings) {
        if (!settings.contains(setting.first)) {
            settings.setValue(setting.first, setting.second);
        }
    }

    // Create observer objects for each setting
    for (const auto& setting : s_defaultSettings) {
        m_observers.emplace(setting.first, setting.first);
    }
}

SettingObserver* RipesSettings::getObserver(const QString& key) {
    return &get().m_observers.at(key);
}

void RipesSettings::setValue(const QString& key, const QVariant& value) {
    get().m_observers.at(key).setValue(value);
}

QVariant RipesSettings::value(const QString& key) {
    return get().m_observers.at(key).value();
}

}  // namespace Ripes
