// O20 / CAN-FD / 右手 —— 版本/速度/动作序列/触觉（带 TX/RX 日志）
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <array>
#include "_win_console_utf8.h"
#include "LinkerHandApi.h"
#include "CommunicationCallbacks.h"
#include "CanFD.h"

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "    O20 CANFD Test Program (Example 1)" << std::endl;
    std::cout << "========================================" << std::endl;

    Communication::CanFD canfd(0, 0);
    if (!canfd.init()) {
        std::cerr << "Failed to initialize CANFD" << std::endl;
        return 1;
    }
    std::cout << "CANFD initialized successfully" << std::endl;

    try {
        LinkerHandApi api(LINKER_HAND::O20, HAND_TYPE::RIGHT, COMM_TYPE::CAN);

        std::cout << "LinkerHandApi created for O20 (RIGHT)" << std::endl;

        auto can_tx_callback = [&](uint32_t can_id, const uint8_t* data, uintptr_t data_len) -> int32_t {
            std::vector<uint8_t> data_vec(data, data + data_len);
            canfd.send(data_vec, can_id, true);
            std::cout << "\033[32m[TX] ID: 0x" << std::hex << std::setw(8) << std::setfill('0') << can_id 
                      << std::dec << " | Len: " << (int)data_len << " | Data: ";
            for (uintptr_t i = 0; i < data_len; i++) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)data[i] << " ";
            }
            std::cout << std::dec << std::endl;
            return 0;
        };

        auto can_rx_callback = [&](uint32_t* can_id_out, uint8_t* data_out, uint8_t* data_len_out) -> int32_t {
            Communication::CanFDFrame frame = canfd.recv(10);
            if (frame.valid) {
                *can_id_out = frame.can_id;
                *data_len_out = Communication::CanFD::dlcToLen(frame.can_dlc);
                memcpy(data_out, frame.data, *data_len_out);
                std::cout << "\033[34m[RX] ID: 0x" << std::hex << std::setw(8) << std::setfill('0') << *can_id_out 
                          << std::dec << " | Len: " << (int)*data_len_out << std::endl;
                return 0;
            }
            return -1;
        };

        api.setCanTxCallback(can_tx_callback);
        api.setCanRxCallback(can_rx_callback);

        std::cout << "Callbacks registered, connecting..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout << std::endl << "--- Test 1: Get Version ---" << std::endl;
        std::string version = api.getVersion();
        std::cout << "Version: " << version << std::endl;

        std::cout << std::endl << "--- Test 2: Set Speed ---" << std::endl;
        std::vector<uint8_t> speed(34, 128);
        api.setSpeed(speed);
        std::cout << "Speed set to 128 for all 34 joints" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        std::cout << std::endl << "--- Test 3: Get Speed ---" << std::endl;
        std::vector<uint8_t> current_speed = api.getSpeed();
        std::cout << "Current speed size: " << current_speed.size() << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        std::cout << std::endl << "--- Test 4: Finger Movement Sequence ---" << std::endl;
        std::vector<uint8_t> poses_seq(17, 0);
        for (size_t i = 0; i < 17; ++i) {
            std::vector<uint8_t> poses_temp = poses_seq;
            poses_temp[i] = 255;
            api.setPosition(poses_temp);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << std::endl << "--- Test 5: Get Force Data ---" << std::endl;
        std::vector<std::vector<std::vector<uint8_t>>> touch_mats = api.getForce();
        const std::array<const char*, 5> finger_name = {
            "THUMB_TOUCH", "INDEX_TOUCH", "MIDDLE_TOUCH", "RING_TOUCH", "LITTLE_TOUCH"
        };

        for (size_t n = 0; n < touch_mats.size() && n < 5; ++n) {
            std::cout << finger_name[n] << ": ";
            size_t total_rows = touch_mats[n].size();
            std::cout << total_rows << " rows" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    } catch (const std::exception& e) {
        std::cerr << "\033[31mError: " << e.what() << "\033[0m" << std::endl;
        canfd.close();
        return 1;
    }

    canfd.close();
    std::cout << std::endl << "========================================" << std::endl;
    std::cout << "O20 CANFD Test completed successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}