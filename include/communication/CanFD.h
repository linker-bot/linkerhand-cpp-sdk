#ifndef CANFD_H
#define CANFD_H

#include <string>
#include <mutex>
#include <atomic>
#include <chrono>
#include "communication/ICanFD.h"
#include "Hcanbus.h"
#include "core/LinkerHandExport.h"

namespace linkerhand {
namespace communication
{
    class LINKERHAND_API CanFD : public ICanFD
    {
    public:
        CanFD(uint32_t dev_num = 0, uint32_t ch_num = 0);
        ~CanFD();

        bool init();
        void close();
        bool isOpen() const override;

        void send(const std::vector<uint8_t>& data, uint32_t can_id, bool is_extended = true) override;
        CanFDFrame recv(int timeout_ms = 100) override;

        static int scanDevices();
        static bool readDeviceInfo(uint32_t dev_num, ::Dev_Info& info);

        static uint8_t lenToDLC(uint8_t len);
        static uint8_t dlcToLen(uint8_t dlc);

    private:
        uint32_t m_dev_num;
        uint32_t m_ch_num;
        std::atomic<bool> m_is_open;
        std::mutex m_tx_mutex;
        std::mutex m_rx_mutex;
    };
}  // namespace communication
}  // namespace linkerhand

namespace Communication {
    using CanFD = ::linkerhand::communication::CanFD;
}

#endif // CANFD_H
