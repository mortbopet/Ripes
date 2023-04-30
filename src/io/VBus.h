#ifndef VBUS_H
#define VBUS_H

// Virtual BUS Protocol ==================
#include <stdint.h>

namespace VBUS {
struct CmdHeader {
    uint32_t msg_type;
    uint32_t payload_size;
    uint64_t time;
};

enum { VB_PINFO = 1, VB_PWRITE, VB_PREAD, VB_PSTATUS, VB_QUIT, VB_SYNC, VB_LAST };

#if __BIG_ENDIAN__
#define htonll(x) (x)
#define ntohll(x) (x)
#else
#define htonll(x) (((uint64_t)htonl((x)&0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) (((uint64_t)ntohl((x)&0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif

}  // namespace VBUS
//=========================================

#endif /* VBUS_H */
