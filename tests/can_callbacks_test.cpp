#include "legacy/CanCallbacks.h"
#include <linkerbot/can/fake_transport.hpp>
#include <future>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace std::chrono_literals;
static void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
int main() try {
    auto transport = std::make_unique<linkerbot::can::FakeTransport>();
    auto* fake = transport.get();
    auto dispatcher = std::make_shared<linkerbot::can::CANDispatcher>(std::move(transport));
    linkerhand::legacy::CanCallbacks first(dispatcher, false, false, 1);
    linkerhand::legacy::CanCallbacks second(dispatcher, false, false);
    linkerhand::legacy::CanCallbacks fd(dispatcher, true, true);
    uint8_t input[64]{};
    input[0] = 42;
    require(first.transmit(0x28, input, 8) == 0, "Classic transmit failed");
    require(first.transmit(0x28, input, 9) == -1, "oversize Classic accepted");
    require(first.transmit(0x800, input, 1) == -1, "invalid standard ID accepted");
    require(!first.last_error().empty(), "transmit error was discarded");
    require(fd.transmit(0x12345, input, 64) == 0, "FD transmit failed");
    const auto sent = fake->sent();
    require(sent.size() == 2 && sent[1].is_fd && sent[1].is_extended && !sent[1].is_brs,
            "FD flags changed");
    require(sent[1].dlc == 64, "FD byte count was encoded as wire DLC");
    linkerbot::can::Message frame;
    frame.id = 0x28;
    frame.dlc = 1;
    frame.data[0] = 42;
    fake->inject(frame);
    uint32_t id = 0;
    uint8_t output[64]{};
    uint8_t size = 0;
    require(second.receive(&id, output, &size, 1s) == 0 && id == 0x28 && output[0] == 42,
            "shared dispatcher did not deliver Classic frame");
    require(fd.receive(&id, output, &size, 0ms) == -1, "FD inbox accepted Classic frame");
    fake->inject(frame);
    // A receive on the unbounded peer is a barrier for dispatch of that frame.
    require(second.receive(&id, output, &size, 1s) == 0, "second Classic frame missing");
    require(first.overflows() == 1, "bounded inbox overflow not counted");
    require(first.receive(&id, output, &size, 0ms) == 0 && size == 1, "oldest frame lost");
    frame.is_fd = true;
    frame.is_extended = true;
    frame.id = 0x12345;
    frame.dlc = 64;
    frame.data[63] = 99;
    fake->inject(frame);
    require(fd.receive(&id, output, &size, 1s) == 0 && size == 64 && output[63] == 99,
            "FD receive length/content changed");
    first.close();
    require(first.transmit(0x28, input, 1) == -1, "closed adapter accepted transmit");
    require(second.transmit(0x28, input, 1) == 0, "closing peer stopped shared bus");
    auto blocked = std::async(std::launch::async, [&] { return second.receive(&id, output, &size, 5s); });
    second.close();
    require(blocked.wait_for(1s) == std::future_status::ready && blocked.get() == -1,
            "close did not wake receiver");
    fake->fail_writes("injected write error");
    require(fd.transmit(0x12345, input, 64) == -1 && !fd.last_error().empty(),
            "backend error escaped callback boundary");
    fd.close();
    dispatcher->stop();
    return 0;
} catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
}
