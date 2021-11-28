#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

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

// Helper class to create StatusManagerBase singletons based on a statically defined string.
#define UniqueType(name) \
    struct NamedTypeUniquer__##name {};

template <typename T>
class StatusManagerBase {
public:
    static StatusManagerBase& get() {
        static StatusManagerBase s;
        return s;
    }

    static void setStatus(const QString& s) { get().emitter.setStatus(s); }
    static void setStatusTimed(const QString& s, int ms = 2000) {
        setStatus(s);
        get().m_timer.start(ms);
    }
    static void clearStatus() { get().emitter.clearStatus(); }
    StatusEmitter emitter;

private:
    StatusManagerBase() {
        m_timer.setSingleShot(true);
        m_timer.connect(&m_timer, &QTimer::timeout, [this] { this->clearStatus(); });
    }
    QTimer m_timer;
};

/**
 * @brief StatusManager
 * Defines a singleton class which may be used to send or clear a message to a specific label within the main window
 * status bar. Each status manager is a singleton based on the status manager name.
 */
#define StatusManager(name)                     \
    using name##TypeUniquer = UniqueType(name); \
    using name##StatusManager = StatusManagerBase<name##TypeUniquer>;

// Declarations for the status managers used in Ripes.
StatusManager(General);
StatusManager(ProcessorInfo);
StatusManager(Processor);
StatusManager(SystemIO);
StatusManager(Syscall);

}  // namespace Ripes
