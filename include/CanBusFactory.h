#ifndef LINKERHAND_CAN_BUS_FACTORY_H
#define LINKERHAND_CAN_BUS_FACTORY_H

#include "ICanBus.h"
#include "CanBus.h"
#include "PCANBus.h"
#if USE_ETHERCAT
#include "EtherCAT.h"
#endif

#include <stdexcept>

namespace linkerhand {
namespace communication {
    class CanBusFactory
    {
    public:
        static std::unique_ptr<ICanBus> createCanBus(
            uint32_t handId,
            const std::string& interfaceOrChannel,
            int bitrate,
            const LINKER_HAND linkerHand
        )
        {
            (void)handId;  // 标记为未使用，避免警告

            #ifdef _WIN32
                // Windows 平台
                TPCANHandle channel = PCAN_USBBUS1;
                TPCANBaudrate baudrate = PCAN_BAUD_1M;
                if (interfaceOrChannel == "can1") {
                    channel = PCAN_USBBUS2;
                }
                return std::make_unique<linkerhand::communication::PCANBus>(channel, baudrate, linkerHand);

            #else
                // Linux/Unix 平台
                if (interfaceOrChannel == "can0" || interfaceOrChannel == "can1") {
                    return std::make_unique<linkerhand::communication::CanBus>(interfaceOrChannel, bitrate, linkerHand);
                } 

                #if USE_ETHERCAT
                else if (interfaceOrChannel == "ethercat") {
                    return std::make_unique<linkerhand::communication::EtherCAT>(handId);
                }
                #endif

                // 默认情况：抛出异常或返回 nullptr
                throw std::runtime_error("Unsupported CAN interface: " + interfaceOrChannel);
                // 或者返回 nullptr:
                // return nullptr;

            #endif
        }
    };
} // namespace communication
} // namespace linkerhand

// 向后兼容：在全局命名空间中提供别名
namespace Communication = linkerhand::communication;

#endif // LINKERHAND_CAN_BUS_FACTORY_H
