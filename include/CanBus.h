#ifdef __linux__
#ifndef CAN_BUS_H
#define CAN_BUS_H

#include <iostream>
#include <iomanip>
#include <sstream>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <queue>
#include <fcntl.h>
#include <mutex>
#include <atomic>
#include <cstring>
#include <cerrno>

#include "Common.h"

#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>  // 包含 ifreq 和 IFNAMSIZ
#include <sys/ioctl.h>  // 包含 ioctl

#include "ICanBus.h"

namespace Communication //Communicator
{
    class CanBus : public ICanBus
    {
    public:
        CanBus(const std::string& interface, int bitrate, const LINKER_HAND linkerhand);
        ~CanBus();
        
        void send(const std::vector<uint8_t>& data, uint32_t can_id, const bool wait = false) override;
        CANFrame recv(uint32_t& id) override;
        
        void setReceiveTimeout(int seconds, int microseconds);
        void updateSendRate();
        void updateReceiveRate();
        std::string printMillisecondTime();

    private:
        int socket_fd;
        std::string interface;
        int bitrate;
        struct sockaddr_can addr;
        struct ifreq ifr;  // 确保 ifreq 是完整的类型
        std::mutex mutex_send; // 互斥锁
        std::mutex mutex_send2; // 互斥锁
        std::mutex mutex_receive; // 互斥锁

        std::mutex send_mutex; // 用于保护发送计数器和时间
        std::atomic<int> send_count; // 发送计数器
        std::chrono::steady_clock::time_point last_time; // 上一次记录时间

        std::mutex receive_mutex; // 用于保护发送计数器和时间
        std::atomic<int> receive_count; // 发送计数器
        std::chrono::steady_clock::time_point receive_last_time; // 上一次记录时间
        std::queue<struct can_frame> send_queue; // 发送队列

        LINKER_HAND linker_hand;
    };
}
#endif
#endif // CAN_BUS_H

