#ifndef VBUS_H
#define VBUS_H

// Virtual BUS Protocol ==================
#include <stdint.h>

namespace VBUS {
struct CmdHeader {
    uint32_t msg_type;
    uint32_t payload_size;
};

enum { VB_PINFO = 1, VB_PWRITE, VB_PREAD, VB_PSTATUS, VB_DMAWR, VB_DMARD, VB_QUIT, VB_LAST };
}  // namespace VBUS
//=========================================

#endif /* VBUS_H */
