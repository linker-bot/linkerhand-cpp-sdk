#ifndef I_CAN_BUS_H
#define I_CAN_BUS_H

#include <vector>
#include <cstdint>
#include <string>
#include "Common.h"
#include "CanFrame.h" // 包含公共 CAN 帧声明

namespace Communication
{
    class ICanBus
    {
    public:
        virtual ~ICanBus() = default;

        virtual void send(const std::vector<uint8_t>& data, uint32_t can_id, const bool wait = false) = 0;
        virtual CANFrame recv(uint32_t& id) = 0;
    };
}

#endif // I_CAN_BUS_H
