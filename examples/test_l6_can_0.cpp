// L6 / CAN / 右手 —— 读取版本/状态并演示五指压感(B1~B5)与全掌压感(B6，带 TX/RX 日志)
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
        // 初始化灵巧手（右手，与 candump CAN_ID 0x27 对应）
        std::shared_ptr<LinkerHandApi> hand = std::make_shared<LinkerHandApi>(LINKER_HAND::L6, HAND_TYPE::RIGHT);

        // 按手别创建 CAN 总线对象
        std::shared_ptr<Communication::ICanBus> bus = Communication::CommFactory::createCanBus(HAND_TYPE::RIGHT);

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
            return 0;
        });

        std::cout << "----------------------------------------" << std::endl;

        // 打印版本 + SN（SN 里的厂商标识决定 B6 请求报文如何下发）
        std::cout << hand->getVersion() << std::endl;

        hand->setTorque({200, 200, 200, 200, 200, 200});
        hand->setSpeed({200, 200, 200, 200, 200, 200});

        // 握拳 / 张开
        hand->setPosition({255, 255, 0, 0, 0, 0});
        std::this_thread::sleep_for(std::chrono::seconds(1));
        hand->setPosition({255, 104, 255, 255, 255, 255});
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 五指矩阵压感 B1~B5
        std::cout << "getForce（五指矩阵）----------------------------------------" << std::endl;
        std::vector<std::vector<std::vector<uint8_t>>> touch_mats = hand->getForce();
        const std::array<const char*, 5> finger_name = {
            "THUMB_TOUCH", "INDEX_TOUCH", "MIDDLE_TOUCH", "RING_TOUCH", "LITTLE_TOUCH"
        };
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        for (size_t n = 0; n < touch_mats.size(); ++n) {
            std::cout << finger_name[n] << ":\n";
            for (const auto &row : touch_mats[n]) {
                for (uint8_t val : row)
                    std::cout << std::setw(2) << static_cast<int>(val) << ' ';
                std::cout << '\n';
            }
            std::cout << '\n';
        }

        // 全掌压感 B6（尺寸随厂商/固件动态变化，palm_mat 按帧行列扩容）
        std::cout << "getPalmForce（全掌压感）----------------------------------------" << std::endl;
        hand->getPalmForce();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::vector<std::vector<uint8_t>> palm = hand->getPalmForce();
        std::cout << "palm rows: " << palm.size() << std::endl;
        for (const auto &row : palm) {
            for (uint8_t val : row)
                std::cout << std::setw(3) << static_cast<int>(val) << ' ';
            std::cout << '\n';
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
