// O20 / CAN-FD / 双手并存示例
// -----------------------------------------------------------------------------
// 关键约定（详见 docs/FAQ.md Q11 / Q12）：
//   1. 一根 CAN-FD 适配器对应一只手：右手用 dev_num=0，左手用 dev_num=1
//      （编号取决于插入顺序：先插的是 0，后插的是 1）。
//   2. Communication::CanFD 是纯传输层，不知道左右手；左右手由
//      LinkerHandApi 构造函数的 HAND_TYPE 决定，O20Hand 内部把它编码为
//      device_id（0x01=右手 / 0x02=左手）写进 29-bit 扩展帧的高位。
//   3. tx/rx lambda 必须闭包捕获**各自**的 CanFD 实例，两只手绝不共用。
//   4. HAND_TYPE 传错也没关系：O20Hand 会在 setCanRxCallback 时主动探测
//      固件自报手型，与传入不一致时静默校正 device_id 并在 stdout 打印
//      "[LinkerHand O20] ⚠ 手型自动校正: ..." 提示。
// -----------------------------------------------------------------------------
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include "_win_console_utf8.h"
#include "LinkerHandApi.h"
#include "CommunicationCallbacks.h"
#include "CanFD.h"

namespace {

struct HandRuntime {
    Communication::CanFD canfd;
    std::unique_ptr<LinkerHandApi> api;
    const char* label;

    HandRuntime(uint32_t dev_num, const char* lbl)
        : canfd(dev_num, 0), label(lbl) {}
};

bool bringUp(HandRuntime& hand, HAND_TYPE side) {
    if (!hand.canfd.init()) {
        std::cerr << "[" << hand.label << "] Failed to initialize CANFD" << std::endl;
        return false;
    }
    std::cout << "[" << hand.label << "] CANFD initialized" << std::endl;

    hand.api.reset(new LinkerHandApi(LINKER_HAND::O20, side, COMM_TYPE::CAN));

    auto tx = [&hand](uint32_t can_id, const uint8_t* data, uintptr_t data_len) -> int32_t {
        std::vector<uint8_t> data_vec(data, data + data_len);
        hand.canfd.send(data_vec, can_id, true);
        return 0;
    };
    auto rx = [&hand](uint32_t* can_id_out, uint8_t* data_out, uint8_t* data_len_out) -> int32_t {
        Communication::CanFDFrame frame = hand.canfd.recv(10);
        if (frame.valid) {
            *can_id_out = frame.can_id;
            *data_len_out = Communication::CanFD::dlcToLen(frame.can_dlc);
            memcpy(data_out, frame.data, *data_len_out);
            return 0;
        }
        return -1;
    };

    hand.api->setCanTxCallback(tx);
    hand.api->setCanRxCallback(rx);
    return true;
}

} // namespace

int main() {
    std::cout << "O20 CANFD Double-Hand Test" << std::endl;

    HandRuntime right(0, "RIGHT");
    HandRuntime left (1, "LEFT ");

    if (!bringUp(right, HAND_TYPE::RIGHT)) {
        return 1;
    }
    if (!bringUp(left, HAND_TYPE::LEFT)) {
        right.canfd.close();
        return 1;
    }

    // 探测 + 常规读请求已在 setCanRxCallback 里发起，稍等一下让响应就绪
    std::this_thread::sleep_for(std::chrono::seconds(1));

    try {
        std::cout << "[RIGHT] Version: " << right.api->getVersion() << std::endl;
        std::cout << "[LEFT ] Version: " << left.api->getVersion() << std::endl;

        std::vector<uint8_t> pos(34, 0);
        right.api->setPosition(pos);
        left.api->setPosition(pos);

        std::this_thread::sleep_for(std::chrono::seconds(1));

        auto dump_speed = [](const char* lbl, const std::vector<uint8_t>& v) {
            std::cout << "[" << lbl << "] Speed:";
            for (auto b : v) std::cout << ' ' << (int)b;
            std::cout << std::endl;
        };
        dump_speed("RIGHT", right.api->getSpeed());
        dump_speed("LEFT ", left.api->getSpeed());
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    right.canfd.close();
    left.canfd.close();
    std::cout << "Test completed" << std::endl;
    return 0;
}
