#ifndef COMM_FACTORY_H
#define COMM_FACTORY_H

#include <memory>
#include <stdexcept>
#include <string>

#include "core/Common.h"
#include "communication/ICanBus.h"
#include "communication/CanBus.h"
#include "communication/PCANBus.h"
#include "communication/ICanFD.h"
#include "communication/CanFD.h"
#include "communication/IModbus.h"
#include "communication/Modbus.h"
#if USE_ETHERCAT
#include "communication/EtherCAT.h"
#include "communication/IEtherCAT.h"
#endif

namespace linkerhand {
namespace communication
{
    // 通信对象工厂。可创建：CanBus / PCANBus / CanFD / Modbus / EtherCAT。
    // 历史名 CanBusFactory 保留为兼容别名，见 CanBusFactory.h。
    class CommFactory
    {
    public:
        // ====================== CAN ======================

        // 按"通道字符串 + 比特率"创建 CAN 总线。
        // - Linux: interfaceOrChannel 直接透传给 SocketCAN（如 "can0" / "vcan0"）
        // - Windows: 解析尾部数字 N，映射到 PCAN_USBBUS{N+1}（"can0"→USBBUS1）；
        //            bitrate 必须为标准值之一：1000000 / 800000 / 500000 / 250000 /
        //            125000 / 100000 / 50000 / 20000
        static std::unique_ptr<ICanBus> createCanBus(const std::string& interfaceOrChannel, const int bitrate)
        {
            if (interfaceOrChannel.empty()) {
                throw std::runtime_error("createCanBus: empty interface name");
            }

            #ifdef _WIN32
                // 解析 canN：取首个数字开始的整数后缀
                const auto pos = interfaceOrChannel.find_first_of("0123456789");
                if (pos == std::string::npos) {
                    throw std::runtime_error(
                        "createCanBus: cannot parse channel index from '" + interfaceOrChannel + "'");
                }
                int idx = -1;
                try {
                    idx = std::stoi(interfaceOrChannel.substr(pos));
                } catch (const std::exception&) {
                    throw std::runtime_error(
                        "createCanBus: invalid channel index in '" + interfaceOrChannel + "'");
                }
                if (idx < 0 || idx > 15) {
                    throw std::runtime_error(
                        "createCanBus: PCAN supports can0..can15 only, got '" + interfaceOrChannel + "'");
                }
                static const TPCANHandle kPcanHandles[16] = {
                    PCAN_USBBUS1,  PCAN_USBBUS2,  PCAN_USBBUS3,  PCAN_USBBUS4,
                    PCAN_USBBUS5,  PCAN_USBBUS6,  PCAN_USBBUS7,  PCAN_USBBUS8,
                    PCAN_USBBUS9,  PCAN_USBBUS10, PCAN_USBBUS11, PCAN_USBBUS12,
                    PCAN_USBBUS13, PCAN_USBBUS14, PCAN_USBBUS15, PCAN_USBBUS16,
                };
                TPCANHandle channel = kPcanHandles[idx];

                TPCANBaudrate baud;
                switch (bitrate) {
                    case 1000000: baud = PCAN_BAUD_1M;   break;
                    case 800000:  baud = PCAN_BAUD_800K; break;
                    case 500000:  baud = PCAN_BAUD_500K; break;
                    case 250000:  baud = PCAN_BAUD_250K; break;
                    case 125000:  baud = PCAN_BAUD_125K; break;
                    case 100000:  baud = PCAN_BAUD_100K; break;
                    case 50000:   baud = PCAN_BAUD_50K;  break;
                    case 20000:   baud = PCAN_BAUD_20K;  break;
                    default:
                        throw std::runtime_error(
                            "createCanBus: unsupported bitrate " + std::to_string(bitrate) +
                            "; valid: 1000000/800000/500000/250000/125000/100000/50000/20000");
                }
                return std::make_unique<PCANBus>(channel, baud);
            #else
                return std::make_unique<CanBus>(interfaceOrChannel, bitrate);
            #endif
        }

        static std::unique_ptr<ICanBus> createCanBus(const HAND_TYPE hand)
        {
            if (hand == HAND_TYPE::LEFT || hand == HAND_TYPE::RIGHT) {
                #ifdef _WIN32
                    return std::make_unique<PCANBus>(hand);
                #else
                    return std::make_unique<CanBus>(hand);
                #endif
            } else {
                throw std::runtime_error("Unsupported: " + hand);
            }
        }

        // ====================== CAN FD ======================
        // CanFD 类跨平台；构造参数 (dev_num, ch_num) 由 third_party libcanbus 定义。
        static std::unique_ptr<ICanFD> createCanFD(uint32_t dev_num, uint32_t ch_num)
        {
            return std::unique_ptr<ICanFD>(new CanFD(dev_num, ch_num));
        }

        // 按手别推断通道号（LEFT→ch0, RIGHT→ch1，dev 固定 0）；
        // 真实硬件如有不同布线，请改用上面带显式参数的重载。
        static std::unique_ptr<ICanFD> createCanFD(const HAND_TYPE hand)
        {
            uint32_t ch = 0;
            if (hand == HAND_TYPE::RIGHT) ch = 1;
            else if (hand != HAND_TYPE::LEFT) {
                throw std::runtime_error("createCanFD: Unsupported HAND_TYPE");
            }
            return std::unique_ptr<ICanFD>(new CanFD(0, ch));
        }

        // ====================== Modbus RTU ======================

        static std::unique_ptr<IModbus> createModbus(const std::string& device,
                                                    int baudrate = 115200,
                                                    char parity = 'N')
        {
            return std::unique_ptr<IModbus>(new Modbus(device, baudrate, parity));
        }

        // 按手别自动检测串口（实现见 Modbus(HAND_TYPE,...) 构造）
        static std::unique_ptr<IModbus> createModbus(HAND_TYPE hand,
                                                    int baudrate = 115200,
                                                    char parity = 'N')
        {
            if (hand != HAND_TYPE::LEFT && hand != HAND_TYPE::RIGHT) {
                throw std::runtime_error("createModbus: Unsupported HAND_TYPE");
            }
            return std::unique_ptr<IModbus>(new Modbus(hand, baudrate, parity));
        }

        // ====================== EtherCAT ======================
        // 仅 Linux + USE_ETHERCAT=ON。未启用编译选项时本方法不声明，
        // 调用站点编译期可见缺失，便于排错。
        #if USE_ETHERCAT
        static std::unique_ptr<IEtherCAT> createEtherCAT(uint32_t handId)
        {
            return std::unique_ptr<IEtherCAT>(new EtherCAT(handId));
        }
        #endif
    };
}  // namespace communication
}  // namespace linkerhand

namespace Communication {
    using CommFactory = ::linkerhand::communication::CommFactory;
}

#endif  // COMM_FACTORY_H
