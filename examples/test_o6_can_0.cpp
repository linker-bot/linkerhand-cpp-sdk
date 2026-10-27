// O6 / CAN / 左手 —— 读取版本/力矩/速度/状态/触觉并执行握拳与张开（带 TX/RX 日志）
#include <array>
#include <iostream>
#include <thread>
#include <vector>
#include <iomanip>
#include <memory>

#include "_win_console_utf8.h"
#include "LinkerHandApi.h"
#include "CommFactory.h"

// 格式化当前时间，用于 TX/RX 日志
std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    #if defined(_WIN32) || defined(_WIN64)
        std::tm tm_info = {};
        localtime_s(&tm_info, &now_time);
        std::tm* tm = &tm_info;
    #else
        std::tm* tm = std::localtime(&now_time);
    #endif
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);

    std::ostringstream oss;
    oss << buffer << "." << std::setfill('0') << std::setw(3) << now_ms.count();
    return oss.str();
}

int main() {
    try {
        // 初始化灵巧手
        std::shared_ptr<LinkerHandApi> hand_left = std::make_shared<LinkerHandApi>(LINKER_HAND::O6, HAND_TYPE::LEFT);
        std::shared_ptr<Communication::ICanBus> bus_left = Communication::CommFactory::createCanBus(HAND_TYPE::LEFT);
        
        std::shared_ptr<LinkerHandApi> hand_right = std::make_shared<LinkerHandApi>(LINKER_HAND::O6, HAND_TYPE::RIGHT);
        std::shared_ptr<Communication::ICanBus> bus_right = Communication::CommFactory::createCanBus(HAND_TYPE::RIGHT);
        

        hand_left->setCanTxCallback([bus_left](uint32_t can_id, const uint8_t *data, uintptr_t data_len) -> int32_t {
            static int tx_count = 0;
            tx_count++;
            std::cout << "\033[32m[TX #" << tx_count << "]\033[0m "
                      << getCurrentTime()
                      << " | CAN_ID: 0x" << std::hex << std::setw(8) << std::setfill('0') << can_id
                      << std::dec << " (" << can_id << ")"
                      << " | Len: " << data_len
                      << " | Data: ";
            for (uintptr_t i = 0; i < data_len; i++) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)data[i] << " ";
            }
            std::cout << std::dec << std::endl;

            std::vector<uint8_t> data_vec(data, data + data_len);
            bus_left->send(data_vec, can_id);
            return 0;
        });

        hand_left->setCanRxCallback([bus_left](uint32_t* can_id_out, uint8_t* data_out, uint8_t* len_out) -> int32_t {
            auto frame = bus_left->recv();
            if (frame.can_id == 0 && frame.can_dlc == 0) {
                return -1;
            }
            *can_id_out = frame.can_id;
            *len_out = frame.can_dlc;
            memcpy(data_out, frame.data, frame.can_dlc);

            static int rx_count = 0;
            rx_count++;
            std::cout << "\033[34m[RX #" << rx_count << "]\033[0m "
                      << getCurrentTime()
                      << " | CAN_ID: 0x" << std::hex << std::setw(8) << std::setfill('0') << frame.can_id
                      << std::dec << " (" << frame.can_id << ")"
                      << " | DLC: " << (int)frame.can_dlc
                      << " | Data: ";
            for (int i = 0; i < frame.can_dlc; i++) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)frame.data[i] << " ";
            }
            std::cout << std::dec << std::endl;
            return 0;
        });
        
        hand_right->setCanTxCallback([bus_right](uint32_t can_id, const uint8_t *data, uintptr_t data_len) -> int32_t {
            static int tx_count = 0;
            tx_count++;
            std::cout << "\033[32m[TX #" << tx_count << "]\033[0m "
                      << getCurrentTime()
                      << " | CAN_ID: 0x" << std::hex << std::setw(8) << std::setfill('0') << can_id
                      << std::dec << " (" << can_id << ")"
                      << " | Len: " << data_len
                      << " | Data: ";
            for (uintptr_t i = 0; i < data_len; i++) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)data[i] << " ";
            }
            std::cout << std::dec << std::endl;

            std::vector<uint8_t> data_vec(data, data + data_len);
            bus_right->send(data_vec, can_id);
            return 0;
        });

        hand_right->setCanRxCallback([bus_right](uint32_t* can_id_out, uint8_t* data_out, uint8_t* len_out) -> int32_t {
            auto frame = bus_right->recv();
            if (frame.can_id == 0 && frame.can_dlc == 0) {
                return -1;
            }
            *can_id_out = frame.can_id;
            *len_out = frame.can_dlc;
            memcpy(data_out, frame.data, frame.can_dlc);

            static int rx_count = 0;
            rx_count++;
            std::cout << "\033[34m[RX #" << rx_count << "]\033[0m "
                      << getCurrentTime()
                      << " | CAN_ID: 0x" << std::hex << std::setw(8) << std::setfill('0') << frame.can_id
                      << std::dec << " (" << frame.can_id << ")"
                      << " | DLC: " << (int)frame.can_dlc
                      << " | Data: ";
            for (int i = 0; i < frame.can_dlc; i++) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)frame.data[i] << " ";
            }
            std::cout << std::dec << std::endl;
            return 0;
        });

        std::cout << "----------------------------------------" << std::endl;

        for (size_t i = 0; i < 3; ++i) {
            std::cout << hand_left->getVersion() << std::endl;
            std::cout << hand_right->getVersion() << std::endl;

            std::cout << "setTorque----------------------------------------" << std::endl;
            hand_left->setTorque({200,200,200,200,200,200});
            std::cout << "setSpeed----------------------------------------" << std::endl;
            hand_left->setSpeed({200,200,200,200,200,200});
            std::cout << "setTorque----------------------------------------" << std::endl;
            hand_right->setTorque({200,200,200,200,200,200});
            std::cout << "setSpeed----------------------------------------" << std::endl;
            hand_right->setSpeed({200,200,200,200,200,200});

            // 握拳
            std::vector<uint8_t> fist_pose = {255, 255, 0, 0, 0, 0};
            hand_left->setPosition(fist_pose);
            std::this_thread::sleep_for(std::chrono::seconds(1));

            // 张开
            std::vector<uint8_t> open_pose = {255, 104, 255, 255, 255, 255};
            hand_left->setPosition(open_pose);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            
            
            // 握拳
            hand_right->setPosition(fist_pose);
            std::this_thread::sleep_for(std::chrono::seconds(1));

            // 张开
            hand_right->setPosition(open_pose);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
