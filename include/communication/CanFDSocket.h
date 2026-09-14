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
#include <ifaddrs.h>
#include "communication/ICanFD.h"
#include "core/Common.h"
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
        // 自动检测左右手：遍历已启动的 CAN 接口，逐一发 0xC3(CANFDID/左右手) 读请求，
        // 命中与 hand_type 匹配的接口后打开该接口（参考 CanBus 的 auto_detect_channel）。
        // L30V6 协议：arb 1Mbps / data 5Mbps FD 帧，需内核侧已 fd on。
        explicit CanFDSocket(const HAND_TYPE& hand_type);
        ~CanFDSocket();

        bool init();
        void close();
        bool isOpen() const override;

        void send(const std::vector<uint8_t>& data, uint32_t can_id, bool is_extended = true) override;
        CanFDFrame recv(int timeout_ms = 100) override;

    private:
        // —— 自动检测左右手辅助（仅供 CanFDSocket(HAND_TYPE) 使用）——
        static std::vector<std::string> detect_can_interfaces();
        static bool is_interface_up(const std::string& ifname);
        // 在 ifname 上发 0xC3 读请求，若能读到与 hand_type 匹配的应答则返回 true
        static bool probe_hand(const std::string& ifname, HAND_TYPE hand_type);
        static std::string auto_detect_channel(HAND_TYPE hand_type);

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
