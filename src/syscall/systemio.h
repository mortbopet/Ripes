#pragma once

#include <QObject>

namespace Ripes {

class SystemIO : public QObject {
    Q_OBJECT
public:
    static SystemIO& get() {
        static SystemIO sio;
        return sio;
    }

    static void printString(const QString& string) { emit get().doPrint(string); }

signals:
    void doPrint(const QString&);

private:
    SystemIO() {}
};

}  // namespace Ripes
