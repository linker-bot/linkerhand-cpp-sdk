/*
 * @Author: liangshaoteng liangshaoteng2012@163.com
 * @Date: 2026-01-29 17:01:44
 * @LastEditors: liangshaoteng liangshaoteng2012@163.com
 * @LastEditTime: 2026-03-09 13:19:16
 * @FilePath: /linkerhand/include/CanBusFactory.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
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
                return std::make_unique<linkerhand::communication::CanBus>(interfaceOrChannel, bitrate, linkerHand);

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
