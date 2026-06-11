#ifdef __linux__
#ifndef CAN_BUS_H
#define CAN_BUS_H

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include "communication/ICanBus.h"
#include "core/Common.h"

namespace linkerhand {
namespace communication {
    class CanBus : public ICanBus {
    public:
        CanBus(const HAND_TYPE& hand_type);
        CanBus(const std::string& interface, int bitrate = 1000000);

        virtual ~CanBus();

        // 核心接口
        void send(const std::vector<uint8_t>& data, uint32_t can_id, bool wait = false) override;
        CANFrame recv() override;
        void shutdown();

        // 工具方法
        void setReceiveTimeout(int seconds, int microseconds);
        static void globalShutdown();

    private:
        struct DetectionJob {
            const char* name;
            uint32_t id;
            bool is_ext;
            uint8_t data0;
            uint8_t dlc;
            uint8_t data[8];
        };

        bool ping_interface(const std::string& ifname, const DetectionJob& job);
        bool is_interface_up(const std::string& ifname);
        std::vector<std::string> detect_can_interfaces();
        std::string auto_detect_channel(HAND_TYPE hand_type);

        int socket_fd = -1;
        std::string interface;
        std::atomic<bool> is_shutting_down{false};

        std::mutex mutex_comm; // 统一保护Socket读写安全

        // 统计相关
        std::mutex rate_mutex;
        int send_count = 0;
        int recv_count = 0;
        std::chrono::steady_clock::time_point last_stat_time;
        void updateRates(bool is_send);
    };
}  // namespace communication
}  // namespace linkerhand

namespace Communication {
    using CanBus = ::linkerhand::communication::CanBus;
}

#endif
#endif
