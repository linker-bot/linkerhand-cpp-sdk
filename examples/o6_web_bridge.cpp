// o6_web_bridge — stdin->CAN bridge for the O6 hand, with position readback.
// Reads commands on stdin and drives the hand; periodically emits the hand's
// actual joint positions so the UI can show commanded-vs-actual.
//
// Commands (whitespace-separated, one per line):
//   P v0..v5   -> setPosition (6 bytes, each clamped 0..255)
//   S v0..v5   -> setSpeed
//   T v0..v5   -> setTorque
//   Q          -> quit
// Emits:
//   READY               once initialized
//   POS v0 v1 .. v5     ~6-7 Hz, the hand's measured joint positions
//
// A reader thread only queues stdin lines; ALL SDK calls happen on the main
// thread, so commands and the readback poll never race on the CAN bus.
// Based on examples/test_o6_can_0.cpp.
#include <atomic>
#include <chrono>
#include <cstring>
#include <deque>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "LinkerHandApi.h"
#include "CommFactory.h"

static std::mutex g_qmtx;
static std::deque<std::string> g_queue;
static std::atomic<bool> g_running{true};

int main(int argc, char** argv) {
    HAND_TYPE side = HAND_TYPE::LEFT;
    if (argc > 1 && std::string(argv[1]) == "right") side = HAND_TYPE::RIGHT;

    try {
        auto hand = std::make_shared<LinkerHandApi>(LINKER_HAND::O6, side);
        std::shared_ptr<Communication::ICanBus> bus = Communication::CommFactory::createCanBus(side);

        hand->setCanTxCallback([bus](uint32_t can_id, const uint8_t* data, uintptr_t len) -> int32_t {
            std::vector<uint8_t> v(data, data + len);
            bus->send(v, can_id);
            return 0;
        });
        hand->setCanRxCallback([bus](uint32_t* id_out, uint8_t* data_out, uint8_t* len_out) -> int32_t {
            auto f = bus->recv();
            if (f.can_id == 0 && f.can_dlc == 0) return -1;
            *id_out = f.can_id;
            *len_out = f.can_dlc;
            memcpy(data_out, f.data, f.can_dlc);
            return 0;
        });

        hand->setTorque({200, 200, 200, 200, 200, 200});
        hand->setSpeed({200, 200, 200, 200, 200, 200});

        std::cout << "READY" << std::endl;

        // Reader thread: queue stdin lines only. EOF -> stop.
        std::thread reader([] {
            std::string line;
            while (std::getline(std::cin, line)) {
                std::lock_guard<std::mutex> lk(g_qmtx);
                g_queue.push_back(line);
            }
            g_running = false;
        });
        reader.detach();

        auto apply = [&](const std::string& line) {
            std::istringstream ss(line);
            std::string cmd;
            if (!(ss >> cmd)) return;
            if (cmd == "Q" || cmd == "q") { g_running = false; return; }
            if (cmd != "P" && cmd != "S" && cmd != "T") return;
            std::vector<uint8_t> vals;
            int x;
            while (ss >> x) {
                if (x < 0) x = 0;
                if (x > 255) x = 255;
                vals.push_back(static_cast<uint8_t>(x));
            }
            if (vals.size() != 6) return;
            if (cmd == "P") hand->setPosition(vals);
            else if (cmd == "S") hand->setSpeed(vals);
            else hand->setTorque(vals);
        };

        // Main loop: 25 ms tick. Apply queued commands every tick (snappy);
        // poll readback every 6th tick (~6-7 Hz).
        int tick = 0;
        while (g_running) {
            std::deque<std::string> local;
            {
                std::lock_guard<std::mutex> lk(g_qmtx);
                local.swap(g_queue);
            }
            for (auto& ln : local) apply(ln);

            if (++tick % 6 == 0) {
                std::vector<uint8_t> pos = hand->getPosition();
                if (pos.size() >= 6) {
                    std::ostringstream out;
                    out << "POS";
                    for (int i = 0; i < 6; i++) out << ' ' << (int)pos[i];
                    std::cout << out.str() << std::endl;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
        }
    } catch (const std::exception& e) {
        std::cerr << "BRIDGE_ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
