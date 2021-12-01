#include "ripessettings.h"
#include "assembler/program.h"
#include "cachesim/cachesim.h"

#include <QCoreApplication>
#include <map>

namespace Ripes {

// ============= Definitions of all default settings within Ripes ==============
const std::map<QString, QVariant> s_defaultSettings = {
    // User-modifyable settings
    {RIPES_SETTING_REWINDSTACKSIZE, 100},
    {RIPES_SETTING_CCPATH, ""},
    {RIPES_SETTING_FORMATTER_PATH, "clang-format"},
    {RIPES_SETTING_FORMAT_ON_SAVE, false},
    {RIPES_SETTING_FORMATTER_ARGS, "--style=file --fallback-style=LLVM"},
    {RIPES_SETTING_CCARGS, "-O0 -g"},
    {RIPES_SETTING_LDARGS, "-static -lm"},  // Ensure statically linked executable + link with math library
    {RIPES_SETTING_CONSOLEECHO, "true"},
    {RIPES_SETTING_CONSOLEBG, QColorConstants::White},
    {RIPES_SETTING_CONSOLEFONTCOLOR, QColorConstants::Black},
    {RIPES_SETTING_CONSOLEFONT, QVariant() /* Let Console define its own default font */},
    {RIPES_SETTING_CONSOLEFONT, QColorConstants::Black},
    {RIPES_SETTING_INDENTAMT, 4},
    {RIPES_SETTING_UIUPDATEPS, 25},

    {RIPES_SETTING_ASSEMBLER_TEXTSTART, 0x0},
    {RIPES_SETTING_ASSEMBLER_DATASTART, 0x10000000},
    {RIPES_SETTING_ASSEMBLER_BSSSTART, 0x11000000},
    {RIPES_SETTING_PERIPHERALS_START, static_cast<unsigned>(0xF0000000)},
    {RIPES_SETTING_EDITORREGS, true},
    {RIPES_SETTING_EDITORCONSOLE, true},
    {RIPES_SETTING_EDITORSTAGEHIGHLIGHTING, true},

    {RIPES_SETTING_PIPEDIAGRAM_MAXCYCLES, 100},
    {RIPES_SETTING_CACHE_MAXCYCLES, 10000},
    {RIPES_SETTING_CACHE_MAXPOINTS, 1000},
    {RIPES_SETTING_CACHE_PRESETS,
     QVariant::fromValue<QList<CachePreset>>(
         {CachePreset{"32-entry 4-word direct-mapped", 2, 5, 0, WritePolicy::WriteBack, WriteAllocPolicy::WriteAllocate,
                      ReplPolicy::LRU},
          CachePreset{"32-entry 4-word fully associative", 2, 0, 5, WritePolicy::WriteBack,
                      WriteAllocPolicy::WriteAllocate, ReplPolicy::LRU},
          CachePreset{"32-entry 4-word 2-way set associative", 2, 4, 1, WritePolicy::WriteBack,
                      WriteAllocPolicy::WriteAllocate, ReplPolicy::LRU}})},

    // Program state preserving settings
    {RIPES_GLOBALSIGNAL_QUIT, 0},
    {RIPES_GLOBALSIGNAL_REQRESET, 0},

    {RIPES_SETTING_SETTING_TAB, 0},
    {RIPES_SETTING_VIEW_ZOOM, 250},
    {RIPES_SETTING_PERIPHERAL_SETTINGS, ""},
    {RIPES_SETTING_PROCESSOR_ID, QVariant() /* Let processorhandler define default */},
    {RIPES_SETTING_PROCESSOR_LAYOUT_ID, 0},
    {RIPES_SETTING_FOLLOW_EXEC, "true"},
    {RIPES_SETTING_SOURCECODE, ""},
    {RIPES_SETTING_DARKMODE, false},
    {RIPES_SETTING_SHOWSIGNALS, false},
    {RIPES_SETTING_INPUT_TYPE, static_cast<unsigned>(SourceType::Assembly)},
    {RIPES_SETTING_AUTOCLOCK_INTERVAL, 100},

    {RIPES_SETTING_HAS_SAVEFILE, false},
    {RIPES_SETTING_SAVEPATH, ""},
    {RIPES_SETTING_SAVE_SOURCE, true},
    {RIPES_SETTING_SAVE_BINARY, false}};

void SettingObserver::setValue(const QVariant& v) {
    QSettings settings;
    Q_ASSERT(settings.contains(m_key));
    settings.setValue(m_key, v);

    emit modified(value());
}

void SettingObserver::trigger() {
    setValue(value());
}

RipesSettings::~RipesSettings() {}

RipesSettings::RipesSettings() {
    // Create a global organization & application name for QSettings to refer to
    QCoreApplication::setOrganizationName("Ripes");
    QCoreApplication::setOrganizationDomain("https://github.com/mortbopet/Ripes");
    QCoreApplication::setApplicationName("Ripes");

    // Serializer registrations
    qRegisterMetaTypeStreamOperators<CachePreset>("cachePreset");
    qRegisterMetaTypeStreamOperators<QList<CachePreset>>("cachePresetList");

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

}  // namespace Ripes
Q_DECLARE_METATYPE(QList<Ripes::CachePreset>);
