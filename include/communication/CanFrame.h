#ifndef CAN_FRAME_H
#define CAN_FRAME_H

#include <iostream>
#include <cstdint>
#include <vector>

#ifdef _WIN32
// 定义 NOMINMAX 以防止 Windows SDK 定义 min/max 宏与 std::min/std::max 冲突
#define NOMINMAX
// 重命名 PCANBasic.h 中与 Hcanbus.h 冲突的函数
#define CAN_Reset PCAN_Reset
#define CAN_GetStatus PCAN_GetStatus
#define CAN_Init PCAN_Init
#define CAN_Transmit PCAN_Transmit
#define CAN_TransmitRt PCAN_TransmitRt
#define CAN_GetReceiveNum PCAN_GetReceiveNum
#define CAN_Receive PCAN_Receive
#define CAN_CloseDevice PCAN_CloseDevice
#define CANFD_Init PCANFD_Init
#define CANFD_Transmit PCANFD_Transmit
#define CANFD_TransmitRt PCANFD_TransmitRt
#define CANFD_GetReceiveNum PCANFD_GetReceiveNum
#define CANFD_Receive PCANFD_Receive
#include "PCANBasic.h"
// 恢复原始函数名
#undef CAN_Reset
#undef CAN_GetStatus
#undef CAN_Init
#undef CAN_Transmit
#undef CAN_TransmitRt
#undef CAN_GetReceiveNum
#undef CAN_Receive
#undef CAN_CloseDevice
#undef CANFD_Init
#undef CANFD_Transmit
#undef CANFD_TransmitRt
#undef CANFD_GetReceiveNum
#undef CANFD_Receive
using CanFrame = TPCANMsg;
#elif defined(__linux__)
#include <linux/can.h>
#include <linux/can/raw.h>
using CanFrame = struct can_frame;
#elif defined(__APPLE__) && defined(__MACH__)
std::cout << "This is macOS!" << std::endl;
#else
#error "Unsupported platform"
#endif

// 公共的 CAN 帧结构体
struct CANFrame
{
    uint32_t can_id;        // CAN 帧 ID
    uint8_t can_dlc;        // 数据长度
    uint8_t data[8];    // 数据
};


#endif // CAN_FRAME_H
