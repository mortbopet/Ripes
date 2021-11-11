
#ifndef VBUS_H
#define VBUS_H

// Virtual BUS Protocol ==================
#include <stdint.h>

typedef struct {
    uint32_t msg_type;
    uint32_t payload_size;
} cmd_header_t;

enum { VB_PINFO = 1, VB_PWRITE, VB_PREAD, VB_PSTATUS, VB_DMAWR, VB_DMARD, VB_LAST };
//=========================================

#endif /* VBUS_H */
