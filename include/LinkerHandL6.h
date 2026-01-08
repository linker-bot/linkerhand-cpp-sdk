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
    // 指令码	指令功能		        	数据长度	CAN发送DLC	CAN接收DLC	数据范围
    JOINT_POSITION = 0x01,	            // 关节1-6的关节位置		6	7	7	0-0xFF
    TORQUE_LIMIT = 0x02,	            // 关节1-6的转矩限制		6	7	7	0-0xFF
    JOINT_SPEED = 0x05,	                // 关节1-6的速度			6	7	7	0-0xFF
    JOINT_ACCELERATION = 0x07,          // 关节1-6的加速度		6	7	7	0-0xFE
    MOTOR_TEMPERATURE = 0x33,	        // 关节1-6的温度信息		7	1	8	0-0xFF
    MOTOR_ERROR_CODE = 0x35,	        // 关节1-6的错误码		7	1	8	0-0xFF
    CLEAR_FAULT_CODE = 0x83,	        // 清除关节1-6的错误码		6	7	7	0-0xFF
    RE_HOME_COMMAND = 0x38,	            // 重新调零点命令		8	8	2	
    
    // 设备信息指令
    DEVICE_SERIAL_NUMBER = 0xC0,        // 设备出厂编码			24	1	3帧8字节	
    DEVICE_HARDWARE_VERSION = 0xC1,     // 设备硬件PCB版本号		3	1	4	
    DEVICE_SOFTWARE_VERSION = 0xC2,     // 设备软件版本号		3	1	4	
    DEVICE_MECHANICAL_VERSION = 0xC4,   // 设备机械结构版本号	3	1	4	
    MODIFY_CAN_ID = 0xC3,               // 修改CANID指令		3			
    STALL_THRESHOLD = 0xC5,             // 堵转阈值				
    STALL_TIME = 0xC6,                  // 堵转时间				
    STALL_TORQUE = 0xC7,                // 堵转扭矩				
    FACTORY_RESET = 0xCE,               // 恢复出厂设置			
    SAVE_PARAMETERS = 0xCF,             // 保存参数				

    // 触觉传感器指令
    TOUCH_SENSOR_TYPE = 0xB0,	        // 触觉传感器类型
    THUMB_TOUCH = 0xB1,	                // 大拇指触觉传感
    INDEX_TOUCH = 0xB2,	                // 食指触觉传感
    MIDDLE_TOUCH = 0xB3,                // 中指触觉传感
    RING_TOUCH = 0xB4,                  // 无名指触觉传感
    LITTLE_TOUCH = 0xB5,                // 小拇指触觉传感
    PALM_TOUCH = 0xB6                   // 手掌触觉传感
} FRAME_PROPERTY;

// 协议辅助常量
static constexpr uint8_t TOUCH_TYPE_MATRIX = 0x02; // 矩阵型触觉类型值
static constexpr uint8_t TOUCH_PAGE_REQ    = 0xC6; // 触觉分页读取的请求子命令

class LinkerHand : public IHand
{
public:
    LinkerHand(uint32_t handId, const std::string &canChannel, int baudrate);
    ~LinkerHand();

	// 设置关节位置
    void setJointPositions(const std::vector<uint8_t> &jointAngles) override;
    void setJointPositionArc(const std::vector<double> &jointAngles) override;
    
    void setSpeed(const std::vector<uint8_t> &speed) override;
    
    void setTorque(const std::vector<uint8_t> &torque);

    void setAcceleration(const std::vector<uint8_t> &acceleration);

    // 重新调零
    void reHome();
    
    // 获取当前关节状态
    std::vector<uint8_t> getCurrentStatus() override;
    std::vector<double> getCurrentStatusArc() override;
    
    // 获取当前速度
    std::vector<uint8_t> getSpeed() override;
    
    // 获取当前扭矩
    std::vector<uint8_t> getTorque() override;
    
    // 获取当前加速度
    std::vector<uint8_t> getAcceleration();
    
    // 获取电机错误码
    std::vector<uint8_t> getFaultCode() override;

    void clearFaultCode(const std::vector<uint8_t> &code);
    
    // 获取压感数据
    std::vector<std::vector<std::vector<uint8_t>>> getForce() override;
    
    // 获取电机温度
    std::vector<uint8_t> getTemperature() override;
    
    // 获取版本信息
    std::string getVersion() override;
    
    // 获取设备序列号
    std::string getSerialNumber();
    
    // 获取硬件版本
    std::string getHardwareVersion();
    
    // 获取机械版本
    std::string getMechanicalVersion();
    
    // 系统功能
    void factoryReset();
    void saveParameters();
	
private:
    uint32_t handId;
    std::unique_ptr<Communication::ICanBus> bus;
    std::thread receiveThread;
    bool running;
    std::mutex data_mutex_;

    void receiveResponse();

    // 数据存储
    std::vector<uint8_t> joint_position;
    std::vector<uint8_t> joint_speeds;
    std::vector<uint8_t> joint_torques;
    std::vector<uint8_t> joint_accelerations;
    std::vector<uint8_t> motor_temperature;
    std::vector<uint8_t> error_codes;
    std::vector<uint8_t> device_serial;
    std::vector<uint8_t> hardware_version;
    std::vector<uint8_t> software_version;
    std::vector<uint8_t> mechanical_version;

    std::vector<std::vector<std::vector<uint8_t>>> touch_mats;

    uint8_t sensor_type = 0;
    
    // 辅助方法
    void requestDeviceInfo();
    void parseSerialNumber(const std::vector<uint8_t>& data);
    std::string getErrorDescription(uint8_t error_code);
};
} // namespace LinkerHandL6
#endif // LINKER_HAND_L6_H
