#ifndef I_MODBUS_H
#define I_MODBUS_H

#include <cstddef>
#include <cstdint>

namespace linkerhand {
namespace communication {

// Modbus RTU 通信抽象。
// 实例化由 CanBusFactory::createModbus 返回 std::unique_ptr<IModbus>。
// 静态工具方法（detect_serial_ports / auto_detect_serial_port）保留在
// 实现类 Modbus 上，不进抽象接口。
class IModbus {
public:
    virtual ~IModbus() = default;

    virtual bool isOpen() const = 0;
    virtual void close() = 0;

    // 直接发送一帧字节（不做 CRC 处理）
    virtual bool sendRawFrame(const uint8_t* data, size_t length) = 0;

    // 自动按 RTU 协议接收完整一帧；超时 timeout_ms 毫秒；返回字节数，失败 -1
    virtual int receiveCompleteFrame(uint8_t* buffer, size_t max_size, int timeout_ms = 500) = 0;

    // 原子事务：发送 request 并等待 response（线程安全）；返回 response 长度，失败 -1
    virtual int transact(const uint8_t* request, size_t request_len,
                         uint8_t* response, size_t max_response_len,
                         int timeout_ms = 500) = 0;
};

}  // namespace communication
}  // namespace linkerhand

namespace Communication {
    using IModbus = ::linkerhand::communication::IModbus;
}

#endif  // I_MODBUS_H
