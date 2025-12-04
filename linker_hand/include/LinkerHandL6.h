#ifndef LINKER_HAND_L6_H
#define LINKER_HAND_L6_H

#include <thread>
#include <mutex>
#include <queue>
#include <chrono>
#include <iostream>
#include <sstream>
#include <condition_variable>

#include "IHand.h"
#include "CanBusFactory.h"

namespace LinkerHandL6
{

typedef enum
{									  
    // 指令码	指令功能		        	数据长度	        CAN发送DLC	CAN接收DLC	数据范围
    JOINT_POSITION = 0x01,	            // 关节1-6的关节位置		7	8	8	0-0xFF
    MOTOR_TEMPERATURE = 0x33,	        // 关节1-6的温度信息		7	1	8	0-0xFF
    LINKER_HAND_VERSION = 0x64,	        // 版本号			       8   1   8    0-0xFF

    // 新压感
    TOUCH_SENSOR_TYPE = 0xB0,	// 触觉传感器类型
    THUMB_TOUCH = 0xB1,	// 大拇指触觉传感
    INDEX_TOUCH = 0xB2,	// 食指触觉传感
    MIDDLE_TOUCH = 0xB3, //	中指触觉传感
    RING_TOUCH = 0xB4, // 无名指触觉传感
    LITTLE_TOUCH = 0xB5, //	小拇指触觉传感
    PALM_TOUCH = 0xB6 // 手掌指触觉传感
}FRAME_PROPERTY;

class LinkerHand : public IHand
{
public:
    LinkerHand(uint32_t handId, const std::string &canChannel, int baudrate);
    ~LinkerHand();

	// 设置关节位置
    void setJointPositions(const std::vector<uint8_t> &jointAngles) override;
    void setJointPositionArc(const std::vector<double> &jointAngles) override;
    // 获取当前关节状态
    std::vector<uint8_t> getCurrentStatus() override;
    std::vector<double> getCurrentStatusArc() override;
    // 获取压感数据
    std::vector<std::vector<std::vector<uint8_t>>> getForce() override;
	// 获取电机温度
    std::vector<uint8_t> getTemperature() override;
    // 获取版本信息
    std::string getVersion() override;
	
private:
    uint32_t handId;
    std::unique_ptr<Communication::ICanBus> bus;
    std::thread receiveThread;
    bool running;

    void receiveResponse();

    // 队列和条件变量
	std::vector<uint8_t> joint_position;
	std::vector<uint8_t> joint_position2;

    std::vector<std::vector<std::vector<uint8_t>>> touch_mats;

	// 电机温度
    std::vector<uint8_t> motorTemperature_1;
	std::vector<uint8_t> motorTemperature_2;
	
	// 版本信息
	std::vector<uint8_t> version;

    uint8_t sensor_type = 0;
};
} // namespace LinkerHandL6
#endif // LINKER_HAND_L6_H