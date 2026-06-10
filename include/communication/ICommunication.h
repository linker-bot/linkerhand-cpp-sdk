#ifndef I_COMMUNICATION_H
#define I_COMMUNICATION_H

#include <set>
#include <vector>
#include <string>
#include <iostream>

namespace linkerhand {
namespace communication
{
    // 跨总线（CAN/Modbus/EtherCAT）抽象的占位接口。
    // 当前未被任何实现类继承——保留作为未来跨总线统一抽象的占位，
    // 实际实现请走 ICanBus / ICanFD / IModbus / IEtherCAT 各自的接口。
    class ICommunication
    {
    public:
        virtual ~ICommunication() = default;
        virtual void send(const std::vector<uint8_t>& data, uint32_t &id, const int &start_address = 0, const int &num = 0) = 0;
        virtual std::vector<uint8_t> recv(uint32_t& id, const int &start_address = 0, const int &num = 0) = 0;
    };
}  // namespace communication
}  // namespace linkerhand

// 向后兼容：原本 ICommunication 是全局类
using ICommunication = ::linkerhand::communication::ICommunication;

#endif  // I_COMMUNICATION_H