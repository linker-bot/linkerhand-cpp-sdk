// O6 / CAN / 左手 —— 双线程并发：一线程读触觉，一线程设速度/扭矩/动作
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
#include <array>

#include "_win_console_utf8.h"
#include "LinkerHandApi.h"
#include "CommunicationCallbacks.h"
#include "CommFactory.h"

// 辅助函数：打印向量数据
void printVector(const std::string& label, const std::vector<uint8_t>& vec) {
    std::cout << label << ": [";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << static_cast<int>(vec[i]);
        if (i < vec.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

int main(int argc, char* argv[]) {
    try {
        std::cout << "========================================" << std::endl;
        std::cout << "    O6 CAN 测试示例 (多线程并发测试)" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << std::endl;

        std::string channel_name = "can0";
        if (argc > 1) {
            channel_name = argv[1];
        }

        // 创建 CAN 总线对象（工厂按平台分流 SocketCAN / PCAN）
        std::shared_ptr<Communication::ICanBus> bus = Communication::CommFactory::createCanBus(channel_name, 1000000);

        // 1. 初始化手部 API
        LinkerHandApi hand(LINKER_HAND::O6, HAND_TYPE::LEFT, COMM_TYPE::CAN);

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

        std::cout << "初始化完成！" << std::endl;
        std::cout << std::endl;

        // 线程：循环读取压感数据
        std::thread get_thread([&hand]() {
            std::cout << "----------------------------------------" << std::endl;
            std::cout << "读取压感数据" << std::endl;
            std::cout << "----------------------------------------" << std::endl;
            for (int i = 0; i < 100; ++i) {
                std::vector<std::vector<std::vector<uint8_t>>> touch_mats = hand.getForce();
                // 手指名称表，顺序对应 touch_mats[0..4]
                const std::array<const char*, 5> finger_name = {
                    "THUMB_TOUCH", "INDEX_TOUCH", "MIDDLE_TOUCH", "RING_TOUCH", "LITTLE_TOUCH"
                };
                for (size_t n = 0; n < touch_mats.size(); ++n) {
                    std::cout << finger_name[n] << ":\n";
                    for (const auto &row : touch_mats[n]) {
                        for (uint8_t val : row)
                            std::cout << std::setw(2) << static_cast<int>(val) << ' ';
                        std::cout << '\n';
                    }
                    std::cout << '\n';
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });

        // 线程：设置速度/扭矩并执行手指动作序列
        std::thread set_thread([&hand]() {
            for (int i = 0; i < 3; ++i) {
                std::cout << "----------------------------------------" << std::endl;
                std::cout << "设置速度" << std::endl;
                std::cout << "----------------------------------------" << std::endl;
                std::vector<uint8_t> speed(6, 128);  // 6个关节，速度值 128
                hand.setSpeed(speed);
                printVector("设置速度", speed);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                std::cout << std::endl;

                std::cout << "----------------------------------------" << std::endl;
                std::cout << "设置扭矩" << std::endl;
                std::cout << "----------------------------------------" << std::endl;
                std::vector<uint8_t> torque(6, 200);  // 6个关节，扭矩值 200
                hand.setTorque(torque);
                printVector("设置扭矩", torque);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                std::cout << std::endl;

                std::cout << "----------------------------------------" << std::endl;
                std::cout << "手指动作序列" << std::endl;
                std::cout << "----------------------------------------" << std::endl;

                // O6 有 6 个关节：大拇指弯曲、大拇指横摆、食指、中指、无名指、小拇指
                std::vector<std::vector<uint8_t>> poses = {
                    {255, 255, 255, 255, 255, 255},  // 完全张开
                    {0, 255, 255, 255, 255, 255},    // 大拇指弯曲
                    {255, 0, 255, 255, 255, 255},    // 大拇指横摆
                    {255, 255, 0, 255, 255, 255},    // 食指弯曲
                    {255, 255, 255, 0, 255, 255},    // 中指弯曲
                    {255, 255, 255, 255, 0, 255},    // 无名指弯曲
                    {255, 255, 255, 255, 255, 0},    // 小拇指弯曲
                    {0, 0, 0, 0, 0, 0},               // 握拳
                    {255, 255, 255, 255, 255, 255}   // 完全张开
                };
                for (size_t j = 0; j < poses.size(); ++j) {
                    std::cout << "动作 " << (j + 1) << "/" << poses.size() << ": ";
                    printVector("位置", poses[j]);
                    hand.setPosition(poses[j]);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
                std::cout << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        });

        get_thread.join();
        set_thread.join();

        std::cout << "========================================" << std::endl;
        std::cout << "测试完成！" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
