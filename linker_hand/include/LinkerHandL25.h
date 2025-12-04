#ifndef LINKER_HAND_L25_H
#define LINKER_HAND_L25_H

#include <thread>
#include <mutex>
#include <queue>
#include <iostream>
#include <sstream>
#include <condition_variable>
#include <map>
#include <numeric>
#include "IHand.h"
#include "CanBusFactory.h"

namespace LinkerHandL25
{

typedef enum
{									  
    INVALID_FRAME_PROPERTY = 0x00, // 无效的can帧属性 | 无返回
    // 并行指令区域
    ROLL_POS = 0x01, // 横滚关节位置 | 坐标系建在每个手指的指根部位，按手指伸直的状态去定义旋转角度 [10,11,12,13,14]
    YAW_POS = 0x02, // 航向关节位置 | 坐标系建在每个手指的指根部位，按手指伸直的状态去定义旋转角度 [5,6,7,8,9]
    ROOT1_POS = 0x03, // 指根1关节位置 | 最接近手掌的指根关节 [0,1,2,3,4]
    ROOT2_POS = 0x04, // 指根2关节位置 | 最接近手掌的指根关节  [15, 16,17,18,19]
    ROOT3_POS = 0x05, // 指根3关节位置 | 最接近手掌的指根关节 暂无
    TIP_POS = 0x06, // 指尖关节位置 | 最接近手掌的指根关节 [20,21,22,23,24]
    
    ROLL_SPEED = 0x09, // 横滚关节速度 | 坐标系建在每个手指的指根部位，按手指伸直的状态去定义旋转角度
    YAW_SPEED = 0x0A, // 航向关节速度 | 坐标系建在每个手指的指根部位，按手指伸直的状态去定义旋转角度
    ROOT1_SPEED = 0x0B, // 指根1关节速度 | 最接近手掌的指根关节
    ROOT2_SPEED = 0x0C, // 指根2关节速度 | 最接近手掌的指根关节
    ROOT3_SPEED = 0x0D, // 指根3关节速度 | 最接近手掌的指根关节
    TIP_SPEED = 0x0E, // 指尖关节速度 | 最接近手掌的指根关节

    ROLL_TORQUE = 0x11, // 横滚关节扭矩 | 坐标系建在每个手指的指根部位，按手指伸直的状态去定义旋转角度
    YAW_TORQUE = 0x12, // 航向关节扭矩 | 坐标系建在每个手指的指根部位，按手指伸直的状态去定义旋转角度
    ROOT1_TORQUE = 0x13, // 指根1关节扭矩 | 最接近手掌的指根关节
    ROOT2_TORQUE = 0x14, // 指根2关节扭矩 | 最接近手掌的指根关节
    ROOT3_TORQUE = 0x15, // 指根3关节扭矩 | 最接近手掌的指根关节
    TIP_TORQUE = 0x16, // 指尖关节扭矩 | 最接近手掌的指根关节

    ROLL_FAULT = 0x19, // 横滚关节故障码 | 坐标系建在每个手指的指根部位，按手指伸直的状态去定义旋转角度
    YAW_FAULT = 0x1A, // 航向关节故障码 | 坐标系建在每个手指的指根部位，按手指伸直的状态去定义旋转角度
    ROOT1_FAULT = 0x1B, // 指根1关节故障码 | 最接近手掌的指根关节
    ROOT2_FAULT = 0x1C, // 指根2关节故障码 | 最接近手掌的指根关节
    ROOT3_FAULT = 0x1D, // 指根3关节故障码 | 最接近手掌的指根关节
    TIP_FAULT = 0x1E, // 指尖关节故障码 | 最接近手掌的指根关节

    ROLL_TEMPERATURE = 0x21, // 横滚关节温度 | 坐标系建在每个手指的指根部位，按手指伸直的状态去定义旋转角度
    YAW_TEMPERATURE = 0x22, // 航向关节温度 | 坐标系建在每个手指的指根部位，按手指伸直的状态去定义旋转角度
    ROOT1_TEMPERATURE = 0x23, // 指根1关节温度 | 最接近手掌的指根关节
    ROOT2_TEMPERATURE = 0x24, // 指根2关节温度 | 最接近手掌的指根关节
    ROOT3_TEMPERATURE = 0x25, // 指根3关节温度 | 最接近手掌的指根关节
    TIP_TEMPERATURE = 0x26, // 指尖关节温度 | 最接近手掌的指根关节
    // 并行指令区域

    // 串行指令区域
    THUMB_POS = 0x41, // 大拇指指关节位置 | 返回本类型数据
    INDEX_POS = 0x42, // 食指关节位置 | 返回本类型数据
    MIDDLE_POS = 0x43, // 中指关节位置 | 返回本类型数据
    RING_POS = 0x44, // 无名指关节位置 | 返回本类型数据
    LITTLE_POS = 0x45, // 小拇指关节位置 | 返回本类型数据

    THUMB_SPEED = 0x49, // 大拇指速度 | 返回本类型数据
    INDEX_SPEED = 0x4A, // 食指速度 | 返回本类型数据
    MIDDLE_SPEED = 0x4B, // 中指速度 | 返回本类型数据
    RING_SPEED = 0x4C, // 无名指速度 | 返回本类型数据
    LITTLE_SPEED = 0x4D, // 小拇指速度 | 返回本类型数据

    THUMB_TORQUE = 0x51, // 大拇指扭矩 | 返回本类型数据
    INDEX_TORQUE = 0x52, // 食指扭矩 | 返回本类型数据
    MIDDLE_TORQUE = 0x53, // 中指扭矩 | 返回本类型数据
    RING_TORQUE = 0x54, // 无名指扭矩 | 返回本类型数据
    LITTLE_TORQUE = 0x55, // 小拇指扭矩 | 返回本类型数据

    THUMB_FAULT = 0x59, // 大拇指故障码 | 返回本类型数据
    INDEX_FAULT = 0x5A, // 食指故障码 | 返回本类型数据
    MIDDLE_FAULT = 0x5B, // 中指故障码 | 返回本类型数据
    RING_FAULT = 0x5C, // 无名指故障码 | 返回本类型数据
    LITTLE_FAULT = 0x5D, // 小拇指故障码 | 返回本类型数据

    THUMB_TEMPERATURE = 0x61, // 大拇指温度 | 返回本类型数据
    INDEX_TEMPERATURE = 0x62, // 食指温度 | 返回本类型数据
    MIDDLE_TEMPERATURE = 0x63, // 中指温度 | 返回本类型数据
    RING_TEMPERATURE = 0x64, // 无名指温度 | 返回本类型数据
    LITTLE_TEMPERATURE = 0x65, // 小拇指温度 | 返回本类型数据
    // 串行指令区域

    // 合并指令区域，同一手指非必要单控数据合并
    FINGER_SPEED = 0x81, // 手指速度 | 返回本类型数据
    FINGER_TORQUE = 0x82, // 转矩 | 返回本类型数据
    FINGER_FAULT = 0x83, // 手指故障码 | 返回本类型数据
    MOTOR_ENABLE = 0x85, // 使能 | 返回本类型数据
    // 指尖传感器数据组
    HAND_NORMAL_FORCE = 0x90, // 五指法向压力
    HAND_TANGENTIAL_FORCE = 0x91, // 五指切向压力
    HAND_TANGENTIAL_FORCE_DIR = 0x92, // 五指切向方向
    HAND_APPROACH_INC = 0x93, // 五指接近感应

    THUMB_ALL_DATA = 0x98, // 大拇指所有数据
    INDEX_ALL_DATA = 0x99, // 食指所有数据
    MIDDLE_ALL_DATA = 0x9A, // 中指所有数据
    RING_ALL_DATA = 0x9B, // 无名指所有数据
    LITTLE_ALL_DATA = 0x9C, // 小拇指所有数据
    // 动作指令 ·ACTION
    ACTION_PLAY = 0xA0, // 动作，预设动作指令

    // L21新增
    TOUCH_SENSOR_TYPE = 0xB0,	// 触觉传感器类型
    THUMB_TOUCH = 0xB1,	// 大拇指触觉传感
    INDEX_TOUCH = 0xB2,	// 食指触觉传感
    MIDDLE_TOUCH = 0xB3, //	中指触觉传感
    RING_TOUCH = 0xB4, // 无名指触觉传感
    LITTLE_TOUCH = 0xB5, //	小拇指触觉传感
    PALM_TOUCH = 0xB6, // 手掌指触觉传感


    // 配置命令·CONFIG
    HAND_UID = 0xC0, // 设备唯一标识码
    HAND_HARDWARE_VERSION = 0xC1, // 硬件版本
    HAND_SOFTWARE_VERSION = 0xC2, // 软件版本
    HAND_COMM_ID = 0xC3, // 设备id
    HAND_FACTORY_RESET = 0xCE, // 恢复出厂设置
    HAND_SAVE_PARAMETER = 0xCF, // 保存参数

    WHOLE_FRAME = 0xF0, // 整帧传输 | 返回一字节帧属性+整个结构体485及网络传输专属
}FRAME_PROPERTY;


class LinkerHand : public IHand {
public:
    LinkerHand(uint32_t handId, const std::string& canChannel, int baudrate, const int currentHandType = 0);
    ~LinkerHand();

    // 设置关节位置
    void setJointPositions(const std::vector<uint8_t> &jointAngles) override;
    void setJointPositionArc(const std::vector<double> &jointAngles) override;
    #if 0
    // 横滚关节位置
    void setRoll(const std::vector<uint8_t> &roll) override;
	// 航向关节位置
    void setYaw(const std::vector<uint8_t> &yaw) override;
    // 指根1关节位置
    void setRoot1(const std::vector<uint8_t> &root1) override;
    // 指根2关节位置
    void setRoot2(const std::vector<uint8_t> &root2) override;
    // 指根3关节位置
    void setRoot3(const std::vector<uint8_t> &root3) override;
    // 指尖关节位置
    void setTip(const std::vector<uint8_t> &tip) override;
    #endif

    std::vector<uint8_t> state_to_cmd(const std::vector<uint8_t>& l25_state);
    // 获取当前关节状态
    std::vector<uint8_t> getCurrentStatus() override;
    std::vector<double> getCurrentStatusArc() override;
    #if 0
    // 大拇指指关节位置
    std::vector<uint8_t> getThumb() override;
    // 食指关节位置
    std::vector<uint8_t> getIndex() override;
    // 中指关节位置
    std::vector<uint8_t> getMiddle() override;
    // 无名指关节位置
    std::vector<uint8_t> getRing() override;
    // 小拇指关节位置
    std::vector<uint8_t> getLittle() override;
    #endif
    // 设置关节速度-有合并指令待确认
    void setSpeed(const std::vector<uint8_t> &speed) override;
    #if 0
    // 横滚关节速度
    void setRollSpeed(const std::vector<uint8_t> &rollSpeed) override;
    // 航向关节速度
    void setYawSpeed(const std::vector<uint8_t> &yawSpeed) override;
    // 指根1关节速度
    void setRoot1Speed(const std::vector<uint8_t> &root1Speed) override;
    // 指根2关节速度
    void setRoot2Speed(const std::vector<uint8_t> &root2Speed) override;
    // 指根3关节速度
    void setRoot3Speed(const std::vector<uint8_t> &root3Speed) override;
    // 指尖关节速度
    void setTipSpeed(const std::vector<uint8_t> &tipSpeed) override;
    #endif
    // 获取当前速度-有合并指令待确认
    std::vector<uint8_t> getSpeed() override;
    #if 0
    // 大拇指速度
    std::vector<uint8_t> getThumbSpeed() override;
    // 食指速度
    std::vector<uint8_t> getIndexSpeed() override;
    // 中指速度
    std::vector<uint8_t> getMiddleSpeed() override;
    // 无名指速度
    std::vector<uint8_t> getRingSpeed() override;
    // 小拇指速度
    std::vector<uint8_t> getLittleSpeed() override;
    #endif
    // 设置扭矩-有合并指令待确认
	void setTorque(const std::vector<uint8_t> &torque) override;
    #if 0
    // 横滚关节扭矩
    void setRollTorque(const std::vector<uint8_t> &rollTorque) override;
    // 航向关节扭矩
    void setYawTorque(const std::vector<uint8_t> &yawTorque) override;
    // 指根1关节扭矩
    void setRoot1Torque(const std::vector<uint8_t> &root1Torque) override;
    // 指根2关节扭矩
    void setRoot2Torque(const std::vector<uint8_t> &root2Torque) override;
    // 指根3关节扭矩
    void setRoot3Torque(const std::vector<uint8_t> &root3Torque) override;
    // 指尖关节扭矩
    void setTipTorque(const std::vector<uint8_t> &tipTorque) override;
    #endif
    // 获取当前扭矩-有合并指令待确认
	std::vector<uint8_t> getTorque() override;
    #if 0
    // 大拇指扭矩
    std::vector<uint8_t> getThumbTorque() override;
    // 食指扭矩
    std::vector<uint8_t> getIndexTorque() override;
    // 中指扭矩
    std::vector<uint8_t> getMiddleTorque() override;
    // 无名指扭矩
    std::vector<uint8_t> getRingTorque() override;
    // 小拇指扭矩
    std::vector<uint8_t> getLittleTorque() override;
    #endif
    // 获取故障码-有合并指令待确认
    std::vector<uint8_t> getFaultCode() override;
    #if 0
    // 清除电机故障码
    void clearFaultCode(const std::vector<uint8_t> &torque = std::vector<uint8_t>(5, 0)) override;
    // 大拇指故障码
    std::vector<uint8_t> getThumbFaultCode() override;
    // 食指故障码
    std::vector<uint8_t> getIndexFaultCode() override;
    // 中指故障码
    std::vector<uint8_t> getMiddleFaultCode() override;
    // 无名指故障码
    std::vector<uint8_t> getRingFaultCode() override;
    // 小拇指故障码
    std::vector<uint8_t> getLittleFaultCode() override;
    // ------------------------------------------------以下函数待确认
    // 横滚关节故障码
    std::vector<uint8_t> getRollFaultCode() override;
    // 航向关节故障码
    std::vector<uint8_t> getYawFaultCode() override;
    // 指根1关节故障码
    std::vector<uint8_t> getRoot1FaultCode() override;
    // 指根2关节故障码
    std::vector<uint8_t> getRoot2FaultCode() override;
    // 指根3关节故障码
    std::vector<uint8_t> getRoot3FaultCode() override;
    // 指尖关节故障码
    std::vector<uint8_t> getTipFaultCode() override;
    #endif
    //--------------------------------------------------------------------
    // 获取温度
    std::vector<uint8_t> getTemperature() override;
    #if 0
    // 大拇指温度
    std::vector<uint8_t> getThumbTemperature() override;
    // 食指温度
    std::vector<uint8_t> getIndexTemperature() override;
    // 中指温度
    std::vector<uint8_t> getMiddleTemperature() override;
    // 无名指温度
    std::vector<uint8_t> getRingTemperature() override;
    // 小拇指温度
    std::vector<uint8_t> getLittleTemperature() override;
    //----------------------------------------------------以下函数待确认
    // 获取横滚关节温度
    std::vector<uint8_t> getRollTemperature() override;
    // 获取航向关节温度
    std::vector<uint8_t> getYawTemperature() override;
    // 获取指根1关节温度
    std::vector<uint8_t> getRoot1Temperature() override;
    // 获取指根2关节温度
    std::vector<uint8_t> getRoot2Temperature() override;
    // 获取指根3关节温度
    std::vector<uint8_t> getRoot3Temperature() override;
    // 获取指尖关节温度
    std::vector<uint8_t> getTipTemperature() override;
    #endif
	//--------------------------------------------------------------------
	// 获取所有压感数据
    std::vector<std::vector<uint8_t>> getForce(const int type = 0) override;
    std::vector<std::vector<std::vector<uint8_t>>> getForce() override;
    #if 1
    // 获取大拇指压感数据
    std::vector<uint8_t> getThumbForce() override;
    // 获取食指压感数据
    std::vector<uint8_t> getIndexForce() override;
    // 获取中指压感数据
    std::vector<uint8_t> getMiddleForce() override;
    // 获取无名指压感数据
    std::vector<uint8_t> getRingForce() override;
    // 获取小拇指压感数据
    std::vector<uint8_t> getLittleForce() override;
    //-----------------------------------------------
	// 获取五个手指的法向压力
    std::vector<uint8_t> getNormalForce() override;
	// 获取五个手指的切向压力
    std::vector<uint8_t> getTangentialForce() override;
	// 获取五个手指的切向方向
    std::vector<uint8_t> getTangentialForceDir() override;
	// 获取五个手指指接近感应
    std::vector<uint8_t> getApproachInc() override;
    #endif
    //--------------------------------------------------------------------
    // 获取版本号
    std::string getVersion() override;
    // 设置电机使能 目前仅支持L25
	void setMotorEnable(const std::vector<uint8_t> &enable = std::vector<uint8_t>(5, 0));
	// 设置电机使能 目前仅支持L25
	void setMotorDisable(const std::vector<uint8_t> &disable = std::vector<uint8_t>(5, 1));
    // 设备唯一标识码
    std::vector<uint8_t> getUID();
    // 设备id
    std::vector<uint8_t> getCommID();
    // 恢复出厂设置
    void factoryReset();
    // 保存参数
    void saveParameter();


    #if 0
    // 动作，预设动作指令
    void playAction(const std::vector<uint8_t> &action) override;
    //--------------------------------------------------------------------
    
    // 整帧传输
    void setWholeFrame(bool wholeFrame) override;
    #endif

private:
    uint32_t handId;
    std::unique_ptr<Communication::ICanBus> bus;
    std::thread receiveThread;
    bool running;
    std::mutex responseMutex;

    void receiveResponse();

    // 故障码
    std::vector<uint8_t> motor_fault_code;
    // 电流
    std::vector<uint8_t> motor_current;


    // 速度
    // std::vector<uint8_t> motor_speed; 
    std::vector<uint8_t> thumb_speed; // 大拇指速度
    std::vector<uint8_t> index_speed; // 食指速度 
    std::vector<uint8_t> middle_speed; // 中指速度
    std::vector<uint8_t> ring_speed; // 无名指速度
    std::vector<uint8_t> little_speed; // 小拇指速度


    // 关节位置
    // std::vector<uint8_t> joint_pos; 
    std::vector<uint8_t> thumb_pos; // 大拇指位置
    std::vector<uint8_t> index_pos; // 食指位置
    std::vector<uint8_t> middle_pos; // 中指位置
    std::vector<uint8_t> ring_pos; // 无名指位置
    std::vector<uint8_t> little_pos; // 小拇指位置

    // 温度
    // std::vector<uint8_t> motor_temperature;
    std::vector<uint8_t> thumb_temperature; // 大拇指温度
    std::vector<uint8_t> index_temperature;  // 食指温度
    std::vector<uint8_t> middle_temperature; // 中指温度
    std::vector<uint8_t> ring_temperature; // 无名指温度
    std::vector<uint8_t> little_temperature; // 小拇指温度


    // 扭矩
    std::vector<uint8_t> thumb_torque; // 大拇指扭矩
    std::vector<uint8_t> index_torque; // 食指扭矩
    std::vector<uint8_t> middle_torque; // 中指扭矩
    std::vector<uint8_t> ring_torque; // 无名指扭矩
    std::vector<uint8_t> little_torque; // 小拇指扭矩

    // 故障
    std::vector<uint8_t> thumb_fault; // 大拇指故障
    std::vector<uint8_t> index_fault; // 食指故障
    std::vector<uint8_t> middle_fault; // 中指故障
    std::vector<uint8_t> ring_fault; // 无名指故障
    std::vector<uint8_t> little_fault; // 小拇指故障

    std::vector<uint8_t> normal_force;
    std::vector<uint8_t> tangential_force;
    std::vector<uint8_t> tangential_force_dir;
    std::vector<uint8_t> approach_inc;

    std::vector<uint8_t> thumb_pressure;
    std::vector<uint8_t> index_finger_pressure;
    std::vector<uint8_t> middle_finger_pressure;
    std::vector<uint8_t> ring_finger_pressure;
    std::vector<uint8_t> little_finger_pressure;
    std::vector<uint8_t> palm_force_data;
    
	// std::vector<uint8_t> joint_position2;
    // std::vector<uint8_t> joint_position3;
    // std::vector<uint8_t> joint_position4;
    // std::vector<uint8_t> joint_position5;

    // 版本号
    std::vector<uint8_t> hand_hardware_version;
    std::vector<uint8_t> hand_software_version;

    // 设备唯一标志
    std::vector<uint8_t> hand_uid;
    std::vector<uint8_t> hand_comm_id;

    // 堵转计数
    std::vector<uint8_t> rotor_lock_count;

    // 手型
    int current_hand_type = 0; // 0:L25 1:L21

    uint8_t sensor_type = 0;

    std::vector<std::vector<std::vector<uint8_t>>> touch_mats;
};
}
#endif // LINKER_HAND_L25_H