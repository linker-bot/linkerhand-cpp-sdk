#ifndef CAN_BUS_FACTORY_H
#define CAN_BUS_FACTORY_H

// 历史名兼容：CanBusFactory 已重命名为 CommFactory（实际可创建 CanBus / PCANBus
// / CanFD / Modbus / EtherCAT）。本头保留作为零代价转发，避免破坏已发布客户代码。
// 推荐新代码直接 include "CommFactory.h" 并使用 Communication::CommFactory。
#include "communication/CommFactory.h"

namespace linkerhand {
namespace communication {
    using CanBusFactory = CommFactory;
}  // namespace communication
}  // namespace linkerhand

namespace Communication {
    using CanBusFactory = ::linkerhand::communication::CommFactory;
}

#endif  // CAN_BUS_FACTORY_H
