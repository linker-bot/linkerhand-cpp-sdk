#pragma once

#include "communication/IModbus.h"
#include <array>
#include <cstring>
#include <memory>
#include <stdexcept>

namespace linkerhand::legacy {
// Preserve raw RTU callbacks. This does not turn split TX/RX calls into an
// atomic transaction: callers must serialize complete SDK operations per line.
class ModbusCallbacks {
public:
    explicit ModbusCallbacks(std::shared_ptr<communication::IModbus> bus)
        : bus_(std::move(bus)) {
        if (!bus_) throw std::invalid_argument("Modbus bus is required");
    }
    int32_t transmit(const uint8_t* data, uintptr_t size) noexcept {
        try {
            if (!data || size == 0 || size > 256) return -1;
            return bus_->sendRawFrame(data, size) ? 0 : -1;
        } catch (...) { return -1; }
    }
    int32_t receive(uint8_t slave, uint16_t* address, uint8_t* data, uint8_t* size) noexcept {
        try {
            if (!data || !size) return -1;
            std::array<uint8_t, 256> frame{};
            const int count = bus_->receiveCompleteFrame(frame.data(), frame.size(), 500);
            // The old length output cannot represent a 256-byte ADU. Validate
            // before copying into the SDK buffer or narrowing the length.
            if (count <= 0 || count > 255 || frame[0] != slave) return -1;
            std::memcpy(data, frame.data(), static_cast<size_t>(count));
            *size = static_cast<uint8_t>(count);
            if (address) *address = 0;
            return 0;
        } catch (...) { return -1; }
    }
private:
    std::shared_ptr<communication::IModbus> bus_;
};

template <class Hand>
std::shared_ptr<ModbusCallbacks> bind_modbus_callbacks(
    Hand& hand, std::shared_ptr<communication::IModbus> bus) {
    auto callbacks = std::make_shared<ModbusCallbacks>(std::move(bus));
    hand.setModbusTxCallback([callbacks](uint8_t, uint16_t, const uint8_t* data, uintptr_t size) {
        return callbacks->transmit(data, size);
    });
    hand.setModbusRxCallback([callbacks](uint8_t slave, uint16_t* address, uint8_t* data, uint8_t* size) {
        return callbacks->receive(slave, address, data, size);
    });
    return callbacks;
}
} // namespace linkerhand::legacy
