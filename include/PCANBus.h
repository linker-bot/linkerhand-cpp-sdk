#ifdef _WIN32
#ifndef LINKERHAND_PCAN_BUS_H
#define LINKERHAND_PCAN_BUS_H

#include <iostream>
#include <windows.h>
#include <PCANBasic.h>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <queue>
#include <sstream>
#include <iomanip>
#include <thread>

#include "ICanBus.h"

namespace linkerhand {
namespace communication {

/**
 * @brief Windows PCAN 总线实现
 */
class PCANBus : public ICanBus
    {
    public:
        PCANBus(TPCANHandle channel, TPCANBaudrate bitrate, const LINKER_HAND linkerhand);
        ~PCANBus();
        std::string printMillisecondTime();
        void send(const std::vector<uint8_t>& data, uint32_t can_id, const bool wait = false) override;
        CANFrame recv(uint32_t &id) override;
        void updateSendRate();
        void updateReceiveRate();

    private:
        TPCANHandle channel;
        LINKER_HAND linker_hand;
        std::mutex mutex_send;
        std::mutex mutex_receive;
        std::mutex send_mutex;
        std::atomic<int> send_count;
        std::chrono::steady_clock::time_point last_time;
        std::mutex receive_mutex;
        std::atomic<int> receive_count;
        std::chrono::steady_clock::time_point receive_last_time;
        std::queue<TPCANMsg> send_queue;
    };
} // namespace communication
} // namespace linkerhand

// 向后兼容：在全局命名空间中提供别名
namespace Communication = linkerhand::communication;

#endif // LINKERHAND_PCAN_BUS_H
#endif
