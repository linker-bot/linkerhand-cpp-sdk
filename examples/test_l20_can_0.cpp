// L20 / CAN / 左手 —— 读取状态/触觉并执行握拳与张开（带 TX/RX 日志）
#include <array>
#include <cstdint>

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

double scale_value(double original_value, double a_min, double a_max, double b_min, double b_max)
{
    return (original_value - a_min) * (b_max - b_min) / (a_max - a_min) + b_min;
}

int main() {
    try {
        // 初始化灵巧手
        LinkerHandApi hand(LINKER_HAND::L20, HAND_TYPE::RIGHT);

        // 创建 CAN 总线对象
        std::shared_ptr<Communication::ICanBus> bus = Communication::CommFactory::createCanBus("can0", 1000000);

        hand.setCanTxCallback([bus](uint32_t can_id, const uint8_t *data, uintptr_t data_len) -> int32_t {
            static int tx_count = 0;
            tx_count++;
            /*
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
*/
            std::vector<uint8_t> data_vec(data, data + data_len);
            bus->send(data_vec, can_id);
            return 0;
        });

        hand.setCanRxCallback([bus](uint32_t* can_id_out, uint8_t* data_out, uint8_t* len_out) -> int32_t {
            auto frame = bus->recv();
            if (frame.can_id == 0 && frame.can_dlc == 0) {
                return -1;
            }
            *can_id_out = frame.can_id;
            *len_out = frame.can_dlc;
            memcpy(data_out, frame.data, frame.can_dlc);

            static int rx_count = 0;
            rx_count++;
            /*
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
            */
            return 0;
        });

        // for (size_t i = 0; i < 1; ++i) {
        //     std::cout << hand.getVersion() << std::endl;

        //     std::vector<uint8_t> torque = hand.getTorque();
        //     std::cout << "torque size:" << torque.size() << std::endl;
        //     for (size_t j = 0; j < torque.size(); j++) {
        //         std::cout << (int)torque[j] << " ";
        //     }
        //     std::cout << std::endl;

        //     std::vector<uint8_t> speed = hand.getSpeed();
        //     std::cout << "speed size:" << speed.size() << std::endl;
        //     for (size_t j = 0; j < speed.size(); j++) {
        //         std::cout << (int)speed[j] << " ";
        //     }
        //     std::cout << std::endl;

        //     std::vector<uint8_t> state = hand.getPosition();
        //     std::cout << "state size:" << state.size() << std::endl;
        //     for (size_t j = 0; j < state.size(); j++) {
        //         std::cout << (int)state[j] << " ";
        //     }
        //     std::cout << std::endl;

        //     std::vector<std::vector<std::vector<uint8_t>>> touch_mats = hand.getForce();
        //     // 手指名称表，顺序对应 touch_mats[0..4]
        //     const std::array<const char*, 5> finger_name = {
        //         "THUMB_TOUCH", "INDEX_TOUCH", "MIDDLE_TOUCH", "RING_TOUCH", "LITTLE_TOUCH"
        //     };
        //     for (size_t n = 0; n < touch_mats.size(); ++n) {
        //         std::cout << finger_name[n] << ":\n";
        //         for (const auto &row : touch_mats[n]) {
        //             for (uint8_t val : row)
        //                 std::cout << std::setw(2) << static_cast<int>(val) << ' ';
        //             std::cout << '\n';
        //         }
        //         std::cout << '\n';
        //     }

        //     // 握拳
        //     std::vector<uint8_t> fist_pose = {74, 0, 0, 0, 0, 176, 52, 120, 150, 201, 198, 0, 0, 0, 0, 254, 0, 0, 0, 0, 100, 0, 0, 0, 0};
        //     hand.setPosition(fist_pose);
        //     std::this_thread::sleep_for(std::chrono::seconds(1));

        //     // 张开
        //     std::vector<uint8_t> open_pose = {74, 254, 254, 254, 253, 176, 52, 120, 150, 201, 198, 0, 0, 0, 0, 254, 0, 0, 0, 0, 254, 255, 255, 255, 250};
        //     hand.setPosition(open_pose);
        //     std::this_thread::sleep_for(std::chrono::seconds(1));
        // }
        
        
        std::cout << hand.getVersion() << std::endl;

        // 张开
        std::vector<uint8_t> open_pose = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255, 255, 255, 255};
        hand.setPosition(open_pose);
        
        // std::vector<double> op_arc = {0.0, 0.0, 0.0, 0.0, 0.0, -0.5, -0.18, 0.03, 0.11, 0.18, 0.15, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        // hand.setPositionArc(op_arc);
        std::this_thread::sleep_for(std::chrono::seconds(1));


        std::vector<uint8_t> state1 = hand.getPosition();
        std::cout << "state1 size:" << state1.size() << std::endl;
        for (size_t j = 0; j < state1.size(); j++) {
            std::cout << (int)state1[j] << " ";
        }
        std::cout << std::endl;
        
        
        
        
        
        std::vector<double> state2 = hand.testPositionArc(open_pose);
        std::cout << "state1 size:" << state2.size() << std::endl;
        for (size_t j = 0; j < state2.size(); j++) {
            std::cout << (float)state2[j] << " ";
        }
        std::cout << std::endl;
        
        

        std::vector<double> state = hand.getPositionArc();
        std::cout << "state size:" << state.size() << std::endl;
        for (size_t j = 0; j < state.size(); j++) {
            std::cout << (float)state[j] << " ";
        }
        std::cout << std::endl;


        // std::vector<double> fist_pose = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        // hand.setPositionArc(fist_pose);
        // std::this_thread::sleep_for(std::chrono::seconds(1));


    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
