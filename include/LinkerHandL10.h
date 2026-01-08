#ifndef LINKER_HAND_L10_H
#define LINKER_HAND_L10_H

#include <thread>
#include <mutex>
#include <queue>
#include <iostream>
#include <sstream>
#include <chrono>
#include <condition_variable>

#include "IHand.h"
#include "CanBusFactory.h"

namespace LinkerHandL10
{

typedef enum
{									  
	// INVALID_FRAME_PROPERTY = 0x00,	// 无效的can帧属性
    JOINT_POSITION_RCO = 0x01,			// 关节1-6的关节位置
    TORQUE_LIMIT = 0x02,				// 五根手指的转矩限制
    TORQUE_LIMIT_2 = 0x03,				// 五根手指的转矩限制2
    JOINT_POSITION2_RCO = 0x04,			// 关节7-10的关节位置
    JOINT_SPEED = 0x05,					// 五根手指的速度
    JOINT_SPEED_2 = 0x06,				// 五根手指的速度2
    REQUEST_DATA_RETURN = 0x09,			// 获取所有关节位置和压力
    // JOINT_POSITION_N = 0x11,
    // MAX_PRESS_N = 0x12,
    HAND_NORMAL_FORCE = 0X20,			// 五个手指的法向压力
    HAND_TANGENTIAL_FORCE = 0X21,		// 五个手指的切向压力
    HAND_TANGENTIAL_FORCE_DIR = 0X22,	// 五个手指的切向方向
    HAND_APPROACH_INC = 0X23,			// 五个手指指接近感应
	THUMB_ALL_DATA = 0x28,              // 大拇指所有数据 | 返回 1法向压力 2切向压力 3切向方向 4接近感应
    INDEX_FINGER_ALL_DATA = 0x29,       // 食指所有数据 | 返回 1法向压力 2切向压力 3切向方向 4接近感应
    MIDDLE_FINGER_ALL_DATA = 0x30,      // 中指所有数据 | 返回 1法向压力 2切向压力 3切向方向 4接近感应
    RING_FINGER_ALL_DATA = 0x31,        // 无名指所有数据 | 返回 1法向压力 2切向压力 3切向方向 4接近感应
    LITTLE_FINGER_ALL_DATA = 0x32,      // 小拇指所有数据 | 返回 1法向压力 2切向压力 3切向方向 4接近感应
	MOTOR_TEMPERATURE_1 = 0x33,			// 电机温度1
    MOTOR_TEMPERATURE_2 = 0x34,			// 电机温度2
	MOTOR_FAULT_CODE_1 = 0x35,			// 电机故障码1
	MOTOR_FAULT_CODE_2 = 0x36,			// 电机故障码2
	LINKER_HAND_VERSION = 0X64,			// 版本号

    // 新压感
    TOUCH_SENSOR_TYPE = 0xB0,	// 触觉传感器类型
    THUMB_TOUCH = 0xB1,	// 大拇指触觉传感
    INDEX_TOUCH = 0xB2,	// 食指触觉传感
    MIDDLE_TOUCH = 0xB3, //	中指触觉传感
    RING_TOUCH = 0xB4, // 无名指触觉传感
    LITTLE_TOUCH = 0xB5, //	小拇指触觉传感
    PALM_TOUCH = 0xB6, // 手掌指触觉传感
}FRAME_PROPERTY;

class LinkerHand : public IHand
{
public:
    LinkerHand(uint32_t handId, const std::string &canChannel, int baudrate);
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
	// 获取当前关节状态
    std::vector<uint8_t> getCurrentStatus() override;
    std::vector<double> getCurrentStatusArc() override;
	//--------------------------------------------------------------------
	// 获取所有压感数据
    std::vector<std::vector<std::vector<uint8_t>>> getForce() override;
	// 获取五个手指的法向压力
    std::vector<uint8_t> getNormalForce() override;
	// 获取五个手指的切向压力
    std::vector<uint8_t> getTangentialForce() override;
	// 获取五个手指的切向方向
    std::vector<uint8_t> getTangentialForceDir() override;
	// 获取五个手指指接近感应
    std::vector<uint8_t> getApproachInc() override;
	//--------------------------------------------------------------------
	// 获取版本信息
    std::string getVersion() override;
	// 获取电机温度
    std::vector<uint8_t> getTemperature() override;
	// 获取电机故障码
    std::vector<uint8_t> getFaultCode() override;
	// 获取电机电流
    std::vector<uint8_t> getCurrent() override;	// 暂时无用
	// 获取所有关节位置和压力
    std::vector<uint8_t> requestAllStatus(); // 暂时无用
	// 获取当前扭矩
	std::vector<uint8_t> getTorque() override;

private:
    uint32_t handId;
    std::unique_ptr<Communication::ICanBus> bus;
    std::thread receiveThread;
    bool running;
    std::mutex responseMutex;

    void receiveResponse();

    std::queue<std::vector<uint8_t>> responseQueue; // 通用响应队列
    std::condition_variable queueCond;              // 通用队列条件变量
    std::mutex queueMutex;                          // 通用队列互斥锁

    // 队列和条件变量
	std::vector<uint8_t> joint_position;
	std::vector<uint8_t> joint_position2;
	std::vector<uint8_t> joint_speed;
    std::vector<uint8_t> joint_speed_2;

	std::vector<uint8_t> normal_force;
    std::vector<uint8_t> tangential_force;
    std::vector<uint8_t> tangential_force_dir;
    std::vector<uint8_t> approach_inc;

	std::vector<uint8_t> thumb_pressure;
    std::vector<uint8_t> index_finger_pressure;
    std::vector<uint8_t> middle_finger_pressure;
    std::vector<uint8_t> ring_finger_pressure;
    std::vector<uint8_t> little_finger_pressure;

    std::vector<std::vector<std::vector<uint8_t>>> touch_mats;

	// 最大扭矩
    std::vector<uint8_t> max_torque;
    std::vector<uint8_t> max_torque_2;

	// 电机温度
    std::vector<uint8_t> motorTemperature_1;
	std::vector<uint8_t> motorTemperature_2;
	
	// 电机故障码
    std::vector<uint8_t> motorFaultCode_1;
	std::vector<uint8_t> motorFaultCode_2;

	// 版本信息
	std::vector<uint8_t> version;

    uint8_t sensor_type = 0;
};
} // namespace LinkerHandL10
#endif // LINKER_HAND_L10_H