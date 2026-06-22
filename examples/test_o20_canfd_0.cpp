// O20 / CAN-FD / 右手 —— 版本/动作/速度最小示例
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include "_win_console_utf8.h"
#include "LinkerHandApi.h"
#include "CommunicationCallbacks.h"
#include "CanFD.h"

int main() {
    std::cout << "O20 CANFD Test" << std::endl;

    // 初始化 CANFD 设备
    Communication::CanFD canfd(0, 0);
    if (!canfd.init()) {
        std::cerr << "Failed to initialize CANFD" << std::endl;
        return 1;
    }
    std::cout << "CANFD initialized successfully" << std::endl;

    try {
        LinkerHandApi api(LINKER_HAND::O20, HAND_TYPE::RIGHT, COMM_TYPE::CAN);

        std::cout << "LinkerHandApi created" << std::endl;

        auto can_tx_callback = [&](uint32_t can_id, const uint8_t* data, uintptr_t data_len) -> int32_t {
            std::vector<uint8_t> data_vec(data, data + data_len);
            canfd.send(data_vec, can_id, true);
            return 0;
        };

        auto can_rx_callback = [&](uint32_t* can_id_out, uint8_t* data_out, uint8_t* data_len_out) -> int32_t {
            Communication::CanFDFrame frame = canfd.recv(10);
            if (frame.valid) {
                *can_id_out = frame.can_id;
                *data_len_out = Communication::CanFD::dlcToLen(frame.can_dlc);
                memcpy(data_out, frame.data, *data_len_out);
                return 0;
            }
            return -1;
        };

        std::cout << "RX callback set" << std::endl;

        api.setCanTxCallback(can_tx_callback);
        api.setCanRxCallback(can_rx_callback);

        std::cout << "Callbacks registered, sleeping 1 second..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout << "Calling getVersion()..." << std::endl;
        std::string version = api.getVersion();
        std::cout << "Version: " << version << std::endl;

        std::vector<uint8_t> pos(34, 0);
        std::cout << "Calling setPosition()..." << std::endl;
        api.setPosition(pos);

        std::cout << "Calling getSpeed()..." << std::endl;
        std::vector<uint8_t> speed = api.getSpeed();
        
        for (size_t i = 0; i < speed.size(); i++) {
            std::cout << (int)speed[i] << std::dec << " ";
        }
        std::cout << std::dec << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        canfd.close();
        return 1;
    }

    canfd.close();
    std::cout << "Test completed" << std::endl;
    return 0;
}