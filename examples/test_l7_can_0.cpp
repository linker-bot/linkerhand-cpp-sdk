// L7 / CAN / 右手 —— 读取版本/状态/速度/扭矩/温度/故障码/触觉
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <iomanip>
#include <memory>
#include <array>

#include "_win_console_utf8.h"
#include "LinkerHandApi.h"
#include "CommFactory.h"

int main() {
    try {
        // 创建 CAN 总线对象
        std::shared_ptr<Communication::ICanBus> canBus = Communication::CommFactory::createCanBus("can0", 1000000);

        LinkerHandApi hand(LINKER_HAND::L7, HAND_TYPE::RIGHT);
        std::cout << "LinkerHandL7 Real CAN Test Program" << std::endl;

        // 设置 CAN 发送回调
        hand.setCanTxCallback([canBus](uint32_t can_id, const uint8_t *data, uintptr_t data_len) -> int32_t {
            std::vector<uint8_t> dataVec(data, data + data_len);
            canBus->send(dataVec, can_id);
            return 0;
        });

        // 设置 CAN 接收回调
        hand.setCanRxCallback([canBus](uint32_t* can_id_out, uint8_t* data_out, uint8_t* data_len_out) -> int32_t {
            auto frame = canBus->recv();
            if (frame.can_dlc > 0) {
                *can_id_out = frame.can_id;
                *data_len_out = frame.can_dlc;
                memcpy(data_out, frame.data, frame.can_dlc);
                std::cout << "Received frame: can_id=" << frame.can_id << ", data=";
                for (int i = 0; i < frame.can_dlc; ++i) {
                    std::cout << std::hex << (int)frame.data[i] << " ";
                }
                std::cout << std::dec << std::endl;
                return 0;
            }
            return -1;
        });

        // 等待连接建立
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 获取版本信息
        std::cout << "Version Info:" << std::endl;
        std::cout << hand.getVersion() << std::endl;

        // 获取当前状态
        std::cout << "Getting current status..." << std::endl;
        auto currentStatus = hand.getState();
        std::cout << "Current status: ";
        for (auto pos : currentStatus) {
            std::cout << (int)pos << " ";
        }
        std::cout << std::endl;

        // 设置速度（7 个关节）
        std::vector<uint8_t> speeds = {255, 255, 255, 255, 255, 255, 255};
        std::cout << "Setting joint speeds..." << std::endl;
        hand.setSpeed(speeds);

        std::cout << "Getting current speeds..." << std::endl;
        auto currentSpeeds = hand.getSpeed();
        std::cout << "Current speeds: ";
        for (auto speed : currentSpeeds) {
            std::cout << (int)speed << " ";
        }
        std::cout << std::endl;

        // 设置扭矩（7 个关节）
        std::vector<uint8_t> torques = {255, 255, 255, 255, 255, 255, 255};
        std::cout << "Setting torques..." << std::endl;
        hand.setTorque(torques);

        std::cout << "Getting current torques..." << std::endl;
        auto currentTorques = hand.getTorque();
        std::cout << "Current torques: ";
        for (auto torque : currentTorques) {
            std::cout << (int)torque << " ";
        }
        std::cout << std::endl;

        // 获取温度信息
        std::cout << "Getting temperature..." << std::endl;
        auto temperatures = hand.getTemperature();
        std::cout << "Temperatures: ";
        for (auto temp : temperatures) {
            std::cout << (int)temp << " ";
        }
        std::cout << std::endl;

        // 获取故障码
        std::cout << "Getting fault codes..." << std::endl;
        auto faultCodes = hand.getFaultCode();
        std::cout << "Fault codes: ";
        for (auto code : faultCodes) {
            std::cout << (int)code << " ";
        }
        std::cout << std::endl;

        // 获取压力数据
        std::cout << "Getting force data..." << std::endl;
        std::vector<std::vector<std::vector<uint8_t>>> touch_mats = hand.getForce();
        // 手指名称表，顺序对应 touch_mats[0..4]
        const std::array<const char *, 5> finger_name = {
            "THUMB_TOUCH", "INDEX_TOUCH", "MIDDLE_TOUCH", "RING_TOUCH", "LITTLE_TOUCH"};
        for (size_t n = 0; n < touch_mats.size(); ++n) {
            std::cout << finger_name[n] << ":\n";
            for (const auto &row : touch_mats[n]) {
                for (uint8_t val : row)
                    std::cout << std::setw(3) << static_cast<int>(val) << ' ';
                std::cout << '\n';
            }
            std::cout << '\n';
        }

        std::cout << "Test completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
