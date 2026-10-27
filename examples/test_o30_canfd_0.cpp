// O30 / 厂商 libcanbus CAN-FD / 右手 —— HOP 协议最小示例
//
// O30 采用 HOP 协议（数据域 = [ctrl(MI+RTS), SI, EDL, payload...]），
// 首版走 CAN FD + 标准帧(11 位)：请求 ID=0x001，响应 ID=0x401。
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
    std::cout << "O30 CANFD (HOP) Test" << std::endl;

    Communication::CanFD canfd(0, 0);
    if (!canfd.init()) {
        std::cerr << "Failed to initialize CANFD" << std::endl;
        return 1;
    }
    std::cout << "CANFD initialized successfully" << std::endl;

    try {
        LinkerHandApi api(LINKER_HAND::O30, HAND_TYPE::RIGHT, COMM_TYPE::CAN);
        std::cout << "LinkerHandApi created" << std::endl;

        // O30 HOP 用标准帧：is_extended = false
        auto can_tx_callback = [&](uint32_t can_id, const uint8_t* data, uintptr_t data_len) -> int32_t {
            std::vector<uint8_t> data_vec(data, data + data_len);
            canfd.send(data_vec, can_id, /*is_extended=*/false);
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

        api.setCanTxCallback(can_tx_callback);
        api.setCanRxCallback(can_rx_callback);

        std::cout << "Callbacks registered, sleeping 1 second..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout << "Calling getVersion()..." << std::endl;
        std::string version = api.getVersion();
        std::cout << "Version:\n" << version << std::endl;

        // O30 对外为 20 个有效电机（SDK 已剔除无效占位）
        std::vector<uint8_t> pos(20, 128);
        std::cout << "Calling setPosition()..." << std::endl;
        api.setPosition(pos);

        std::cout << "Calling getSpeed()..." << std::endl;
        std::vector<uint8_t> speed = api.getSpeed();
        for (size_t i = 0; i < speed.size(); i++) std::cout << (int)speed[i] << " ";
        std::cout << std::endl;

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
