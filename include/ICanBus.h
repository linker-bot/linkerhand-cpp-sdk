#ifndef LINKERHAND_ICANBUS_H
#define LINKERHAND_ICANBUS_H

#include <vector>
#include <cstdint>
#include <string>
#include "Common.h"
#include "CanFrame.h"

namespace linkerhand {
namespace communication {

/**
 * @brief CAN 总线接口
 * 
 * 定义 CAN 总线通信的标准接口
 */
class ICanBus {
public:
    virtual ~ICanBus() = default;

    /**
     * @brief 发送 CAN 帧
     * @param data 要发送的数据
     * @param can_id CAN 帧 ID
     * @param wait 是否等待发送完成（默认 false）
     */
    virtual void send(const std::vector<uint8_t>& data, uint32_t can_id, 
                     const bool wait = false) = 0;
    
    /**
     * @brief 接收 CAN 帧
     * @param id CAN 帧 ID（输出参数）
     * @return 接收到的 CAN 帧
     */
    virtual CANFrame recv(uint32_t& id) = 0;
};

} // namespace communication
} // namespace linkerhand

// 向后兼容的别名
namespace Communication = linkerhand::communication;

#endif // LINKERHAND_ICANBUS_H
