#ifndef COMMON_H
#define COMMON_H

#include <cstdint>

enum LINKER_HAND {
    L6,
    L7,
    L10,
    L20,
    L21,
    L25,
    O6,
    G20,
    O20,
    L30,
    O30,
    L20_10 = G20,
    L20_11 = G20,
    L20_Lite = L10
};

enum HAND_TYPE {
    LEFT = 0x28,
    RIGHT = 0x27
};

enum COMM_TYPE {
    CAN,
    MODBUS,
    ETHERCAT
};

// 读取来源：对应 HOP 协议控制字节的 RTS 位（bit7）。
//   STATE   = 读“实际/运行状态值”（RTS=0，默认，历史行为）
//   SETTING = 读“最近一次成功写入的设定值”（RTS=1）
// 目前 O30(HOP) 的 getPosition/getSpeed 支持；其他型号忽略该参数按实际值处理。
enum ReadSource {
    STATE   = 0,
    SETTING = 1
};

// 新风格枚举（推荐新代码使用）。
// 旧的 LINKER_HAND / HAND_TYPE / COMM_TYPE 保留作向后兼容，不要删。
namespace linkerhand
{

enum class HandModel : uint8_t {
    L6,
    L7,
    L10,
    L20,
    L21,
    L25,
    G20,
    O6,
    O20,
    L30,
    O30,
    L20_10 = G20,
    L20_11 = G20,
    L20_Lite = L10
};

enum class HandType : uint8_t {
    LEFT  = 0x28,
    RIGHT = 0x27
};

enum class CommType : uint8_t {
    CAN,
    MODBUS,
    ETHERCAT
};

inline LINKER_HAND to_legacy(HandModel m) {
    switch (m) {
        case HandModel::L6:  return LINKER_HAND::L6;
        case HandModel::L7:  return LINKER_HAND::L7;
        case HandModel::L10: return LINKER_HAND::L10;
        case HandModel::L20: return LINKER_HAND::L20;
        case HandModel::L21: return LINKER_HAND::L21;
        case HandModel::L25: return LINKER_HAND::L25;
        case HandModel::G20: return LINKER_HAND::G20;
        case HandModel::O6:  return LINKER_HAND::O6;
        case HandModel::O20: return LINKER_HAND::O20;
        case HandModel::L30: return LINKER_HAND::L30;
        case HandModel::O30: return LINKER_HAND::O30;
    }
    return LINKER_HAND::L10;
}

inline HAND_TYPE to_legacy(HandType t) {
    return t == HandType::LEFT ? HAND_TYPE::LEFT : HAND_TYPE::RIGHT;
}

inline COMM_TYPE to_legacy(CommType c) {
    switch (c) {
        case CommType::CAN:      return COMM_TYPE::CAN;
        case CommType::MODBUS:   return COMM_TYPE::MODBUS;
        case CommType::ETHERCAT: return COMM_TYPE::ETHERCAT;
    }
    return COMM_TYPE::CAN;
}

}  // namespace linkerhand

namespace Common
{
    extern float current_hand_version;
}
#define SEND_DEBUG 0
#define RECV_DEBUG 0

#endif // COMMON_H
