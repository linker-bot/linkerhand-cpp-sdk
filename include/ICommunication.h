#ifndef LINKERHAND_ICOMMUNICATION_H
#define LINKERHAND_ICOMMUNICATION_H

#include <vector>
#include <cstdint>

namespace linkerhand {
namespace communication {

/**
 * @brief 通信接口基类
 * 
 * 定义通用的通信接口，支持 ModBus 等协议
 */
class ICommunication {
public:
    virtual ~ICommunication() = default;
    
    /**
     * @brief 发送数据
     * @param data 要发送的数据
     * @param id 设备ID（输入输出参数）
     * @param start_address 起始地址（默认0）
     * @param num 数量（默认0）
     */
    virtual void send(const std::vector<uint8_t>& data, uint32_t &id, 
                     const int &start_address = 0, const int &num = 0) = 0;
    
    /**
     * @brief 接收数据
     * @param id 设备ID（输入输出参数）
     * @param start_address 起始地址（默认0）
     * @param num 数量（默认0）
     * @return 接收到的数据
     */
    virtual std::vector<uint8_t> recv(uint32_t& id, 
                                    const int &start_address = 0, 
                                    const int &num = 0) = 0;
};

} // namespace communication
} // namespace linkerhand

#endif // LINKERHAND_ICOMMUNICATION_H