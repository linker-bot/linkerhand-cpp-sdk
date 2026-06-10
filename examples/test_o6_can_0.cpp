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
        std::shared_ptr<LinkerHandApi> hand = std::make_shared<LinkerHandApi>(LINKER_HAND::O6, HAND_TYPE::LEFT);

        // 按手别创建 CAN 总线对象
        std::shared_ptr<Communication::ICanBus> bus = Communication::CommFactory::createCanBus(HAND_TYPE::LEFT);

        hand->setCanTxCallback([bus](uint32_t can_id, const uint8_t *data, uintptr_t data_len) -> int32_t {
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
            bus->send(data_vec, can_id);
            return 0;
        });

        hand->setCanRxCallback([bus](uint32_t* can_id_out, uint8_t* data_out, uint8_t* len_out) -> int32_t {
            auto frame = bus->recv();
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

        for (size_t i = 0; i < 1; ++i) {
            std::cout << hand->getVersion() << std::endl;

            std::cout << "setTorque----------------------------------------" << std::endl;
            hand->setTorque({200,200,200,200,200,200});
            std::cout << "setSpeed----------------------------------------" << std::endl;
            hand->setSpeed({200,200,200,200,200,200});

            // 握拳
            std::vector<uint8_t> fist_pose = {255, 255, 0, 0, 0, 0};
            hand->fingerMove(fist_pose);
            std::this_thread::sleep_for(std::chrono::seconds(1));

            // 张开
            std::vector<uint8_t> open_pose = {255, 104, 255, 255, 255, 255};
            hand->fingerMove(open_pose);
            std::this_thread::sleep_for(std::chrono::seconds(1));

            for (size_t k = 0; k < 3; k++) {
                std::cout << "getTorque----------------------------------------" << std::endl;
                std::vector<uint8_t> torque = hand->getTorque();
                std::cout << "torque size:" << torque.size() << std::endl;
                for (size_t j = 0; j < torque.size(); j++) {
                    std::cout << (int)torque[j] << " ";
                }
                std::cout << std::endl;

                std::cout << "getSpeed----------------------------------------" << std::endl;
                std::vector<uint8_t> speed = hand->getSpeed();
                std::cout << "speed size:" << speed.size() << std::endl;
                for (size_t j = 0; j < speed.size(); j++) {
                    std::cout << (int)speed[j] << " ";
                }
                std::cout << std::endl;

                std::cout << "getState----------------------------------------" << std::endl;
                std::vector<uint8_t> state = hand->getState();
                std::cout << "state size:" << state.size() << std::endl;
                for (size_t j = 0; j < state.size(); j++) {
                    std::cout << (int)state[j] << " ";
                }
                std::cout << std::endl;

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            std::vector<std::vector<std::vector<uint8_t>>> touch_mats = hand->getForce();
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
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
