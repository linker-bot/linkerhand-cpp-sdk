// Legacy device stdin/stdout bridge for webui/run.py.
// Usage: web_bridge <MODEL> <left|right> [can|canfd|modbus] [channel]
// P/S/T accept model-sized 0-255 vectors. R selects telemetry frequency; Q exits.
// O20 uses extended CAN FD; L7 is the only maintained Modbus path.
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
#include "LegacyModels.h"
#include "CommFactory.h"
#include "Modbus.h"
#ifdef LINKERHAND_TRANSPORT_ADAPTER
#include "legacy/CanCallbacks.h"
#include <linkerbot/can/vendor_canfd_transport.hpp>
#ifdef __linux__
#include <linkerbot/can/socketcan_transport.hpp>
#else
#include <linkerbot/can/pcan_transport.hpp>
#endif
#endif
#if defined(WEB_BRIDGE_HAS_CANFD)
#include "CanFD.h"
#endif
#if defined(__linux__)
#include "CanFDSocket.h"
#endif

static std::mutex g_qmtx;
static std::deque<std::string> g_queue;
static std::atomic<bool> g_running{true};

// —— 极简 JSON 序列化（避免任何第三方 JSON 依赖）——
static std::string json_row(const std::vector<uint8_t>& v) {
    std::ostringstream o;
    o << '[';
    for (size_t i = 0; i < v.size(); ++i) { if (i) o << ','; o << (int)v[i]; }
    o << ']';
    return o.str();
}
static std::string json_mat(const std::vector<std::vector<uint8_t>>& m) {
    std::ostringstream o;
    o << '[';
    for (size_t i = 0; i < m.size(); ++i) { if (i) o << ','; o << json_row(m[i]); }
    o << ']';
    return o.str();
}
static std::string json_cube(const std::vector<std::vector<std::vector<uint8_t>>>& c) {
    std::ostringstream o;
    o << '[';
    for (size_t i = 0; i < c.size(); ++i) { if (i) o << ','; o << json_mat(c[i]); }
    o << ']';
    return o.str();
}

// 是否含单元格结构（有传感器即持续上报，哪怕全 0；无传感器时 getForce 返回空则不发）。
static bool cube_has_cells(const std::vector<std::vector<std::vector<uint8_t>>>& c) {
    for (auto& m : c) for (auto& r : m) if (!r.empty()) return true;
    return false;
}
static bool mat_has_cells(const std::vector<std::vector<uint8_t>>& m) {
    for (auto& r : m) if (!r.empty()) return true;
    return false;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "usage: web_bridge <MODEL> <left|right> [can|canfd|modbus] [channel]" << std::endl;
        return 2;
    }
    const auto* info = linkerhand::legacy::find_model(argv[1]);
    if (!info) {
        std::cerr << "BRIDGE_ERROR: model is outside legacy maintenance scope; see docs/migration.md\n";
        return 2;
    }
    const auto model = info->model;
    const std::string model_str = info->name;
    if (std::string(argv[2]) != "left" && std::string(argv[2]) != "right") return 2;
    HAND_TYPE side = std::string(argv[2]) == "right" ? HAND_TYPE::RIGHT : HAND_TYPE::LEFT;
    const std::string comm = argc > 3 ? argv[3] : (info->fd ? "canfd" : "can");
    const std::string channel = argc > 4 ? argv[4] : "";
    const bool is_modbus = comm == "modbus";
    const bool is_canfd = comm == "canfd";
    if ((is_modbus && !info->modbus) ||
        (!is_modbus && comm != (info->fd ? "canfd" : "can"))) {
        std::cerr << "BRIDGE_ERROR: unsupported model/transport combination\n";
        return 2;
    }
    const int dof = info->position_size;

    try {
#ifdef LINKERHAND_TRANSPORT_ADAPTER
        std::shared_ptr<linkerbot::can::CANDispatcher> dispatcher;
        std::shared_ptr<linkerhand::legacy::CanCallbacks> callbacks;
#endif
        COMM_TYPE ct = is_modbus ? COMM_TYPE::MODBUS : COMM_TYPE::CAN;
        auto hand = std::make_shared<LinkerHandApi>(model, side, ct);

        if (is_modbus) {
            // channel 为串口路径；留空按 side 自动探测。
            auto mb = channel.empty()
                          ? std::make_shared<Modbus>(side, 115200)
                          : std::make_shared<Modbus>(channel, 115200);
            if (!mb->isOpen()) { std::cerr << "BRIDGE_ERROR: Modbus 串口打开失败 (" << (channel.empty() ? "auto" : channel) << ")" << std::endl; return 1; }
            hand->setModbusTxCallback([mb](uint8_t /*sid*/, uint16_t /*addr*/, const uint8_t* d, uintptr_t n) -> int32_t {
                try { return mb->sendRawFrame(d, n) ? 0 : -1; } catch (...) { return -1; }
            });
            hand->setModbusRxCallback([mb](uint8_t sid, uint16_t* addr_out, uint8_t* d_out, uint8_t* n_out) -> int32_t {
                try {
                    int len = mb->receiveCompleteFrame(d_out, 256, 500);
                    if (len <= 0 || d_out[0] != sid) return -1;
                    *n_out = static_cast<uint8_t>(len);
                    if (addr_out) *addr_out = 0;
                } catch (...) { return -1; }
                return 0;
            });
#ifdef LINKERHAND_TRANSPORT_ADAPTER
        } else {
            std::unique_ptr<linkerbot::can::CANTransport> backend;
            if (is_canfd && channel.rfind("socketcan:", 0) != 0) {
                if (!channel.empty() && channel != "0")
                    throw std::runtime_error("Vendor O20 bridge accepts empty channel or 0; use socketcan:can0 for SocketCAN");
                backend = std::make_unique<linkerbot::can::VendorCanFdTransport>();
            } else {
                if (channel.empty()) throw std::runtime_error("Transport adapter requires an explicit channel");
#ifdef __linux__
                const auto iface = is_canfd ? channel.substr(10) : channel;
                backend = std::make_unique<linkerbot::can::SocketCANTransport>(iface, is_canfd);
#else
                linkerbot::can::PcanConfig config;
                config.channel = linkerbot::can::pcan_channel_handle(channel);
                config.mode = is_canfd ? linkerbot::can::PcanMode::kFd : linkerbot::can::PcanMode::kClassic;
                backend = std::make_unique<linkerbot::can::PcanTransport>(config);
#endif
            }
            dispatcher = std::make_shared<linkerbot::can::CANDispatcher>(std::move(backend));
            callbacks = std::make_shared<linkerhand::legacy::CanCallbacks>(dispatcher, is_canfd, is_canfd);
            hand->setCanTxCallback([callbacks](uint32_t id, const uint8_t* data, uintptr_t size) {
                return callbacks->transmit(id, data, size);
            });
            hand->setCanRxCallback([callbacks](uint32_t* id, uint8_t* data, uint8_t* size) {
                return callbacks->receive(id, data, size);
            });
        }
#else
        } else if (is_canfd) {
            // O20 accepts an explicit SocketCAN FD interface; otherwise use vendor (0,0).
            const bool use_socketcan = channel.rfind("socketcan:", 0) == 0;
            if (use_socketcan) {
#if defined(__linux__)
                const std::string iface = channel.substr(std::string("socketcan:").size());
                if (iface.empty()) throw std::runtime_error("O20 requires an explicit SocketCAN interface");
                auto cf = std::make_shared<Communication::CanFDSocket>(iface);
                if (!cf->init()) throw std::runtime_error("CanFDSocket init failed");
                hand->setCanTxCallback([cf](uint32_t id, const uint8_t* d, uintptr_t n) -> int32_t {
                    try { std::vector<uint8_t> v(d, d + n); cf->send(v, id, true); } catch (...) { return -1; }
                    return 0;
                });
                hand->setCanRxCallback([cf](uint32_t* id_out, uint8_t* d_out, uint8_t* n_out) -> int32_t {
                    try {
                        Communication::CanFDFrame f = cf->recv(10);
                        if (!f.valid) return -1;
                        *id_out = f.can_id;
                        *n_out  = f.can_dlc;               // SocketCAN 变体：can_dlc 即实际字节长度
                        memcpy(d_out, f.data, f.can_dlc);
                    } catch (...) { return -1; }
                    return 0;
                });
#else
                std::cerr << "BRIDGE_ERROR: socketcan CAN-FD is Linux-only" << std::endl; return 1;
#endif
            } else {
#if defined(WEB_BRIDGE_HAS_CANFD)
                auto cf = std::make_shared<Communication::CanFD>(0, 0);
                if (!cf->init()) { std::cerr << "BRIDGE_ERROR: CanFD init failed" << std::endl; return 1; }
                hand->setCanTxCallback([cf](uint32_t id, const uint8_t* d, uintptr_t n) -> int32_t {
                    try { std::vector<uint8_t> v(d, d + n); cf->send(v, id, true); } catch (...) { return -1; }
                    return 0;
                });
                hand->setCanRxCallback([cf](uint32_t* id_out, uint8_t* d_out, uint8_t* n_out) -> int32_t {
                    try {
                        Communication::CanFDFrame f = cf->recv(10);
                        if (!f.valid) return -1;
                        *id_out = f.can_id;
                        *n_out  = Communication::CanFD::dlcToLen(f.can_dlc);  // 厂商变体：DLC 编码需转字节数
                        memcpy(d_out, f.data, *n_out);
                    } catch (...) { return -1; }
                    return 0;
                });
#else
                std::cerr << "BRIDGE_ERROR: O20 needs CAN-FD, but this build has no vendor CAN-FD. "
                             "Rebuild with USE_CANFD=ON, or pass channel 'socketcan:can0'." << std::endl;
                return 1;
#endif
            }
        } else {
            // 普通型号：SocketCAN / PCAN。channel 留空按 side 自动探测。
            std::shared_ptr<Communication::ICanBus> bus =
                channel.empty() ? Communication::CommFactory::createCanBus(side)
                                : Communication::CommFactory::createCanBus(channel, 1000000);
            hand->setCanTxCallback([bus](uint32_t id, const uint8_t* d, uintptr_t n) -> int32_t {
                try { std::vector<uint8_t> v(d, d + n); bus->send(v, id); } catch (...) { return -1; }
                return 0;
            });
            hand->setCanRxCallback([bus](uint32_t* id_out, uint8_t* d_out, uint8_t* n_out) -> int32_t {
                try {
                    auto f = bus->recv();
                    if (f.can_id == 0 && f.can_dlc == 0) return -1;
                    *id_out = f.can_id;
                    *n_out  = f.can_dlc;
                    memcpy(d_out, f.data, f.can_dlc);
                } catch (...) { return -1; }
                return 0;
            });
        }

#endif

        // Preserve legacy parameter initialization; this is not a read-only connection.
        hand->setSpeed(std::vector<uint8_t>(dof, 180));
        hand->setTorque(std::vector<uint8_t>(dof, 180));

        // 版本串各字段以换行分隔；压进单行 JSON 用 ';' 保留字段边界（前端按 ';' 切段），引号替空格。
        auto sanitize = [](std::string v) {
            for (char& c : v) { if (c == '"') c = ' '; else if (c == '\n' || c == '\r') c = ';'; }
            return v;
        };
        // 各回读通道周期(ms)，前端可经 stdin "R <chan> <hz>" 调整（1~100Hz）。
        // force 决定 getForce() 触发 B1~B5 的节拍，也就是 B5→下一轮 B1 的空档。
        int pos_ms = 100, st_ms = 200, force_ms = 33, temp_ms = 1000, fault_ms = 1000;

        auto hz_of = [](int period_ms) { return period_ms > 0 ? 1000 / period_ms : 0; };

        auto emit_meta = [&](const std::string& v) {
            std::cout << "META {\"model\":\"" << model_str << "\",\"dof\":" << dof
                      << ",\"version\":\"" << v << "\""
                      << ",\"rates\":{\"pos\":" << hz_of(pos_ms) << ",\"st\":" << hz_of(st_ms)
                      << ",\"force\":" << hz_of(force_ms) << ",\"temp\":" << hz_of(temp_ms)
                      << ",\"fault\":" << hz_of(fault_ms) << "}";
            std::cout << "}" << std::endl;
        };

        // getVersion 发请求后读缓存，版本回帧由 rx 线程异步填充；刚上电时手在预热，可能迟迟不到。
        // 启动先快速轮询 ~1.5s；仍拿不到也照常 READY，之后由主循环每 ~500ms 重试并补发 META。
        std::string version;
        for (int i = 0; i < 30 && version.empty(); ++i) {
            try { version = hand->getVersion(); } catch (...) {}
            if (version.empty()) std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        version = sanitize(version);
        bool meta_has_version = !version.empty();

        std::cout << "READY" << std::endl;
        emit_meta(version);

        // 读线程：只把 stdin 整行入队；EOF -> 停机。
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
            if (cmd == "R" || cmd == "r") {         // 回读频率(Hz)：R <pos|st|force|temp|fault> <hz>，限幅 1~100
                std::string chan; int hz = 0;
                if ((ss >> chan) && (ss >> hz)) {
                    if (hz < 1) hz = 1;
                    if (hz > 100) hz = 100;
                    const int p = 1000 / hz;
                    if      (chan == "pos")   pos_ms = p;
                    else if (chan == "st")    st_ms = p;
                    else if (chan == "force") force_ms = p;
                    else if (chan == "temp")  temp_ms = p;
                    else if (chan == "fault") fault_ms = p;
                }
                return;
            }
            if (cmd != "P" && cmd != "S" && cmd != "T") return;
            std::vector<uint8_t> vals;
            int x;
            while (ss >> x) {
                if (x < 0) x = 0;
                if (x > 255) x = 255;
                vals.push_back(static_cast<uint8_t>(x));
            }
            if ((int)vals.size() != dof) return;   // 长度不符直接丢弃
            try {
                if (cmd == "P") hand->setPosition(vals);
                else if (cmd == "S") hand->setSpeed(vals);
                else hand->setTorque(vals);
            } catch (const std::exception& e) {
                std::cerr << "BRIDGE_WARN: apply failed: " << e.what() << std::endl;
            }
        };

        // 主循环：5ms 轮询排空命令 + 时间戳驱动分频回读，各通道按各自周期上报。
        // 用时间戳而非 tick 计数，因 30Hz(33ms) 无法由固定 tick 整除得到。
        using clk = std::chrono::steady_clock;
        using ms = std::chrono::milliseconds;
        auto now = clk::now();
        auto next_pos = now, next_st = now, next_force = now, next_temp = now, next_fault = now;
        auto next_meta = now + ms(500);   // 版本未就绪时的补发节拍
        while (g_running) {
            std::deque<std::string> local;
            {
                std::lock_guard<std::mutex> lk(g_qmtx);
                local.swap(g_queue);
            }
            for (auto& ln : local) apply(ln);

            now = clk::now();
            // 刚上电时设备信息可能晚到；未就绪则周期重试 getVersion，拿到后补发 META
            // （Python 端 reader 会 meta.update，前端轮询时刷新设备信息，无需重启）。
            if (!meta_has_version && now >= next_meta) {
                next_meta = now + ms(500);
                std::string v;
                try { v = sanitize(hand->getVersion()); } catch (...) {}
                if (!v.empty()) { meta_has_version = true; emit_meta(v); }
            }
            try {
                if (now >= next_pos) {                     // 位置回读（周期可调）
                    next_pos = now + ms(pos_ms);
                    const auto pos = hand->getPosition();
                    if (!pos.empty()) { std::ostringstream o; o << "POS"; for (auto v : pos) o << ' ' << int(v); std::cout << o.str() << std::endl; }
                }
                if (now >= next_st) {
                    next_st = now + ms(st_ms);
                    const auto sp = hand->getSpeed();
                    const auto tq = hand->getTorque();
                    if (!sp.empty()) { std::ostringstream o; o << "SPD"; for (auto v : sp) o << ' ' << int(v); std::cout << o.str() << std::endl; }
                    if (!tq.empty()) { std::ostringstream o; o << "TRQ"; for (auto v : tq) o << ' ' << int(v); std::cout << o.str() << std::endl; }
                }

                if (now >= next_force) {                   // 触觉/掌心，周期由前端可调
                    next_force = now + ms(force_ms);
                    auto force = hand->getForce();
                    if (cube_has_cells(force))
                        std::cout << "FORCE {\"fingers\":" << json_cube(force) << "}" << std::endl;
                    auto palm = hand->getPalmForce();
                    if (mat_has_cells(palm))
                        std::cout << "PALM {\"palm\":" << json_mat(palm) << "}" << std::endl;
                }
                if (now >= next_temp) {                    // 温度回读（周期可调）
                    next_temp = now + ms(temp_ms);
                    std::vector<uint8_t> tp = hand->getTemperature();
                    if (!tp.empty()) { std::ostringstream o; o << "TEMP"; for (auto v : tp) o << ' ' << (int)v; std::cout << o.str() << std::endl; }
                }
                if (now >= next_fault) {                   // 故障码回读（周期可调）
                    next_fault = now + ms(fault_ms);
                    std::vector<uint8_t> fc = hand->getFaultCode();
                    if (!fc.empty()) { std::ostringstream o; o << "FAULT"; for (auto v : fc) o << ' ' << (int)v; std::cout << o.str() << std::endl; }
                }
            } catch (const std::exception& e) {
                std::cerr << "BRIDGE_WARN: readback failed: " << e.what() << std::endl;
            }

            std::this_thread::sleep_for(ms(5));
        }

    } catch (const std::exception& e) {
        std::cerr << "BRIDGE_ERROR: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "BRIDGE_ERROR: unknown exception" << std::endl;
        return 1;
    }
    return 0;
}
