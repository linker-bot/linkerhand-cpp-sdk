#ifndef LINKERHAND_CAN_FRAME_H
#define LINKERHAND_CAN_FRAME_H

#include <iostream>
#include <cstdint>
#include <vector>

#ifdef _WIN32
#include "PCANBasic.h"
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


#endif // LINKERHAND_CAN_FRAME_H