#ifndef I_CAN_BUS_H
#define I_CAN_BUS_H

#include <vector>
#include <cstdint>
#include <string>
#include "core/Common.h"
#include "communication/CanFrame.h" // 包含公共 CAN 帧声明

namespace linkerhand {
namespace communication
{
    class ICanBus
    {
    public:
        virtual ~ICanBus() = default;

        virtual void send(const std::vector<uint8_t>& data, uint32_t can_id, const bool wait = false) = 0;
        virtual CANFrame recv() = 0;
    };
}  // namespace communication
}  // namespace linkerhand

// 向后兼容别名：保留 Communication::ICanBus
namespace Communication
{
    using ICanBus = ::linkerhand::communication::ICanBus;
}

#endif // I_CAN_BUS_H
