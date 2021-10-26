#include "ioexternalbus.h"
#include "ui_ioexternalbus.h"

#include <QPainter>
#include <QPen>

#include "STLExtras.h"
#include "ioregistry.h"

namespace Ripes {

IOExternalBus::IOExternalBus(QWidget* parent) : IOBase(IOType::EXTERNALBUS, parent), m_ui(new Ui::IOExternalBus) {
    m_ui->setupUi(this);
}

IOExternalBus::~IOExternalBus() {
    unregister();
    delete m_ui;
};

unsigned IOExternalBus::byteSize() const {
    return 1024;
}

QString IOExternalBus::description() const {
    return "An external bus is a memory mapped bus handled through network transactions. The peripheral connects to an "
           "IP address denoting a peripheral server - for more details, refer to the Ripes wiki.";
}

VInt IOExternalBus::ioRead(AInt offset, unsigned size) {
    return 0;
}
void IOExternalBus::ioWrite(AInt offset, VInt value, unsigned size) {}

void IOExternalBus::parameterChanged(unsigned paramId) {}

}  // namespace Ripes
