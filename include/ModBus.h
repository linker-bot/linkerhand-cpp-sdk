#ifndef LINKERHAND_MOD_BUS_H
#define LINKERHAND_MOD_BUS_H
#if USE_RMAN
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <mutex>
#include <atomic>

#include <unistd.h>
#include <chrono>
#include <thread>
#include <queue>
#include <unistd.h>
#include <fcntl.h>
#include <iomanip>
#include <sstream>
#include "rm_service.h"
#include "Common.h"
#include "ICommunication.h"

namespace linkerhand {
namespace communication {

/**
 * @brief ModBus 通信实现
 */
class ModBus : public ICommunication
    {
    public:
        ModBus(uint32_t handId);
        ~ModBus();

        void send(const std::vector<uint8_t>& data, uint32_t &id, const int &start_address = 0, const int &num = 0) override;
        std::vector<uint8_t> recv(uint32_t& id, const int &start_address = 0, const int &num = 0) override;

        bool writeHoldingRegister(const int &id, const int &start_address, const int &num, const std::vector<uint8_t> &data);
        std::vector<uint8_t> readHoldingRegister(const int &id, const int &start_address, const int &num);

    private:
        uint32_t handId;
        int result = -1;
        rm_robot_handle *robot_handle;
        RM_Service robotic_arm;

        std::mutex send_mutex;
    };
} // namespace communication
} // namespace linkerhand

// 向后兼容：在全局命名空间中提供别名
namespace Communication = linkerhand::communication;

#endif
#endif // LINKERHAND_MOD_BUS_H
