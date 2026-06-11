#ifdef _WIN32
#ifndef PCAN_BUS_H
#define PCAN_BUS_H

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

#include "core/LinkerHandExport.h"
#include "communication/ICanBus.h"

namespace linkerhand {
namespace communication
{
    class LINKERHAND_API PCANBus : public ICanBus
    {
    public:
        PCANBus(TPCANHandle channel, TPCANBaudrate bitrate);
        PCANBus(HAND_TYPE hand_type);
        ~PCANBus();
        std::string printMillisecondTime();
        void send(const std::vector<uint8_t>& data, uint32_t can_id, const bool wait = false) override;
        CANFrame recv() override;
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

        static std::vector<TPCANHandle> detect_can_interfaces();
        static bool ping_interface(TPCANHandle channel, const struct DetectionJob& job);
        static TPCANHandle auto_detect_channel(HAND_TYPE hand_type);
    };
}  // namespace communication
}  // namespace linkerhand

namespace Communication {
    using PCANBus = ::linkerhand::communication::PCANBus;
}

#endif // PCAN_BUS_H
#endif
