#ifndef LINKERHAND_IHAND_H
#define LINKERHAND_IHAND_H

#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <stdexcept>

#include "RangeToArc.h"
#include "Common.h"
#include "ErrorCode.h"

namespace linkerhand {
namespace hand {

/**
 * @brief 灵巧手接口基类
 * 
 * 定义所有手型号必须实现的基础接口
 * 注意：不支持的方法会抛出异常而不是返回空值
 */
class IHand
{
public:
    virtual ~IHand() = default;
    // 设置关节位置
    virtual void setJointPositions(const std::vector<uint8_t> &jointAngles) = 0;
    virtual void setJointPositionArc(const std::vector<double> &jointAngles)
    {
        (void) jointAngles;
        throw UnsupportedFeatureException("setJointPositionArc");
    }
    // 获取速度数据
    virtual std::vector<uint8_t> getSpeed()
    {
        throw UnsupportedFeatureException("getSpeed");
    }
    // 设置关节速度
    virtual void setSpeed(const std::vector<uint8_t> &speed)
    {
        (void) speed;
        throw UnsupportedFeatureException("setSpeed");
    }
    // 获取当前关节位置
    virtual std::vector<uint8_t> getCurrentStatus()
    {
        throw UnsupportedFeatureException("getCurrentStatus");
    }

    virtual std::vector<double> getCurrentStatusArc()
    {
        throw UnsupportedFeatureException("getCurrentStatusArc");
    }
    // 获取电机故障码
    virtual std::vector<uint8_t> getFaultCode()
    {
        throw UnsupportedFeatureException("getFaultCode");
    }
    // 获取电机电流
    virtual std::vector<uint8_t> getCurrent()
    {
        throw UnsupportedFeatureException("getCurrent");
    }
    // ------------------------------------------------------
    // 获取压感数据
    virtual std::vector<std::vector<uint8_t>> getForce(const int type)
    {
        (void)type;
        throw UnsupportedFeatureException("getForce");
    }
    virtual std::vector<std::vector<std::vector<uint8_t>>> getForce() 
    {
        throw UnsupportedFeatureException("getForce");
    }
    // 获取大拇指压感数据
    virtual std::vector<uint8_t> getThumbForce()
    {
        throw UnsupportedFeatureException("getThumbForce");
    }
    // 获取食指压感数据
    virtual std::vector<uint8_t> getIndexForce()
    {
        throw UnsupportedFeatureException("getIndexForce");
    }
    // 获取中指压感数据
    virtual std::vector<uint8_t> getMiddleForce()
    {
        throw UnsupportedFeatureException("getMiddleForce");
    }
    // 获取无名指压感数据
    virtual std::vector<uint8_t> getRingForce()
    {
        throw UnsupportedFeatureException("getRingForce");
    }
    // 获取小拇指压感数据
    virtual std::vector<uint8_t> getLittleForce()
    {
        throw UnsupportedFeatureException("getLittleForce");
    }
    // ------------------------------------------------------
    // 获取五指法向力
    virtual std::vector<uint8_t> getNormalForce()
    {
        throw UnsupportedFeatureException("getNormalForce");
    }
    // 获取五指切向力
    virtual std::vector<uint8_t> getTangentialForce()
    {
        throw UnsupportedFeatureException("getTangentialForce");
    }
    // 获取五指法向力方向
    virtual std::vector<uint8_t> getTangentialForceDir()
    {
        throw UnsupportedFeatureException("getTangentialForceDir");
    }
    // 获取五指接近感应
    virtual std::vector<uint8_t> getApproachInc()
    {
        throw UnsupportedFeatureException("getApproachInc");
    }
    // ------------------------------------------------------
    // 设置扭矩 L20暂不支持
    virtual void setTorque(const std::vector<uint8_t> &torque)
    {
        (void)torque;
        throw UnsupportedFeatureException("setTorque");
    }
    // 获取电机扭矩 L20暂不支持
    virtual std::vector<uint8_t> getTorque()
    {
        throw UnsupportedFeatureException("getTorque");
    }
    // 获取电机温度 L20暂不支持
    virtual std::vector<uint8_t> getTemperature()
    {
        throw UnsupportedFeatureException("getTemperature");
    }
    // 获取版本号   目前仅支持L10
    virtual std::string getVersion()
    {
        throw UnsupportedFeatureException("getVersion");
    }
    // 获取设备ID L20协议
    virtual std::vector<uint8_t> getUID()
    {
        throw UnsupportedFeatureException("getUID");
    }
    // 手指堵转或过流判断计数阀值 L20协议
    virtual std::vector<uint8_t> getRotorLockCount()
    {
        throw UnsupportedFeatureException("getRotorLockCount");
    }
    // 清除电机故障码 目前仅支持L20
	virtual void clearFaultCode(const std::vector<uint8_t> &code)
    {
        (void)code;
        throw UnsupportedFeatureException("clearFaultCode");
    }
    // 设置电流 目前仅支持L20 
	virtual void setCurrent(const std::vector<uint8_t> &current)
    {
        (void)current;
        throw UnsupportedFeatureException("setCurrent");
    }
	// 设置电机使能 目前仅支持L25
	virtual void setMotorEnable(const std::vector<uint8_t> &enable)
    {
        (void)enable;
        throw UnsupportedFeatureException("setMotorEnable");
    }
	// 设置电机使能 目前仅支持L25
	virtual void setMotorDisable(const std::vector<uint8_t> &disable)
    {
        (void)disable;
        throw UnsupportedFeatureException("setMotorDisable");
    }

    // ---------------------------------------------------------------------

    virtual std::vector<uint8_t> getSubVector(const std::vector<uint8_t> &vec)
    {
        std::vector<uint8_t> result;
        if (vec.size() > 1) result.insert(result.end(), vec.begin() + 1, vec.end());
        return result;
    }

    virtual std::vector<uint8_t> getSubVector(const std::vector<uint8_t>& vec1, const std::vector<uint8_t>& vec2)
    {
        std::vector<uint8_t> result;
        if (vec1.size() > 1) result.insert(result.end(), vec1.begin() + 1, vec1.end());
        if (vec2.size() > 1) result.insert(result.end(), vec2.begin() + 1, vec2.end());
        return result;
    }

    virtual std::string getCurrentTime()
    {
        // 获取当前时间点
        auto now = std::chrono::high_resolution_clock::now();
    
        // 使用std::put_time格式化时间点为字符串
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    
        // 获取微秒部分并添加到字符串中
        auto duration = now.time_since_epoch();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration % std::chrono::seconds(1)).count();
        // 只保留毫秒部分
        int milliseconds = microseconds / 1000;
        ss << "." << std::setfill('0') << std::setw(3) << milliseconds;
        // 微妙
        // ss << "." << std::setfill('0') << std::setw(6) << microseconds;

        // 打印格式化的时间字符串
        // std::cout << "send time: " << ss.str() << std::endl;

        return ss.str();
    }

};

} // namespace hand
} // namespace linkerhand

#endif // LINKERHAND_IHAND_H

