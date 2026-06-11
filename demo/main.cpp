#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

#include "LinkerHandApi.h"
#include "CommFactory.h"

int main() {
    constexpr LINKER_HAND model = LINKER_HAND::O6;
    constexpr HAND_TYPE  side   = HAND_TYPE::LEFT;

    auto hand = std::make_shared<LinkerHandApi>(model, side, COMM_TYPE::CAN);
    auto bus  = std::shared_ptr<Communication::ICanBus>(Communication::CommFactory::createCanBus(side));

    hand->setCanTxCallback([bus](uint32_t can_id, const uint8_t *data, uintptr_t len) -> int32_t {
        std::vector<uint8_t> buf(data, data + len);
        bus->send(buf, can_id);
        return 0;
    });

    hand->setCanRxCallback([bus](uint32_t *id_out, uint8_t *data_out, uint8_t *len_out) -> int32_t {
        auto frame = bus->recv();
        if (frame.can_id == 0 && frame.can_dlc == 0) return -1;
        *id_out  = frame.can_id;
        *len_out = frame.can_dlc;
        std::memcpy(data_out, frame.data, frame.can_dlc);
        return 0;
    });

    std::cout << "-----------------------------------" << std::endl;
    std::cout << "version: " << hand->getVersion() << '\n';
    std::cout << "-----------------------------------" << std::endl;
    return 0;
}
