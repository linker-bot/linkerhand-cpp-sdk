#ifndef LINKERHAND_COMMON_H
#define LINKERHAND_COMMON_H

// 为了与预编译库兼容，保持旧的枚举定义（非枚举类）
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

namespace linkerhand {

/**
 * @brief 通用配置和版本信息
 */
namespace Common {
    inline float current_hand_version = 1.0;
}

/**
 * @brief 新的枚举类（用于未来版本，当前保持向后兼容）
 * 
 * 注意：当前库使用旧的枚举类型，这些枚举类用于未来版本
 */
enum class HandModel {
    O6,
    L6,
    L7,
    L10,
    L20,
    L21,
    L25
};

enum class HandType {
    LEFT = 0x28,
    RIGHT = 0x27
};

enum class CommType {
    CAN_0,
    CAN_1,
    MODBUS,
    ETHERCAT
};

} // namespace linkerhand

#define SEND_DEBUG 0
#define RECV_DEBUG 0

#endif // LINKERHAND_COMMON_H
