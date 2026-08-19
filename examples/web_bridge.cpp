// web_bridge — 全型号 stdin->CAN 桥，供无依赖 Python web 前端（webui/run.py）驱动。
// 进程内构造 LinkerHandApi，按型号选普通 CAN 或 O20 CAN-FD 接线，从 stdin 读文本命令下发
// 姿势/速度/力矩，周期性把实测位置、速度、力矩、触觉压感（含掌心）以文本/JSON 行写到 stdout。
//
// 用法：  web_bridge <MODEL> <side> [comm] [channel]
//   MODEL    L6/L7/L10/L20/L21/L25/G20/O6/O20
//   side     left|right
//   comm     可选，缺省 can。can | canfd | modbus。
//            modbus 仅支持 O6/L7/L10；O20 请用 canfd。
//   channel  可选。can：CAN 接口名（如 can0），留空按 side 自动探测。
//            canfd（O20）：留空用厂商 CAN-FD 设备(0,0)；"socketcan:can0" 用内核原生 CAN-FD。
//            modbus：串口路径（如 /dev/ttyUSB0），留空按 side 自动探测。
//
// stdin 命令（空白分隔，每行一条）：
//   P v0..vN   -> setPosition   （N 个 0..255，长度须 = DOF）
//   S v0..vN   -> setSpeed
//   T v0..vN   -> setTorque
//   R ch hz    -> 设置某通道回读频率（ch=pos|st|force|temp|fault，hz=1~100）
//   Q          -> 退出
// stdout 输出：
//   READY                       初始化完成
//   META {json}                 一次：{"model","dof","version"}
//   POS  v0..vN                 关节实测位置，~10Hz
//   SPD  v0..vN / TRQ v0..vN    当前速度/力矩回读，低频
//   FORCE {"fingers":[[[..]]]}  触觉矩阵（维度按型号自适应），有传感器时持续 ~5Hz（全 0 也发）
//   PALM  {"palm":[[..]]}       掌心矩阵（O6/G20），有传感器时持续 ~5Hz（全 0 也发）
//   TEMP  v0..vN                逐关节温度(°C)，~1Hz，非空才发
//   FAULT v0..vN                逐关节故障码，~1Hz，非空才发
//
// 读线程只把 stdin 整行入队；所有 SDK/CAN 调用都在主线程做，命令下发与回读轮询不会
// 在总线上竞争。基于 examples/o6_web_bridge.cpp（参考仓库）与 test_o6_can_0.cpp。
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
#include "Modbus.h"
#if defined(WEB_BRIDGE_HAS_CANFD)
#include "CanFD.h"
#endif
#if defined(__linux__)
#include "CanFDSocket.h"
#endif

static std::mutex g_qmtx;
static std::deque<std::string> g_queue;
static std::atomic<bool> g_running{true};

// 型号字符串 -> 枚举（与 hand_teach_pendant/src/main.cpp 的 -t 解析保持一致）。
static bool parse_model(const std::string& s, LINKER_HAND& out) {
    if (s == "L6")  out = LINKER_HAND::L6;
    else if (s == "L7")  out = LINKER_HAND::L7;
    else if (s == "L10") out = LINKER_HAND::L10;
    else if (s == "L20") out = LINKER_HAND::L20;
    else if (s == "L21") out = LINKER_HAND::L21;
    else if (s == "L25") out = LINKER_HAND::L25;
    else if (s == "G20") out = LINKER_HAND::G20;
    else if (s == "O6")  out = LINKER_HAND::O6;
    else if (s == "O20") out = LINKER_HAND::O20;
    else return false;
    return true;
}

// 型号 -> 控制自由度（复刻 hand_teach_pendant/src/HandController.cpp:get_dof）。
// 前端按此渲染滑块；回读长度以 getPosition().size() 为准（O20 回读 17 > DOF 16）。
static int dof_of(LINKER_HAND m) {
    switch (m) {
        case L6:  case O6:  return 6;
        case L7:            return 7;
        case L10:           return 10;
        case L20:           return 20;
        case L21: case L25: return 25;
        case G20:           return 16;
        case O20:           return 16;
        default:            return 10;
    }
}

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
        std::cerr << "usage: web_bridge <MODEL> <left|right> [channel]" << std::endl;
        return 2;
    }
    LINKER_HAND model;
    if (!parse_model(argv[1], model)) {
        std::cerr << "BRIDGE_ERROR: unknown model '" << argv[1] << "'" << std::endl;
        return 2;
    }
    const std::string model_str = argv[1];
    HAND_TYPE side = (std::string(argv[2]) == "right") ? HAND_TYPE::RIGHT : HAND_TYPE::LEFT;
    const std::string comm = (argc > 3) ? argv[3] : "can";
    const std::string channel = (argc > 4) ? argv[4] : "";
    const int dof = dof_of(model);

    // canfd 是 CAN 之上的传输变体（仅 O20 有意义），SDK 仍以 COMM_TYPE::CAN 构造。
    const bool is_modbus = (comm == "modbus");
    const bool is_canfd  = (comm == "canfd") || (model == LINKER_HAND::O20 && comm != "modbus");

    try {
        COMM_TYPE ct = is_modbus ? COMM_TYPE::MODBUS : COMM_TYPE::CAN;
        auto hand = std::make_shared<LinkerHandApi>(model, side, ct);

        if (is_modbus) {
            // Modbus 仅 O6/L7/L10 有对应 hand 实现（HandFactory 其余会抛异常）。
            if (model != LINKER_HAND::O6 && model != LINKER_HAND::L7 && model != LINKER_HAND::L10) {
                std::cerr << "BRIDGE_ERROR: Modbus 仅支持 O6/L7/L10（当前 " << model_str << "）" << std::endl;
                return 1;
            }
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
        } else if (is_canfd) {
            // O20 走 CAN-FD。默认厂商 libcanbus；channel="socketcan:can0" 时用内核原生。
            bool use_socketcan = channel.rfind("socketcan:", 0) == 0;
            if (use_socketcan) {
#if defined(__linux__)
                std::string iface = channel.substr(std::string("socketcan:").size());
                if (iface.empty()) iface = "can0";
                auto cf = std::make_shared<Communication::CanFDSocket>(iface);
                if (!cf->init()) { std::cerr << "BRIDGE_ERROR: CanFDSocket init failed (" << iface << ")" << std::endl; return 1; }
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

        // 保守初始化：中等速度/力矩，长度 = DOF。
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
                      << ",\"fault\":" << hz_of(fault_ms) << "}"
                      << "}" << std::endl;
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
                    std::vector<uint8_t> pos = hand->getPosition();
                    if (!pos.empty()) { std::ostringstream o; o << "POS"; for (auto v : pos) o << ' ' << (int)v; std::cout << o.str() << std::endl; }
                }
                if (now >= next_st) {                      // 速度/力矩回读（周期可调）
                    next_st = now + ms(st_ms);
                    std::vector<uint8_t> sp = hand->getSpeed();
                    std::vector<uint8_t> tq = hand->getTorque();
                    if (!sp.empty()) { std::ostringstream o; o << "SPD"; for (auto v : sp) o << ' ' << (int)v; std::cout << o.str() << std::endl; }
                    if (!tq.empty()) { std::ostringstream o; o << "TRQ"; for (auto v : tq) o << ' ' << (int)v; std::cout << o.str() << std::endl; }
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
