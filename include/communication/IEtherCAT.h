#ifdef __linux__
#ifndef I_ETHER_CAT_H
#define I_ETHER_CAT_H

#include <cstdint>
#include <vector>

#include "communication/CanFrame.h"

namespace linkerhand {
namespace communication {

// EtherCAT 抽象。EtherCAT 不是 CAN，但 SDK 把"CAN over EtherCAT"做成
// 上层握手协议的兼容层，故仍以 CAN 帧作为收发单元。
//
// 与 ICanBus 故意不共享：recv 签名带 out 参数 id，send 行为也由 EtherCAT
// 周期性 PDO 调度驱动而非"立即发送"。强行复用 ICanBus 会让两个接口的语义
// 互相侵蚀（历史代码就是这么写错的）。
class IEtherCAT {
public:
    virtual ~IEtherCAT() = default;

    // 初始化主站、扫描从站、配置 PDO；失败返回 false
    virtual bool init() = 0;

    // 起停周期任务（init 之后调用）
    virtual void start() = 0;
    virtual void stop() = 0;

    // 向 EtherCAT 总线写一帧"CAN 风格"数据；wait=true 时阻塞至 PDO 周期完成
    virtual void send(const std::vector<uint8_t>& data, uint32_t can_id, bool wait = false) = 0;

    // 读一帧；id 通过 out 参数返回
    virtual CANFrame recv(uint32_t& id) = 0;
};

}  // namespace communication
}  // namespace linkerhand

namespace Communication {
    using IEtherCAT = ::linkerhand::communication::IEtherCAT;
}

#endif  // I_ETHER_CAT_H
#endif  // __linux__
