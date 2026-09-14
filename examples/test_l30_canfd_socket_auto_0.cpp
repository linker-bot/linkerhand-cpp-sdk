// L30 (L30V6) / SocketCAN 原生 CAN-FD / 自动检测左右手 —— 联调测试
//
// 演示 CanFDSocket(HAND_TYPE) 自动检测构造：遍历所有已启动的 CAN 接口，
// 逐一发 0xC3(CANFDID/左右手) 读请求，命中与期望侧别匹配的接口后自动打开。
// 参考 CanBus(HAND_TYPE) 的 auto_detect_channel 语义。
//
// 使用前先在内核侧把两路 canfd 都配好（协议要求 arb 1Mbps / data 5Mbps）：
//   sudo ip link set canX down
//   sudo ip link set canX type can bitrate 1000000 dbitrate 5000000 fd on restart-ms 100
//   sudo ip link set canX up
//   sudo ip link set canX txqueuelen 1000
//
// 运行： ./test_l30_canfd_socket_auto_0 [left|right]   (默认 right)
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <string>
#include "_win_console_utf8.h"
#include "LinkerHandApi.h"
#include "CommunicationCallbacks.h"
#include "CanFDSocket.h"

int main(int argc, char** argv) {
    HAND_TYPE side = HAND_TYPE::RIGHT;
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "left" || arg == "LEFT" || arg == "l") side = HAND_TYPE::LEFT;
    }
    const char* side_name = (side == HAND_TYPE::LEFT) ? "LEFT" : "RIGHT";
    std::cout << "L30 SocketCAN CANFD auto-detect test, target = " << side_name << std::endl;

    try {
        // 自动检测左右手：内部遍历已启动的 CAN 接口并打开命中的那一路
        Communication::CanFDSocket canfd(side);
        std::cout << "SocketCAN CANFD opened successfully (auto-detected)" << std::endl;

        LinkerHandApi api(LINKER_HAND::L30, side, COMM_TYPE::CAN);
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
                *data_len_out = frame.can_dlc;
                memcpy(data_out, frame.data, *data_len_out);
                return 0;
            }
            return -1;
        };

        api.setCanTxCallback(can_tx_callback);
        api.setCanRxCallback(can_rx_callback);

        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout << "Version:\n" << api.getVersion() << std::endl;

        std::vector<uint8_t> pos = api.getPosition();
        std::cout << "pos(" << pos.size() << "): ";
        for (size_t i = 0; i < pos.size(); i++) std::cout << (int)pos[i] << " ";
        std::cout << std::endl;

        canfd.close();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Test completed" << std::endl;
    return 0;
}
