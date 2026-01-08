#ifndef MODBUS_L10_H
#define MODBUS_L10_H
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

namespace ModbusLinkerHandL10
{

class LinkerHand : public IHand
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
}
#endif
#endif // MODBUS_L10_H
