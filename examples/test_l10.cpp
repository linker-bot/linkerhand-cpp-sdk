// L10 / CAN / 左手 —— 入门最小示例（版本、单次动作、状态）
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

        // 打印版本信息
        std::cout << "版本: " << hand.getVersion() << std::endl;

        // 执行一次握拳动作
        std::vector<uint8_t> fist_pose = {101, 60, 0, 0, 0, 0, 255, 255, 255, 51};
        hand.fingerMove(fist_pose);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 读取并打印当前状态
        std::vector<uint8_t> state = hand.getState();
        std::cout << "state size: " << state.size() << std::endl;
        for (size_t i = 0; i < state.size(); ++i) {
            std::cout << (int)state[i] << " ";
        }
        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
