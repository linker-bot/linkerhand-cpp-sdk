// G20 / CAN / 左手 —— 握拳/张开动作循环
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include "_win_console_utf8.h"
#include "LinkerHandApi.h"
#include "CommFactory.h"

#define HAND_SPEED 200

int main(int argc, char *argv[]) {
    try {
        std::cout << "=========================\n";
        std::cout << "    G20 CAN Test\n";
        std::cout << "=========================\n";

        std::string channel_name = "can0";
        if (argc > 1) {
            channel_name = argv[1];
        }

        // 创建 CAN 总线对象（工厂按平台分流 SocketCAN / PCAN）
        std::shared_ptr<Communication::ICanBus> bus = Communication::CommFactory::createCanBus(channel_name, 1000000);

        // 创建 G20 手对象
        LinkerHandApi hand(LINKER_HAND::G20, HAND_TYPE::LEFT, COMM_TYPE::CAN);

        // 设置 CAN 发送回调
        hand.setCanTxCallback([bus](uint32_t can_id, const uint8_t *data, uintptr_t data_len) -> int32_t {
            std::vector<uint8_t> data_vec(data, data + data_len);
            bus->send(data_vec, can_id);
            return 0;
        });

        // 设置 CAN 接收回调
        hand.setCanRxCallback([bus](uint32_t* can_id_out, uint8_t* data_out, uint8_t* len_out) -> int32_t {
            auto frame = bus->recv();
            if (frame.can_id == 0 && frame.can_dlc == 0) {
                return -1;
            }
            *can_id_out = frame.can_id;
            *len_out = frame.can_dlc;
            memcpy(data_out, frame.data, frame.can_dlc);
            return 0;
        });

        // 设置速度
        hand.setSpeed(std::vector<uint8_t>(5, HAND_SPEED));

        // 获取版本信息
        std::cout << "Version: " << hand.getVersion() << std::endl;

        // G20 握拳动作
        std::cout << "G20 - Execute action - Make a fist" << std::endl;
        std::vector<uint8_t> G20_POSE_CLOSE_1 = {255, 0, 0, 0, 0, 255, 255, 178, 84, 0, 255, 255, 0, 0, 0, 0};
        hand.fingerMove(G20_POSE_CLOSE_1);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        std::vector<uint8_t> G20_POSE_CLOSE_2 = {117, 0, 0, 0, 0, 47, 255, 178, 84, 0, 115, 104, 0, 0, 0, 0};
        hand.fingerMove(G20_POSE_CLOSE_2);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // G20 张开动作
        std::cout << "G20 - Execute action - Open hand" << std::endl;
        std::vector<uint8_t> G20_POSE_OPEN = {255, 255, 255, 255, 255, 255, 255, 172, 74, 0, 255, 255, 255, 255, 255, 255};
        hand.fingerMove(G20_POSE_OPEN);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 循环测试
        for (int i = 0; i < 3; ++i) {
            std::cout << "\nLoop " << i + 1 << ":\n";

            // 握拳
            std::cout << "Make a fist..." << std::endl;
            hand.fingerMove(G20_POSE_CLOSE_1);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            hand.fingerMove(G20_POSE_CLOSE_2);
            std::this_thread::sleep_for(std::chrono::seconds(1));

            // 张开
            std::cout << "Open hand..." << std::endl;
            hand.fingerMove(G20_POSE_OPEN);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << "\nTest completed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
