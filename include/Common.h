#ifndef COMMON_H
#define COMMON_H

enum LINKER_HAND {
    O6,
    L6,
    L7,
    L10,
    L20,
    L21,
    L25
};

enum HAND_TYPE {
    LEFT = 0x28,
    RIGHT = 0x27
};

enum COMM_TYPE {
    COMM_CAN_0,
    COMM_CAN_1,
    COMM_MODBUS,
    COMM_ETHERCAT
};

namespace Common
{
    inline float current_hand_version = 1.0;
}
#define SEND_DEBUG 0
#define RECV_DEBUG 0

#endif // COMMON_H
