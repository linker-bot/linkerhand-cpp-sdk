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
        // CAN-FD 上运行的手型协议，决定自动检测左右手时用哪种探测帧。
        //   L30_C3  : L30V6 私有 0xC3 扩展帧（CANFDID/左右手）
        //   O30_HOP : O30 HOP 协议标准帧（读 MI 0x41 SI 0xD9 左右手，特殊 ID 0x7FF 广播）
        enum class FDProtocol { L30_C3, O30_HOP };

        explicit CanFDSocket(const std::string& interface = "can0");
        // 自动检测左右手：遍历已启动的 CAN 接口，逐一发探测读请求，
        // 命中与 hand_type 匹配的接口后打开该接口（参考 CanBus 的 auto_detect_channel）。
        // 默认按 L30V6 协议（0xC3 扩展帧，arb 1Mbps / data 5Mbps FD 帧，需内核侧已 fd on）。
        explicit CanFDSocket(const HAND_TYPE& hand_type);
        // 指定协议的自动检测重载（O30 用 FDProtocol::O30_HOP）。
        CanFDSocket(const HAND_TYPE& hand_type, FDProtocol proto);
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
        // 在 ifname 上发 0xC3(L30) 读请求，若能读到与 hand_type 匹配的应答则返回 true
        static bool probe_hand(const std::string& ifname, HAND_TYPE hand_type);
        // 在 ifname 上按 HOP 协议(O30) 用特殊 ID 0x7FF 读左右手。
        //   match_side=true : 仅当设备左右手与 hand_type 一致时返回 true（严格匹配）
        //   match_side=false: 只要收到任一 HOP 左右手应答即返回 true（宽松，不校验左右手）
        //   found_left(可选输出): 命中时回传设备实际左右手（true=左, false=右）
        static bool probe_hand_hop(const std::string& ifname, HAND_TYPE hand_type,
                                   bool match_side = true, bool* found_left = nullptr);
        static std::string auto_detect_channel(HAND_TYPE hand_type, FDProtocol proto);

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
