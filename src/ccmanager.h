#pragma once

#include <QObject>
#include <QString>

namespace Ripes {

/**
 * @brief The CCManager class
 * Manages the detection, verification and execution of a valid C/C++ compiler suitable for the ISAs targetted by the
 * various processor models of Ripes.
 */
class CCManager : public QObject {
    Q_OBJECT
public:
    static CCManager& get() {
        static CCManager manager;
        return manager;
    }

    bool hasValidCC() const;
    const QString& currentCC() const { return m_currentCC; }

signals:
    /**
     * @brief ccChanged
     * Emitted whenever the current CC path changed. @param valid indicates whether the CC was successfully validated.
     */
    void ccChanged(bool valid);

public slots:
    /**
     * @brief trySetCC
     * Attempts to set the path @param CC to be the current compiler.
     * @return true if the CC was successfully validated and set
     */
    bool trySetCC(const QString& CC);

private:
    /**
     * @brief tryAutodetectCC
     * Will attempt to scan the current PATH to locate a valid compiler
     */
    QString tryAutodetectCC();

    /**
     * @brief verifyCC
     * Attempts to compile a simple test program using the provided compiler path @param CC.
     * @returns true if successful.
     */
    bool verifyCC(const QString& CC);

    CCManager();

    QString m_currentCC;
};

}  // namespace Ripes
