#pragma once

#include <QObject>
#include <QString>

namespace Ripes {
class StatusEmitter : public QObject {
    Q_OBJECT
public:
    void setStatus(const QString& s) { emit statusChanged(s); }
    void clearStatus() { emit clear(); }

signals:
    void statusChanged(const QString&);
    void clear();
};
}  // namespace Ripes

/**
 * @brief StatusManager
 * Defines a singleton class which may be used to send or clear a message to a specific label within the main window
 * status bar.
 */
#define StatusManager(name)                                                     \
    class name##StatusManager {                                                 \
    public:                                                                     \
        static name##StatusManager& get() {                                     \
            static name##StatusManager s;                                       \
            return s;                                                           \
        }                                                                       \
                                                                                \
        static void setStatus(const QString& s) { get().emitter.setStatus(s); } \
        static void clearStatus() { get().emitter.clearStatus(); }              \
        StatusEmitter emitter;                                                  \
                                                                                \
    private:                                                                    \
        name##StatusManager() {}                                                \
    };
