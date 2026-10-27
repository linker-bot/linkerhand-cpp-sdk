// O30 / SocketCAN 原生 CAN-FD / 右手 —— HOP 协议联调测试
//
// O30 采用 HOP 协议（数据域 = [ctrl(MI+RTS), SI, EDL, payload...]），
// 首版走 CAN FD + 标准帧(11 位)：请求 ID=0x001，响应 ID=0x401。
//
// 使用前先在内核侧配置 can0（CAN FD，示例 arb 1Mbps / data 5Mbps，按设备实际配置）：
//   sudo ip link set can0 down
//   sudo ip link set can0 type can bitrate 1000000 dbitrate 5000000 fd on restart-ms 100
//   sudo ip link set can0 up
//   sudo ip link set can0 txqueuelen 1000
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include "_win_console_utf8.h"
#include "LinkerHandApi.h"
#include "CommunicationCallbacks.h"
#include "CanFDSocket.h"

int main(int argc, char** argv) {
    const char* ifname = (argc > 1) ? argv[1] : "can0";
    std::cout << "O30 SocketCAN CANFD (HOP) Test on " << ifname << std::endl;

    Communication::CanFDSocket canfd(ifname);
    if (!canfd.init()) {
        std::cerr << "Failed to open SocketCAN CANFD (" << ifname << ")" << std::endl;
        return 1;
    }
    std::cout << "SocketCAN CANFD opened successfully" << std::endl;

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
                *data_len_out = frame.can_dlc;  // SocketCAN 变体：can_dlc 即实际字节长度
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

        std::cout << "Calling getPosition()..." << std::endl;
        std::vector<uint8_t> pos0 = api.getPosition();
        std::cout << "pos(" << pos0.size() << "): ";
        for (size_t i = 0; i < pos0.size(); i++) std::cout << (int)pos0[i] << " ";
        std::cout << std::endl;

        std::cout << "Calling getFaultCode()..." << std::endl;
        std::vector<uint8_t> err = api.getFaultCode();
        std::cout << "err(" << err.size() << "): ";
        for (size_t i = 0; i < err.size(); i++) std::cout << (int)err[i] << " ";
        std::cout << std::endl;

        std::cout << "Calling getForce()..." << std::endl;
        auto force = api.getForce();
        for (size_t finger = 0; finger < force.size(); ++finger) {
            const size_t rows = force[finger].size();
            const size_t cols = rows == 0 ? 0 : force[finger][0].size();
            std::cout << "force[" << finger << "]: " << rows << "x" << cols;
            if (rows > 0 && cols > 0) {
                std::cout << ", first=" << static_cast<int>(force[finger][0][0]);
            }
            std::cout << std::endl;
        }

        // 写五指指尖张开/握拳循环。O30 对外为 20 个有效电机，
        // 五指指尖在有效向量中的索引为 15~19（拇/食/中/无名/小指尖）。
        for (int cycle = 0; cycle < 2; ++cycle) {
            uint8_t val = (cycle % 2 == 0) ? 200 : 40;
            std::vector<uint8_t> pos(20, 128);
            for (int i = 15; i <= 19; ++i) pos[i] = val;  // 五指指尖
            std::cout << "setPosition(tips=" << (int)val << ")..." << std::endl;
            api.setPosition(pos);
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));

            std::vector<uint8_t> cur = api.getPosition();
            std::cout << "  pos: ";
            for (size_t i = 0; i < cur.size(); i++) std::cout << (int)cur[i] << " ";
            std::cout << std::endl;
        }

        std::cout << "Calling getSpeed()..." << std::endl;
        std::vector<uint8_t> speed = api.getSpeed();
        std::cout << "speed(" << speed.size() << "): ";
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
