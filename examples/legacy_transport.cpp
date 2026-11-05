// Read-only L7 version example using the shared callback adapter.
#include "LinkerHandApi.h"
#include "legacy/CanCallbacks.h"
#ifdef __linux__
#include <linkerbot/can/socketcan_transport.hpp>
#else
#include <linkerbot/can/pcan_transport.hpp>
#endif

int main(int argc, char** argv) try {
    if (argc != 2) {
        std::cerr << "usage: legacy_transport <can0|PCAN_USBBUS1>\n";
        return 2;
    }
    std::unique_ptr<linkerbot::can::CANTransport> transport;
#ifdef __linux__
    transport = std::make_unique<linkerbot::can::SocketCANTransport>(argv[1]);
#else
    linkerbot::can::PcanConfig config;
    config.channel = linkerbot::can::pcan_channel_handle(argv[1]);
    transport = std::make_unique<linkerbot::can::PcanTransport>(config);
#endif
    auto dispatcher = std::make_shared<linkerbot::can::CANDispatcher>(std::move(transport));
    auto callbacks = std::make_shared<linkerhand::legacy::CanCallbacks>(dispatcher, false, false);
    {
        LinkerHandApi hand(L7, LEFT, COMM_TYPE::CAN);
        hand.setCanTxCallback([callbacks](uint32_t id, const uint8_t* data, uintptr_t size) {
            return callbacks->transmit(id, data, size);
        });
        hand.setCanRxCallback([callbacks](uint32_t* id, uint8_t* data, uint8_t* size) {
            return callbacks->receive(id, data, size);
        });
        std::cout << hand.getVersion() << '\n';
    }
    callbacks->close();
    dispatcher->stop();
    return 0;
} catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
}
