#ifndef RS485_MODBUS_DEVICE_H
#define RS485_MODBUS_DEVICE_H

#include <string>
#include <vector>
#include <mutex>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#endif

#include "core/Common.h"
#include "core/LinkerHandExport.h"
#include "communication/IModbus.h"

namespace linkerhand {
namespace communication {

class LINKERHAND_API Modbus : public IModbus {
public:
    Modbus(const std::string& device = "/dev/ttyUSB0", int baudrate = 115200, char parity = 'N');
    Modbus(int baudrate, char parity);  // 自动检测串口的构造函数
    Modbus(HAND_TYPE hand_type, int baudrate = 115200, char parity = 'N');  // 基于手型的自动检测构造函数
    ~Modbus();

    bool initialize();
    void close() override;

    // 检查串口是否成功打开
#ifdef _WIN32
    bool isOpen() const override { return hSerial_ != INVALID_HANDLE_VALUE; }
#else
    bool isOpen() const override { return fd_ >= 0; }
#endif

    // 通用的发送与接收接口
    bool sendRawFrame(const uint8_t* data, size_t length) override;

    /**
     * @brief 自动处理 Modbus RTU 响应的完整性接收
     * @return 接收到的总字节数，失败返回 -1
     */
    int receiveCompleteFrame(uint8_t* buffer, size_t max_size, int timeout_ms = 500) override;

    /**
     * @brief 原子事务：发送请求并接收响应（线程安全）
     * @param request 发送的请求数据
     * @param request_len 请求数据长度
     * @param response 存储响应的缓冲区
     * @param max_response_len 响应缓冲区最大长度
     * @param timeout_ms 超时时间（毫秒）
     * @return 接收到的响应长度，失败返回 -1
     */
    int transact(const uint8_t* request, size_t request_len, uint8_t* response, size_t max_response_len, int timeout_ms = 500) override;

    void setDebugMode(bool enable) { debug_mode_ = enable; }

public:
    // 静态方法：检测可用串口列表
    static std::vector<std::string> detect_serial_ports();
    // 静态方法：自动检测可用串口（返回第一个可用的串口）
    static std::string auto_detect_serial_port(int baudrate);
    // 静态方法：基于手型自动检测串口
    static std::string auto_detect_serial_port(HAND_TYPE hand_type, int baudrate);

private:
    std::string device_;
    int baudrate_;
    char parity_;
#ifdef _WIN32
    HANDLE hSerial_;
#else
    int fd_;
#endif
    std::mutex mutex_;
    bool debug_mode_;

    bool configureSerialPort();
    int readWithTimeout(uint8_t* buf, size_t len, int timeout_ms);

    // 内部无锁版本的方法，供 transact 使用
    bool sendRawFrameInternal(const uint8_t* data, size_t length);
    int receiveCompleteFrameInternal(uint8_t* buffer, size_t max_size, int timeout_ms);

    // 手型检测相关方法
    static bool ping_interface(const std::string& port, int baudrate, const struct DetectionJob& job);
};

}  // namespace communication
}  // namespace linkerhand

// 向后兼容：原本 Modbus 是全局类
using Modbus = ::linkerhand::communication::Modbus;

#endif