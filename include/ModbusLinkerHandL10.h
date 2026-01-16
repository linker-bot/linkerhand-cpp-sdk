#ifndef LINKERHAND_MODBUS_L10_H
#define LINKERHAND_MODBUS_L10_H
#if USE_RMAN
#include <thread>
#include <mutex>
#include <queue>
#include <iostream>
#include <sstream>
#include <vector>
#include <condition_variable>

#include "ModBus.h"
#include "IHand.h"

namespace linkerhand {
namespace hand {

/**
 * @brief Modbus L10 型号灵巧手实现类
 *
 * 提供通过 Modbus 协议通信的 L10 型号功能实现
 */
class ModbusL10Hand : public IHand
{
public:
    LinkerHand(uint32_t handId);
    ~LinkerHand();

	// 设置关节位置
    void setJointPositions(const std::vector<uint8_t> &jointAngles) override;
    void setJointPositionArc(const std::vector<double> &jointAngles) override;
	// 设置最大扭矩
	void setTorque(const std::vector<uint8_t> &torque) override;
	// 设置关节速度
    void setSpeed(const std::vector<uint8_t> &speed) override;
	// 获取当前速度
    std::vector<uint8_t> getSpeed() override;
    // 获取当前扭矩
	std::vector<uint8_t> getTorque() override;
	// 获取当前关节状态
    std::vector<uint8_t> getCurrentStatus() override;
    std::vector<double> getCurrentStatusArc() override;
	

private:
    uint32_t handId;
    std::thread receiveThread;

    Communication::ModBus bus;

    bool running;
    void receiveResponse();

	std::vector<uint8_t> joints;
	std::vector<uint8_t> speed;
    std::vector<uint8_t> torque;

};

} // namespace hand
} // namespace linkerhand

// 向后兼容：在旧命名空间中提供别名
namespace ModbusLinkerHandL10 {
    using LinkerHand = linkerhand::hand::ModbusL10Hand;
} // namespace ModbusLinkerHandL10

#endif
#endif // LINKERHAND_MODBUS_L10_H
