#ifndef LINKERHAND_API_H
#define LINKERHAND_API_H

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include "HandFactory.h"
#include "Common.h"
#include "IHand.h"

/**
 * @brief LinkerHand API 主类
 * 
 * 提供统一的接口用于控制各种型号的灵巧手设备
 * 
 * 注意：为了与预编译库兼容，此类保持在全局命名空间
 */
class LinkerHandApi
{
public:
	/**
	 * @brief 构造函数
	 * @param handModel 手型号（LINKER_HAND 枚举）
	 * @param handType 手类型（左手/右手）
	 * @param commType 通信类型（默认 CAN_0）
	 */
	LinkerHandApi(const LINKER_HAND &handModel, const HAND_TYPE &handType,
	              const COMM_TYPE commType = COMM_CAN_0);
	~LinkerHandApi();

	// 设置关节位置
	void fingerMove(const std::vector<uint8_t> &pose);
	void fingerMoveArc(const std::vector<double> &pose);
	// 设置速度
	void setSpeed(const std::vector<uint8_t> &speed);
	// 设置扭矩
	void setTorque(const std::vector<uint8_t> &torque);
	// 获取压感
	std::vector<std::vector<std::vector<uint8_t>>> getForce();
	// 获取速度
	std::vector<uint8_t> getSpeed();
	// 获取当前关节状态
	std::vector<uint8_t> getState();
	std::vector<double> getStateArc();
	// 获取版本号
	std::string getVersion();

	// ----------------------------------------------------------

	// 获取电机温度
	std::vector<uint8_t> getTemperature();
	// 获取电机故障码
	std::vector<uint8_t> getFaultCode();
	// 获取当前电流
	std::vector<uint8_t> getCurrent();
	// 获取当前最大扭矩
	std::vector<uint8_t> getTorque();
	// ------------------------- 待开发 --------------------------
	// 设置电流 目前仅支持L20 
	void setCurrent(const std::vector<uint8_t> &current);
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
	std::unique_ptr<linkerhand::hand::IHand> hand;
	uint32_t handId;
public:
	LINKER_HAND handModel_;  // 手型号（L6, L7, L10, L20, L21, L25 等）
	HAND_TYPE handType_;
};

// 在新命名空间中提供类型别名（用于未来版本）
namespace linkerhand {
namespace api {
    using LinkerHandApi = ::LinkerHandApi;
} // namespace api
} // namespace linkerhand

#endif // LINKERHAND_API_H
