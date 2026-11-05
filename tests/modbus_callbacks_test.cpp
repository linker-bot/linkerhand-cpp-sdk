#include "legacy/ModbusCallbacks.h"
#include <iostream>
#include <vector>

class RawBus : public linkerhand::communication::IModbus {
public:
    std::vector<uint8_t> reply;
    bool fail = false;
    bool isOpen() const override { return true; }
    void close() override {}
    bool sendRawFrame(const uint8_t*, size_t) override {
        if (fail) throw std::runtime_error("write failed");
        return true;
    }
    int receiveCompleteFrame(uint8_t* buffer, size_t capacity, int) override {
        if (fail) throw std::runtime_error("read failed");
        if (reply.size() > capacity) return -1;
        std::memcpy(buffer, reply.data(), reply.size());
        return static_cast<int>(reply.size());
    }
    int transact(const uint8_t*, size_t, uint8_t*, size_t, int) override {
        throw std::runtime_error("split callback must not create another transaction");
    }
};
static void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
int main() try {
    auto bus = std::make_shared<RawBus>();
    linkerhand::legacy::ModbusCallbacks callbacks(bus);
    uint8_t output[256]{};
    uint8_t size = 77;
    uint16_t address = 123;
    bus->reply = {1, 3, 2, 0, 42, 0, 0};
    require(callbacks.receive(1, &address, output, &size) == 0 && size == 7 && output[4] == 42 && address == 0,
            "raw response changed");
    output[0] = 99;
    size = 77;
    bus->reply.assign(256, 1);
    require(callbacks.receive(1, nullptr, output, &size) == -1 && output[0] == 99 && size == 77,
            "oversize response copied or length narrowed");
    bus->reply.assign(255, 1);
    require(callbacks.receive(1, nullptr, output, &size) == 0 && size == 255,
            "maximum representable response rejected");
    bus->reply = {2, 3, 0, 0};
    require(callbacks.receive(1, nullptr, output, &size) == -1, "wrong slave accepted");
    require(callbacks.transmit(output, 257) == -1, "oversize request accepted");
    bus->fail = true;
    require(callbacks.transmit(output, 4) == -1 && callbacks.receive(1, nullptr, output, &size) == -1,
            "I/O exception escaped callback boundary");
    return 0;
} catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
}
