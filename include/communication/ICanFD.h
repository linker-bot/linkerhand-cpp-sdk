#ifndef I_CANFD_H
#define I_CANFD_H

#include <vector>
#include <cstdint>
#include <string>
#include "core/Common.h"

namespace linkerhand {
namespace communication
{
    struct CanFDFrame
    {
        bool valid;             // 帧是否有效
        uint32_t can_id;        // CAN FD 帧 ID
        uint8_t can_dlc;        // DLC (0-15)
        uint8_t data[64];       // 数据
        uint8_t frame_type;     // 帧类型
        uint8_t extern_flag;    // 扩展帧标识
    };

    class ICanFD
    {
    public:
        virtual ~ICanFD() = default;

        virtual void send(const std::vector<uint8_t>& data, uint32_t can_id, bool is_extended = true) = 0;
        virtual CanFDFrame recv(int timeout_ms = 100) = 0;
        virtual bool isOpen() const = 0;
    };
}  // namespace communication
}  // namespace linkerhand

// 向后兼容别名
namespace Communication
{
    using CanFDFrame = ::linkerhand::communication::CanFDFrame;
    using ICanFD     = ::linkerhand::communication::ICanFD;
}

#endif // I_CANFD_H