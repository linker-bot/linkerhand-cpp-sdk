#ifdef __linux__
#ifndef CANFD_SOCKET_H
#define CANFD_SOCKET_H

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include "communication/ICanFD.h"
#include "core/LinkerHandExport.h"

namespace linkerhand {
namespace communication
{
    // 基于 Linux 内核 SocketCAN 的原生 CAN FD 后端。
    // 与厂商驱动版 CanFD（third_party/libcanbus）并列，只用 linux/can.h，无 third_party 依赖。
    // 波特率由用户在内核侧配置（sudo ip link set canX type can bitrate <b> dbitrate <d> fd on），
    // 本类只负责打开套接字、开启 FD 帧模式、收发。
    class LINKERHAND_API CanFDSocket : public ICanFD
    {
    public:
        explicit CanFDSocket(const std::string& interface = "can0");
        ~CanFDSocket();

        bool init();
        void close();
        bool isOpen() const override;

        void send(const std::vector<uint8_t>& data, uint32_t can_id, bool is_extended = true) override;
        CanFDFrame recv(int timeout_ms = 100) override;

    private:
        int socket_fd = -1;
        std::string interface;
        std::atomic<bool> is_open{false};
        std::mutex tx_mutex;
        std::mutex rx_mutex;
    };
}  // namespace communication
}  // namespace linkerhand

namespace Communication {
    using CanFDSocket = ::linkerhand::communication::CanFDSocket;
}

#endif  // CANFD_SOCKET_H
#endif  // __linux__
