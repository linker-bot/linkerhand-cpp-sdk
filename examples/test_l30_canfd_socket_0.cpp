// L30 (L30V6) / SocketCAN 原生 CAN-FD / 右手 —— 联调测试
//
// 使用前先在内核侧配置 can0（协议要求 arb 1Mbps / data 5Mbps）：
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
    std::cout << "L30 SocketCAN CANFD Test on " << ifname << std::endl;

    Communication::CanFDSocket canfd(ifname);
    if (!canfd.init()) {
        std::cerr << "Failed to open SocketCAN CANFD (" << ifname << ")" << std::endl;
        return 1;
    }
    std::cout << "SocketCAN CANFD opened successfully" << std::endl;

    try {
        LinkerHandApi api(LINKER_HAND::L30, HAND_TYPE::RIGHT, COMM_TYPE::CAN);
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

        std::cout << "Calling getTemperature()..." << std::endl;
        std::vector<uint8_t> temp = api.getTemperature();
        std::cout << "temp(" << temp.size() << "): ";
        for (size_t i = 0; i < temp.size(); i++) std::cout << (int)temp[i] << " ";
        std::cout << std::endl;

        std::cout << "Calling getFaultCode()..." << std::endl;
        std::vector<uint8_t> err = api.getFaultCode();
        std::cout << "err(" << err.size() << "): ";
        for (size_t i = 0; i < err.size(); i++) std::cout << (int)err[i] << " ";
        std::cout << std::endl;

        // 使能全部关节：先失能再使能，制造 0→1 边沿（实测该固件使能为边沿触发，
        // 使能标志已为1时再写1不会重新上电，须先失能再使能）
        std::cout << "Calling setDisable() then setMotorEnable()..." << std::endl;
        api.setDisable(std::vector<uint8_t>(17, 0));
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        api.setEnable(std::vector<uint8_t>(17, 1));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // 设置速度（0-255 → 0~32767；取较低速，避免快速运动堵转）
        std::cout << "Calling setSpeed(50)..." << std::endl;
        api.setSpeed(std::vector<uint8_t>(17, 50));
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        // 张开 / 半握循环。
        // 注意：当前各关节 0-255 → 原始量程为“占位满量程”映射，会把小行程关节
        // （侧摆/旋转）顶到机械极限而堵转，固件会静默保护性关闭个别电机。
        // 因此这里目标取适中值(120/20)，不顶到底；待接入真实 URDF 量程后可放开。
        for (int cycle = 0; cycle < 2; ++cycle) {
            uint8_t val = (cycle % 2 == 0) ? 120 : 20;
            std::cout << "setPosition(" << (int)val << ")..." << std::endl;
            api.setPosition(std::vector<uint8_t>(17, val));
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));

            std::vector<uint8_t> cur = api.getPosition();  // 同步读，返回新到应答
            std::cout << "  pos: ";
            for (size_t i = 0; i < cur.size(); i++) std::cout << (int)cur[i] << " ";
            std::cout << std::endl;
        }

        std::cout << "Calling getSpeed()..." << std::endl;
        std::vector<uint8_t> speed = api.getSpeed();
        std::cout << "speed(" << speed.size() << "): ";
        for (size_t i = 0; i < speed.size(); i++) std::cout << (int)speed[i] << " ";
        std::cout << std::endl;

        std::cout << "Calling getForce() (thumb sample)..." << std::endl;
        auto force = api.getForce();
        if (!force.empty()) {
            std::cout << "  thumb[0]: ";
            for (size_t k = 0; k < force[0][0].size(); ++k) std::cout << (int)force[0][0][k] << " ";
            std::cout << std::endl;
        }

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
