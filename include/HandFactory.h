#ifndef LINKERHAND_HAND_FACTORY_H
#define LINKERHAND_HAND_FACTORY_H

#include "IHand.h"
#include "LinkerHandL6.h"
#include "LinkerHandL7.h"
#include "LinkerHandL10.h"
#include "LinkerHandL20.h"
#include "LinkerHandL25.h"
#include "ModbusLinkerHandL10.h"
#include "Common.h"

namespace linkerhand {
namespace factory {

/**
 * @brief 手型号工厂类
 * 
 * 根据手型号、手类型和通信类型创建对应的手实例
 */
class HandFactory {
public:
    /**
     * @brief 创建手实例
     * @param type 手型号
     * @param handId 手ID（左手/右手）
     * @param commType 通信类型
     * @return 手实例的智能指针
     * @throws std::invalid_argument 如果参数无效
     */
    static std::unique_ptr<hand::IHand> createHand(LINKER_HAND type, uint32_t handId, COMM_TYPE commType) {

        if (handId != static_cast<uint32_t>(LEFT) && 
            handId != static_cast<uint32_t>(RIGHT))
        {
            throw std::invalid_argument("Unsupported hand type");
        }

        std::string canChannel;
        int baudrate = 1000000;

        switch (commType)
        {
        case COMM_CAN_0:
            canChannel = "can0";
            break;
        case COMM_CAN_1:
            canChannel = "can1";
            break;
        case COMM_MODBUS:
            canChannel = "modbus";
            break;
        case COMM_ETHERCAT:
            canChannel = "ethercat";
            break;
        default:
            throw std::invalid_argument("Unknown comm type");
            break;
        }

        if (canChannel == "can0" || canChannel == "can1" || canChannel == "ethercat") {
            switch (type) {
                case O6:
                    return std::unique_ptr<hand::IHand>(std::make_unique<hand::L6Hand>(handId, canChannel, baudrate));
                    break;
                case L6:
                    return std::unique_ptr<hand::IHand>(std::make_unique<hand::L6Hand>(handId, canChannel, baudrate));
                    break;
                case L7:
                    return std::unique_ptr<hand::IHand>(std::make_unique<hand::L7Hand>(handId, canChannel, baudrate));
                    break;
                case L10:
                    return std::unique_ptr<hand::IHand>(std::make_unique<hand::L10Hand>(handId, canChannel, baudrate));
                    break;
                case L20:
                    return std::unique_ptr<hand::IHand>(std::make_unique<hand::L20Hand>(handId, canChannel, baudrate));
                    break;
                case L21:
                    return std::unique_ptr<hand::IHand>(std::make_unique<hand::L25Hand>(handId, canChannel, baudrate, 1));
                    break;
                case L25:
                    return std::unique_ptr<hand::IHand>(std::make_unique<hand::L25Hand>(handId, canChannel, baudrate, 0));
                    break;
                default:
                    throw std::invalid_argument("Unknown hand type");
            }
        } else if (canChannel == "modbus") {
        	#if USE_RMAN
            switch (type) {
                case L10:
                    return std::unique_ptr<hand::IHand>(std::make_unique<hand::ModbusL10Hand>(handId));
                default:
                    throw std::invalid_argument("Unknown hand type");
                    break;
            }
            #else
            	throw std::runtime_error("ModBus support is disabled (USE_RMAN=0)");
			#endif
        }

        return nullptr;
    }
};

} // namespace factory
} // namespace linkerhand

// 向后兼容：在全局命名空间中提供别名
using HandFactory = linkerhand::factory::HandFactory;

#endif // LINKERHAND_HAND_FACTORY_H
