#pragma once

#include <QString>

namespace Ripes {

/**
 * @brief The CCManager class
 * Manages the detection, verification and execution of a valid C/C++ compiler suitable for the ISAs targetted by the
 * various processor models of Ripes.
 */
class CCManager {
public:
    static CCManager& get() {
        static CCManager manager;
        return manager;
    }

private:
    /**
     * @brief tryAutodetectCC
     * Will attempt to scan the current PATH to locate a valid compiler
     */
    QString tryAutodetectCC();

    /**
     * @brief verifyCC
     * Attempts to compile a simple test program using the provided compiler path @param CC
     */
    void verifyCC(const QString& CC);

    CCManager();
};

}  // namespace Ripes
