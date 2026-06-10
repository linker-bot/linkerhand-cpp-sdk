// L10 / CAN / 左手 —— 握拳与张开最小冒烟示例
#include "_win_console_utf8.h"
#include "LinkerHandApi.h"
#include "CommFactory.h"

int main() {
    try {
        // 初始化灵巧手
        LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::LEFT, COMM_TYPE::CAN);

        // 创建 CAN 总线对象
        std::shared_ptr<Communication::ICanBus> bus = Communication::CommFactory::createCanBus("can0", 1000000);

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
        std::cout << "-------------------------------------------" << std::endl;

        std::cout << "获取版本信息：" << std::endl;
        std::cout << hand.getVersion() << std::endl;
        std::cout << "-------------------------------------------" << std::endl;

        std::cout << "执行动作：握拳" << std::endl;
        std::vector<uint8_t> fist_pose = {120, 60, 0, 0, 0, 0, 255, 255, 255, 51};
        hand.fingerMove(fist_pose);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "-------------------------------------------" << std::endl;

        std::cout << "执行动作：张开" << std::endl;
        std::vector<uint8_t> open_pose = {255, 104, 255, 255, 255, 255, 255, 255, 255, 71};
        hand.fingerMove(open_pose);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "-------------------------------------------" << std::endl;

        std::cout << "程序结束！" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
