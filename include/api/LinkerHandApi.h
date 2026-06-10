#ifndef LINKER_HAND_API_H
#define LINKER_HAND_API_H

#include <chrono>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "Common.h"
#include "CommunicationCallbacks.h"
#include "ErrorCode.h"
#include "LinkerHandExport.h"

// 仅前向声明 IHand，避免把 hand/ 子目录的实现细节带进公共头。
// 实例化 std::unique_ptr<IHand> 的析构在 LinkerHandApi.cpp 里完成。
namespace linkerhand { namespace hand { class IHand; } }

namespace linkerhand {
namespace api {

class LINKERHAND_API LinkerHandApi
{
public:
	LinkerHandApi(const LINKER_HAND &handJoint, const HAND_TYPE &handType, const COMM_TYPE commType = COMM_TYPE::CAN);
	// 新风格枚举重载：转发到旧版构造
	LinkerHandApi(::linkerhand::HandModel model,
	              ::linkerhand::HandType side,
	              ::linkerhand::CommType comm = ::linkerhand::CommType::CAN);
	~LinkerHandApi();

	// 设置 CAN TX 回调函数（CAN 发送数据）
    void setCanTxCallback(CanTxCallback can_tx_callback);
    // 设置 CAN RX 回调函数（CAN 接收数据）
    void setCanRxCallback(CanRxCallback can_rx_callback);

	void freeCanCallback();
	void freeModbusCallback();

    // 设置 Modbus TX 回调函数（Modbus 发送）
    void setModbusTxCallback(ModbusTxCallback modbus_tx_callback);
    // 设置 Modbus RX 回调函数（Modbus 接收）
    void setModbusRxCallback(ModbusRxCallback modbus_rx_callback);

	// 设置关节位置
	void fingerMove(const std::vector<uint8_t> &pose);
	void fingerMoveArc(const std::vector<double> &pose);
	// 获取关节位置
	std::vector<uint8_t> getState();
	std::vector<double> getStateArc();

	// 设置速度
	void setSpeed(const std::vector<uint8_t> &speed);
	// 获取速度
	std::vector<uint8_t> getSpeed();

	// 设置扭矩
	void setTorque(const std::vector<uint8_t> &torque);
	// 获取扭矩
	std::vector<uint8_t> getTorque();

	// 获取压感
	std::vector<std::vector<std::vector<uint8_t>>> getForce();

	// 获取版本号
	std::string getVersion();
	// ----------------------------------------------------------
	// 获取电机温度
	std::vector<uint8_t> getTemperature();
	// 获取电机故障码
	std::vector<uint8_t> getFaultCode();

	// ----------------------------------------------------------
	// 清除电机故障码 目前仅支持L20
	void clearFaultCode(const std::vector<uint8_t> &torque = std::vector<uint8_t>(5, 1));
	// 设置电机使能 目前仅支持L25
	void setEnable(const std::vector<uint8_t> &enable = std::vector<uint8_t>(5, 0));
	// 设置电机使能 目前仅支持L25
	void setDisable(const std::vector<uint8_t> &disable = std::vector<uint8_t>(5, 1));

private:
	// 获取法向压力
	void getNormalForce();
	// 获取切向压力
	void getTangentialForce();
	// 获取切向压力方向
	void getTangentialForceDir();
	// 获取接近感应
	void getApproachInc();

private:
	std::unique_ptr<::linkerhand::hand::IHand> hand;
	uint32_t handId;

	CanTxCallback can_tx_callback_;  // TX回调函数
	CanRxCallback can_rx_callback_;  // RX回调函数
    ModbusTxCallback modbus_tx_callback_; // Modbus TX 回调
    ModbusRxCallback modbus_rx_callback_; // Modbus RX 回调

public:
	LINKER_HAND handJoint_;
	HAND_TYPE handType_;
};

}  // namespace api
}  // namespace linkerhand

// 向后兼容
using LinkerHandApi = ::linkerhand::api::LinkerHandApi;

#endif
