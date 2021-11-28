#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <memory>

namespace Ripes {
class StatusEmitter : public QObject {
    Q_OBJECT
public:
    void setStatus(const QString& s) {
        // Ensure that status messages are not multiline - this will expand the bottom status bar in the main window,
        // which we don't want.
        QString singleLineStatus = s;
        singleLineStatus.replace("\n", " ");
        emit statusChanged(singleLineStatus);
    }
    void clearStatus() { emit clear(); }

signals:
    void statusChanged(const QString&);
    void clear();
};

// Helper class to create StatusManagerBase singletons based on a statically defined string.
#define UniqueType(name) \
    struct NamedTypeUniquer__##name {};

/**
 * @brief StatusManager
 * Defines a singleton class which may be used to send or clear a message to a specific label within the main window
 * status bar. Each status manager is a singleton based on the status manager name.
 */
template <typename T>
class StatusManagerBase {
    struct TimedMessage {
        int time = -1;
        QString message;
    };

public:
    static StatusManagerBase& get() {
        static StatusManagerBase s;
        return s;
    }

    static void setPermanent() { get().isPermanent = true; }

    static void setStatusPermanent(const QString& s) {
        auto& me = get();
        assert(me.isPermanent);
        clearStatus();
        me.statusQueue.insert(0, TimedMessage());
        auto& message = me.statusQueue[0];
        message.message = s;
        me.emitCurrentStatus();
    }

    // Pushes a message to the message queue of this status manager. If ms is set and >0, the message will be removed
    // after ms milliseconds.
    static void setStatusTimed(const QString& s, int ms = 1000) {
        auto& me = get();
        assert(!me.isPermanent && "Cannot set timed messages on permanent status managers.");
        me.statusQueue.insert(0, TimedMessage());
        auto& message = me.statusQueue[0];
        message.message = s;
        message.time = ms;
        me.emitCurrentStatus();
    }

    static void clearStatus() {
        auto& me = get();
        if (!me.statusQueue.empty())
            me.statusQueue.pop_back();
        if (!me.statusQueue.empty())
            me.emitCurrentStatus();
        else
            me.emitter.clearStatus();
    }
    StatusEmitter emitter;

private:
    void emitCurrentStatus() {
        if (m_timer.isActive())
            return;
        auto& curMsg = statusQueue.back();
        emitter.setStatus(curMsg.message);
        if (curMsg.time > 0)
            m_timer.start(curMsg.time);
    }
    StatusManagerBase() {
        m_timer.setSingleShot(true);
        m_timer.connect(&m_timer, &QTimer::timeout, [this] { this->clearStatus(); });
    }
    QTimer m_timer;
    QList<TimedMessage> statusQueue;

    // If true, does not allow timed status messages and disables the status queue.
    bool isPermanent;
};

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
