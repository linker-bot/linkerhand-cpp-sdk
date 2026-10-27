// 全型号 / CAN·CAN-FD·Modbus —— 自包 ANSI 全屏 TUI 功能巡检示例
// 无第三方库：原始终端模式(termios / Windows Console API) + ANSI 转义自绘分屏界面，
// 方向键+回车导航；左侧操作菜单，右侧状态面板实时刷新。
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <atomic>
#include <array>
#include <deque>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include "LinkerHandApi.h"
#include "Modbus.h"

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#include "PCANBus.h"
#include "CanFD.h"
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include "CanBus.h"
#include "CanFD.h"
#endif

// 定义ANSI转义序列常量
const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string WHITE = "\033[37m";
const std::string BOLD = "\033[1m";
const std::string REVERSE = "\033[7m";

#define HAND_SPEED 200
#define HAND_TORQUE 255

// 用于标记程序是否应该退出
std::atomic<bool> running(true);

COMM_TYPE channel;

// ===================== TUI 基础设施（自包，全部匿名 namespace） =====================
namespace tui {

struct Size { int rows; int cols; };

Size termSize()
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        int cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        if (rows > 0 && cols > 0) return {rows, cols};
    }
#else
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0 && ws.ws_col > 0)
        return {static_cast<int>(ws.ws_row), static_cast<int>(ws.ws_col)};
#endif
    return {24, 80};
}

// 进入原始模式 + 备用屏 + 隐藏光标；析构复位（任何 return/异常路径都还原终端）
class RawMode {
public:
    RawMode()
    {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        hIn_ = GetStdHandle(STD_INPUT_HANDLE);
        hOut_ = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleMode(hIn_, &origIn_);
        GetConsoleMode(hOut_, &origOut_);
        SetConsoleMode(hIn_, origIn_ & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
        // 老终端无 VT 支持时该调用失败，退化为纯文本（可接受）
        SetConsoleMode(hOut_, origOut_ | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#else
        tcgetattr(STDIN_FILENO, &orig_);
        struct termios raw = orig_;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
#endif
        std::cout << "\033[?1049h\033[?25l" << std::flush; // 备用屏 + 隐藏光标
    }

    ~RawMode()
    {
        std::cout << "\033[?25h\033[?1049l" << std::flush; // 显示光标 + 退出备用屏
#ifdef _WIN32
        SetConsoleMode(hIn_, origIn_);
        SetConsoleMode(hOut_, origOut_);
#else
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_);
#endif
    }

    RawMode(const RawMode&) = delete;
    RawMode& operator=(const RawMode&) = delete;

private:
#ifdef _WIN32
    HANDLE hIn_{}, hOut_{};
    DWORD origIn_{}, origOut_{};
#else
    struct termios orig_{};
#endif
};

enum class Key { None, Up, Down, Left, Right, Enter, Esc, Char };
struct KeyEvent { Key key; char ch; };

#ifndef _WIN32
static bool waitReadable(int ms)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    struct timeval tv{ms / 1000, (ms % 1000) * 1000};
    return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0;
}
#endif

KeyEvent readKey()
{
#ifdef _WIN32
    int c = _getch();
    if (c == '\r' || c == '\n') return {Key::Enter, 0};
    if (c == 27) return {Key::Esc, 0};
    if (c == 0 || c == 0xE0) {
        int c2 = _getch();
        switch (c2) {
        case 72: return {Key::Up, 0};
        case 80: return {Key::Down, 0};
        case 77: return {Key::Right, 0};
        case 75: return {Key::Left, 0};
        }
        return {Key::None, 0};
    }
    return {Key::Char, static_cast<char>(c)};
#else
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return {Key::None, 0};
    if (c == '\r' || c == '\n') return {Key::Enter, 0};
    if (c == 27) {
        if (!waitReadable(30)) return {Key::Esc, 0}; // 裸 ESC
        char c1;
        if (read(STDIN_FILENO, &c1, 1) != 1) return {Key::Esc, 0};
        if (c1 == '[') {
            char c2;
            if (read(STDIN_FILENO, &c2, 1) != 1) return {Key::Esc, 0};
            switch (c2) {
            case 'A': return {Key::Up, 0};
            case 'B': return {Key::Down, 0};
            case 'C': return {Key::Right, 0};
            case 'D': return {Key::Left, 0};
            }
        }
        return {Key::None, 0};
    }
    return {Key::Char, c};
#endif
}

// 非阻塞探测：流式任务中途用户按 ESC / q 请求中断
bool abortRequested()
{
#ifdef _WIN32
    if (_kbhit()) {
        int c = _getch();
        if (c == 0 || c == 0xE0) { _getch(); return false; }
        return c == 27 || c == 'q' || c == 'Q';
    }
    return false;
#else
    if (waitReadable(0)) {
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) return c == 27 || c == 'q' || c == 'Q';
    }
    return false;
#endif
}

// ---- 显示宽度：跳过 ANSI CSI 序列（0 宽）；CJK(3/4 字节 UTF-8) 记 2 宽 ----
int dispWidth(const std::string& s)
{
    int w = 0;
    size_t i = 0, n = s.size();
    while (i < n) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c == 0x1b) { // ESC [ ... 终止字母
            ++i;
            if (i < n && s[i] == '[') {
                ++i;
                while (i < n && !(s[i] >= '@' && s[i] <= '~')) ++i;
                if (i < n) ++i;
            }
            continue;
        }
        if (c < 0x80) { w += 1; i += 1; }
        else if ((c >> 5) == 0x6) { w += 1; i += 2; }
        else if ((c >> 4) == 0xe) { w += 2; i += 3; }
        else if ((c >> 3) == 0x1e) { w += 2; i += 4; }
        else { w += 1; i += 1; }
    }
    return w;
}

std::string clampWidth(const std::string& s, int maxw)
{
    int w = 0;
    size_t i = 0, n = s.size();
    std::string out;
    while (i < n) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c == 0x1b) {
            size_t j = i + 1;
            if (j < n && s[j] == '[') {
                ++j;
                while (j < n && !(s[j] >= '@' && s[j] <= '~')) ++j;
                if (j < n) ++j;
            }
            out.append(s, i, j - i);
            i = j;
            continue;
        }
        int cw;
        size_t len;
        if (c < 0x80) { cw = 1; len = 1; }
        else if ((c >> 5) == 0x6) { cw = 1; len = 2; }
        else if ((c >> 4) == 0xe) { cw = 2; len = 3; }
        else if ((c >> 3) == 0x1e) { cw = 2; len = 4; }
        else { cw = 1; len = 1; }
        if (w + cw > maxw) break;
        out.append(s, i, len);
        w += cw;
        i += len;
    }
    return out;
}

void clearScreen() { std::cout << "\033[2J\033[H"; }
void moveTo(int r, int c) { std::cout << "\033[" << r << ";" << c << "H"; }
void flush() { std::cout << std::flush; }

// 在 (r,c) 起绘制一段（不换行）
void drawAt(int r, int c, const std::string& s)
{
    moveTo(r, c);
    std::cout << s;
}

// 屏幕水平居中绘制一行（text 可含 ANSI 颜色）
void drawCentered(int r, int cols, const std::string& text)
{
    int w = dispWidth(text);
    int c = std::max(1, (cols - w) / 2 + 1);
    drawAt(r, c, text);
}

// 右侧实时状态面板：固定几何 + 行环形缓冲，log() 增量重绘该区域
class StatusPane {
public:
    void setGeometry(int row0, int col0, int width, int height)
    {
        row0_ = row0;
        col0_ = col0;
        width_ = std::max(1, width);
        height_ = std::max(1, height);
        while (static_cast<int>(lines_.size()) > height_) lines_.pop_front();
    }

    void clear()
    {
        lines_.clear();
        redraw();
    }

    // 支持多行字符串（按 '\n' 拆分）
    void log(const std::string& s)
    {
        std::stringstream ss(s);
        std::string line;
        bool any = false;
        while (std::getline(ss, line)) {
            pushOne(line);
            any = true;
        }
        if (!any) pushOne("");
        redraw();
    }

    void redraw() const
    {
        std::string blank(static_cast<size_t>(width_), ' ');
        for (int i = 0; i < height_; ++i) {
            int r = row0_ + i;
            if (i < static_cast<int>(lines_.size())) {
                std::string cell = clampWidth(lines_[i], width_);
                int pad = width_ - dispWidth(cell);
                if (pad > 0) cell.append(static_cast<size_t>(pad), ' ');
                drawAt(r, col0_, cell);
            } else {
                drawAt(r, col0_, blank);
            }
        }
        flush();
    }

private:
    void pushOne(const std::string& l)
    {
        lines_.push_back(l);
        while (static_cast<int>(lines_.size()) > height_) lines_.pop_front();
    }

    std::deque<std::string> lines_;
    int row0_ = 1, col0_ = 1, width_ = 40, height_ = 10;
};

// 全屏单选菜单：↑/↓ 移动，⏎ 选择返回索引，ESC/q 返回 -1；数字键快选
int runMenu(const std::string& title, const std::vector<std::string>& items, const std::string& footer)
{
    int sel = 0;
    while (running) {
        Size sz = termSize();
        clearScreen();
        int startRow = std::max(2, sz.rows / 2 - static_cast<int>(items.size()) / 2 - 2);
        drawCentered(startRow, sz.cols, BOLD + CYAN + title + RESET);
        for (size_t i = 0; i < items.size(); ++i) {
            int r = startRow + 2 + static_cast<int>(i);
            std::string label = "  " + items[i] + "  ";
            if (static_cast<int>(i) == sel)
                drawCentered(r, sz.cols, GREEN + REVERSE + label + RESET);
            else
                drawCentered(r, sz.cols, BLUE + label + RESET);
        }
        drawCentered(sz.rows - 1, sz.cols, YELLOW + footer + RESET);
        flush();

        KeyEvent k = readKey();
        switch (k.key) {
        case Key::Up:
            sel = (sel - 1 + static_cast<int>(items.size())) % static_cast<int>(items.size());
            break;
        case Key::Down:
            sel = (sel + 1) % static_cast<int>(items.size());
            break;
        case Key::Enter:
            return sel;
        case Key::Esc:
            return -1;
        case Key::Char:
            if (k.ch == 'q' || k.ch == 'Q') return -1;
            if (k.ch >= '1' && k.ch <= '9') {
                int idx = k.ch - '1';
                if (idx < static_cast<int>(items.size())) return idx;
            }
            break;
        default:
            break;
        }
    }
    return -1;
}

} // namespace tui

// ===================== 数据/辅助（保留原有） =====================

// 辅助函数：将字节向量转换为十六进制字符串
std::string bytesToHex(const std::vector<uint8_t>& bytes)
{
    std::string hexStr;
    for (uint8_t byte : bytes) hexStr += std::to_string(byte) + " ";
    return hexStr;
}

// 辅助函数：将二维字节向量转换为十六进制字符串
std::string bytesToHex(const std::vector<std::vector<uint8_t>>& bytes)
{
    std::string hexStr;
    for (const auto& vec : bytes) {
        for (uint8_t byte : vec) hexStr += std::to_string(byte) + " ";
        hexStr += "\n";
    }
    return hexStr;
}

// L6 - 执行动作 - 单指
std::vector<std::vector<uint8_t>> L6_POSE_1 = {
    {255, 255, 255, 255, 255, 255},
    {0, 255, 255, 255, 255, 255},
    {255, 0, 255, 255, 255, 255},
    {255, 255, 0, 255, 255, 255},
    {255, 255, 255, 0, 255, 255},
    {255, 255, 255, 255, 0, 255},
    {255, 255, 255, 255, 255, 0},
    {255, 255, 255, 255, 255, 255}};

// L7 - 执行动作 - 手掌握拳
std::vector<std::vector<uint8_t>> L7_POSE_1 = {
    {255, 255, 255, 255, 255, 255, 255},
    {255, 255, 0, 255, 255, 255, 255},
    {255, 255, 0, 0, 255, 255, 255},
    {255, 255, 0, 0, 0, 255, 255},
    {255, 255, 0, 0, 0, 0, 255},
    {72, 90, 0, 0, 0, 0, 55}};
// L7 - 执行动作 - 张开
std::vector<uint8_t> L7_POSE_OPEN = {255, 255, 255, 255, 255, 255, 255};

// L10 - 执行动作 - 手指循环弯曲
std::vector<std::vector<uint8_t>> L10_POSE_1 = {
    {35,140,255,255,255,255,255,255,255,30},    // 拇指弯曲
    {255,70,255,255,255,255,255,255,255,255},   // 手掌张开
    {255,70,0,255,255,255,255,255,255,255},     // 食指弯曲
    {255,70,255,255,255,255,255,255,255,255},   // 手掌张开
    {255,70,255,0,255,255,255,255,255,255},     // 中指弯曲
    {255,70,255,255,255,255,255,255,255,255},   // 手掌张开
    {255,70,255,255,0,255,255,255,255,255},     // 无名指弯曲
    {255,70,255,255,255,255,255,255,255,255},   // 手掌张开
    {255,70,255,255,255,0,255,255,255,255},     // 小拇指弯曲
    {255,70,255,255,255,255,255,255,255,255}    // 手掌张开
};

// L10 - 执行动作 - 拇指与其他手指循环对指
std::vector<std::vector<uint8_t>> L10_POSE_2 = {
    {255, 70, 255, 255, 255, 255, 255, 255, 255, 255},  // 手掌张开
    {145, 128, 146, 255, 255, 255, 255, 255, 255, 90},  // 拇指对食指
    {255, 70, 255, 255, 255, 255, 255, 255, 255, 255},  // 手掌张开
    {145, 88, 255, 148, 255, 255, 255, 255, 255, 75},   // 拇指对中指
    {255, 70, 255, 255, 255, 255, 255, 255, 255, 255},  // 手掌张开
    {145, 63, 255, 255, 150, 255, 255, 255, 255, 60},   // 拇指对无名指
    {255, 70, 255, 255, 255, 255, 255, 255, 255, 255},  // 手掌张开
    {147, 70, 255, 255, 255, 131, 255, 255, 120, 27},   // 拇指对小拇指
    {255, 70, 255, 255, 255, 255, 255, 255, 255, 255}   // 手掌张开
};

// L10 - 执行动作 - 手指侧摆
std::vector<std::vector<uint8_t>> L10_POSE_3 = {
    {255, 255, 255, 255, 255, 255, 40, 88, 80, 63},     // 手掌张开
    {255, 0, 255, 255, 255, 255, 40, 88, 80, 63},       // 拇指侧摆
    {255, 70, 255, 255, 255, 255, 40, 88, 80, 0},       // 手掌张开
    {255, 255, 255, 255, 255, 255, 40, 88, 80, 255},    // 拇指旋转
    {255, 255, 255, 255, 255, 255, 40, 88, 80, 63},     // 手掌张开
    {255, 255, 255, 255, 255, 255, 255, 88, 80, 63},    // 食指侧摆
    {255, 255, 255, 255, 255, 255, 40, 88, 80, 63},     // 手掌张开
    {255, 255, 255, 255, 255, 255, 40, 255, 80, 63},    // 无名指侧摆
    {255, 255, 255, 255, 255, 255, 40, 88, 80, 63},     // 手掌张开
    {255, 255, 255, 255, 255, 255, 40, 88, 255, 63},    // 小拇指侧摆
    {255, 255, 255, 255, 255, 255, 40, 88, 80, 63}      // 手掌张开
};

// L10 - 执行动作 - 张开
std::vector<uint8_t> L10_POSE_OPEN = {255, 104, 255, 255, 255, 255, 255, 255, 255, 71};
// L10 - 执行动作 - 握拳
std::vector<uint8_t> L10_POSE_CLOSE = {101, 60, 0, 0, 0, 0, 255, 255, 255, 51};

// ===================== 各功能任务（输出走 StatusPane） =====================

// 执行预设动作（按型号分派，逻辑保留原样，输出改写入面板）
static void runPresetAction(LinkerHandApi& hand, tui::StatusPane& pane)
{
    if (hand.handJoint_ == LINKER_HAND::L6) {
        pane.log("L6 - Execute action");
        for (size_t i = 0; i < 10 && !tui::abortRequested(); i++) {
            for (const auto& pose : L6_POSE_1) {
                hand.setPosition(pose);
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    } else if (hand.handJoint_ == LINKER_HAND::O6) {
        pane.log("O6 - Execute action");
        for (const auto& pose : L6_POSE_1) {
            if (tui::abortRequested()) break;
            hand.setPosition(pose);
            std::this_thread::sleep_for(std::chrono::milliseconds(1200));
        }
    } else if (hand.handJoint_ == LINKER_HAND::L7) {
        hand.setSpeed(std::vector<uint8_t>(7, HAND_SPEED));   // L7 need 7 speed
        hand.setTorque(std::vector<uint8_t>(7, HAND_TORQUE)); // L7 need 7 torque
        pane.log("L7 - Execute action - Make a fist");
        for (const auto& pose : L7_POSE_1) {
            hand.setPosition(pose);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        pane.log("L7 - Execute action - Open hand");
        hand.setPosition(L7_POSE_OPEN);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } else if (hand.handJoint_ == LINKER_HAND::L10) {
        hand.setSpeed(std::vector<uint8_t>(10, HAND_SPEED));
        hand.setTorque(std::vector<uint8_t>(10, HAND_TORQUE));
        pane.log("L10 - Execute action - Finger cycle bending");
        for (const auto& pose : L10_POSE_1) {
            if (tui::abortRequested()) break;
            hand.setPosition(pose);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        pane.log("L10 - Execute action - Thumb and Circular Fingers");
        for (const auto& pose : L10_POSE_2) {
            if (tui::abortRequested()) break;
            hand.setPosition(pose);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        pane.log("L10 - Execute action - Finger side swing");
        for (const auto& pose : L10_POSE_3) {
            if (tui::abortRequested()) break;
            hand.setPosition(pose);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        pane.log("L10 - Execute action - Make a fist");
        hand.setPosition(L10_POSE_CLOSE);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        pane.log("L10 - Execute action - Open hand");
        hand.setPosition(L10_POSE_OPEN);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } else if (hand.handJoint_ == LINKER_HAND::L20) {
        hand.setSpeed(std::vector<uint8_t>(5, HAND_SPEED)); // L20 need 5 speed
        pane.log("L20 - Execute action - Make a fist");
        std::vector<uint8_t> L20_POSE_CLOSE = {69, 0, 0, 0, 0, 32, 10, 100, 180, 240, 145, 255, 255, 255, 255, 0, 0, 0, 0, 0};
        hand.setPosition(L20_POSE_CLOSE);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        pane.log("L20 - Execute action - Open hand");
        std::vector<uint8_t> L20_POSE_OPEN = {255, 255, 230, 255, 255, 255, 10, 100, 180, 240, 245, 255, 255, 255, 255, 255, 255, 255, 255, 255};
        hand.setPosition(L20_POSE_OPEN);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } else if (hand.handJoint_ == LINKER_HAND::L25 || hand.handJoint_ == LINKER_HAND::L21) {
        hand.setSpeed(std::vector<uint8_t>(25, HAND_SPEED));
        hand.setTorque(std::vector<uint8_t>(25, HAND_TORQUE));

        // Open hand
        std::vector<uint8_t> pos1_1 = {230, 0, 0, 15, 5, 250, 55, 80, 210, 202, 85, 0, 0, 0, 0, 250, 0, 40, 35, 5, 250, 0, 5, 0, 0};
        std::vector<uint8_t> pos1_3 = {75, 255, 255, 255, 255, 176, 51, 80, 210, 202, 202, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
        // Make a fist
        std::vector<uint8_t> pos2_1 = {230, 0, 0, 15, 5, 250, 55, 80, 210, 202, 85, 0, 0, 0, 0, 80, 0, 40, 35, 5, 250, 0, 5, 0, 0};
        std::vector<uint8_t> pos2_2 = {230, 0, 0, 15, 5, 42, 55, 80, 210, 202, 85, 0, 0, 0, 0, 90, 0, 40, 35, 5, 120, 0, 5, 0, 0};

        pane.log("L25/L21 - Execute action - Make a fist");
        hand.setPosition(pos2_1);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        hand.setPosition(pos2_2);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        pane.log("L25/L21 - Execute action - Open hand");
        hand.setPosition(pos1_1);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        hand.setPosition(pos1_3);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } else if (hand.handJoint_ == LINKER_HAND::L20_10) {
        hand.setSpeed(std::vector<uint8_t>(5, HAND_SPEED)); // L20 need 5 speed
        pane.log("L20_10 - Execute action - Make a fist");
        std::vector<uint8_t> L20_10_POSE_CLOSE_1 = {255, 0, 0, 0, 0, 255, 255, 178, 84, 0, 255, 255, 0, 0, 0, 0};
        hand.setPosition(L20_10_POSE_CLOSE_1);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::vector<uint8_t> L20_10_POSE_CLOSE_2 = {117, 0, 0, 0, 0, 47, 255, 178, 84, 0, 115, 104, 0, 0, 0, 0};
        hand.setPosition(L20_10_POSE_CLOSE_2);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        pane.log("L20_10 - Execute action - Open hand");
        std::vector<uint8_t> L20_10_POSE_OPEN = {255, 255, 255, 255, 255, 255, 255, 172, 74, 0, 255, 255, 255, 255, 255, 255};
        hand.setPosition(L20_10_POSE_OPEN);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } else if (hand.handJoint_ == LINKER_HAND::O20) {
        pane.log("暂无动作！");
    }
}

// 把矩阵渲染成等宽文本行（每格 3 字符 + 空格）
static std::vector<std::string> renderMatrix(const std::vector<std::vector<uint8_t>>& m)
{
    std::vector<std::string> lines;
    for (const auto& row : m) {
        std::ostringstream s;
        for (uint8_t v : row) s << std::setw(3) << static_cast<int>(v) << ' ';
        lines.push_back(s.str());
    }
    return lines;
}

// 带标题的文本块
struct Block { std::string title; std::vector<std::string> lines; };

static int blockWidth(const Block& b)
{
    int w = tui::dispWidth(b.title);
    for (const auto& l : b.lines) w = std::max(w, tui::dispWidth(l));
    return w;
}

// 从 top 行起，把带标题的块从左到右铺排，超出宽度自动换行到下一带；返回下一空闲行。
// maxRow：超过屏幕底部的行直接裁掉，避免写穿备用屏引发滚动。
static int layoutBlocks(int top, int cols, int maxRow, const std::vector<Block>& blocks)
{
    const int gap = 2;
    int row = top;
    size_t idx = 0;
    while (idx < blocks.size()) {
        // 先算本带能放下哪些块
        std::vector<size_t> band;
        int c = 1;
        while (idx < blocks.size()) {
            int bw = blockWidth(blocks[idx]);
            int need = (c == 1) ? bw : (c - 1 + gap + bw);
            if (c != 1 && need > cols) break;
            band.push_back(idx);
            c = (c == 1) ? (1 + bw) : (c + gap + bw);
            ++idx;
        }
        // 绘制本带：标题行 + 矩阵行
        int bandRows = 0;
        c = 1;
        for (size_t bi : band) {
            const Block& b = blocks[bi];
            if (row <= maxRow) tui::drawAt(row, c, CYAN + b.title + RESET);
            for (size_t r = 0; r < b.lines.size(); ++r) {
                int rr = row + 1 + static_cast<int>(r);
                if (rr > maxRow) break;
                tui::drawAt(rr, c, b.lines[r]);
            }
            bandRows = std::max(bandRows, static_cast<int>(b.lines.size()));
            c += blockWidth(b) + gap;
        }
        row += bandRows + 2;
        if (row > maxRow) break;
    }
    return row;
}

// 压感全屏实时视图：5 指矩阵横向铺排 + 掌心，就地刷新，ESC/q 返回
static void runForceFullscreen(LinkerHandApi& hand)
{
    const std::array<const char*, 5> finger_name = {
        "THUMB", "INDEX", "MIDDLE", "RING", "LITTLE"};
    while (!tui::abortRequested()) {
        tui::Size sz = tui::termSize();
        tui::clearScreen();
        tui::drawAt(1, 1, BOLD + REVERSE + CYAN +
                    tui::clampWidth(" 压感实时视图   ESC/q 返回 ", sz.cols) + RESET);

        std::vector<std::vector<std::vector<uint8_t>>> touch_mats = hand.getForce();
        std::vector<Block> fingers;
        for (size_t n = 0; n < touch_mats.size(); ++n) {
            fingers.push_back({n < finger_name.size() ? finger_name[n] : "?",
                               renderMatrix(touch_mats[n])});
        }
        int nextRow = layoutBlocks(3, sz.cols, sz.rows, fingers);

        std::vector<std::vector<uint8_t>> palm_mat = hand.getPalmForce();
        if (!palm_mat.empty() && nextRow <= sz.rows) {
            layoutBlocks(nextRow, sz.cols, sz.rows, {{"PALM", renderMatrix(palm_mat)}});
        }
        tui::flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// 执行单个功能项（idx 对应菜单序号 0..9），返回 false 表示请求退出
static bool executeTask(int idx, LinkerHandApi& hand, tui::StatusPane& pane)
{
    switch (idx) {
    case 0:
        pane.log(GREEN + "▶ 版本信息" + RESET);
        pane.log(hand.getVersion());
        break;
    case 1:
        pane.log(GREEN + "▶ 温度" + RESET);
        pane.log(bytesToHex(hand.getTemperature()));
        break;
    case 2:
        pane.log(GREEN + "▶ 故障码" + RESET);
        pane.log(bytesToHex(hand.getFaultCode()));
        break;
    case 3:
        pane.log(GREEN + "▶ 力矩 (x10, ESC 中断)" + RESET);
        for (size_t i = 0; i < 10 && !tui::abortRequested(); i++) {
            pane.log(bytesToHex(hand.getTorque()));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        break;
    case 4:
        pane.log(GREEN + "▶ 速度" + RESET);
        pane.log(bytesToHex(hand.getSpeed()));
        break;
    case 5:
        runForceFullscreen(hand);
        pane.log(GREEN + "▶ 压感视图已退出" + RESET);
        break;
    case 6:
        pane.log(GREEN + "▶ 清除故障码" + RESET);
        hand.clearFaultCode();
        pane.log("done");
        break;
    case 7:
        pane.log(GREEN + "▶ 关节状态 (x10, ESC 中断)" + RESET);
        for (size_t i = 0; i < 10 && !tui::abortRequested(); i++) {
            pane.log(bytesToHex(hand.getPosition()));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        break;
    case 8:
        pane.log(GREEN + "▶ 预设动作 (ESC 中断)" + RESET);
        runPresetAction(hand, pane);
        break;
    case 9:
        return false; // 退出
    default:
        break;
    }
    return true;
}

// ===================== Dashboard（分屏主界面） =====================

static void runDashboard(LinkerHandApi& hand, const std::string& modelName,
                         const std::string& sideName, const std::string& busName)
{
    const std::vector<std::string> menu = {
        "1. 版本信息", "2. 温度", "3. 故障码", "4. 力矩 (x10)", "5. 速度",
        "6. 压感 (x60)", "7. 清除故障码", "8. 关节状态 (x10)", "9. 预设动作", "0. 退出"};

    tui::StatusPane pane;
    int sel = 0;

    while (running) {
        tui::Size sz = tui::termSize();
        const int leftW = 26;
        const int bodyTop = 3;
        const int bodyBot = std::max(bodyTop, sz.rows - 2);
        const int paneCol = leftW + 3;
        const int paneW = std::max(10, sz.cols - paneCol);
        const int paneH = bodyBot - bodyTop + 1;
        pane.setGeometry(bodyTop, paneCol, paneW, paneH);

        tui::clearScreen();

        // 标题栏
        std::string header = " LinkerHand 交互巡检   型号:" + modelName +
                             "   手别:" + sideName + "   总线:" + busName + " ";
        tui::drawAt(1, 1, BOLD + REVERSE + CYAN + tui::clampWidth(header, sz.cols) + RESET);

        // 左菜单
        for (size_t i = 0; i < menu.size(); ++i) {
            int r = bodyTop + static_cast<int>(i);
            if (r > bodyBot) break;
            std::string label = " " + menu[i];
            if (static_cast<int>(i) == sel)
                tui::drawAt(r, 1, GREEN + REVERSE + tui::clampWidth(label, leftW) + RESET);
            else
                tui::drawAt(r, 1, BLUE + tui::clampWidth(label, leftW) + RESET);
        }

        // 竖分隔线
        for (int r = bodyTop; r <= bodyBot; ++r)
            tui::drawAt(r, leftW + 1, YELLOW + std::string("│") + RESET);

        // 底部帮助栏
        std::string footer = " ↑/↓ 选择   ⏎ 执行   ESC/q 退出   (流式任务中 ESC 中断) ";
        tui::drawAt(sz.rows, 1, YELLOW + tui::clampWidth(footer, sz.cols) + RESET);

        // 右侧面板（保留历史输出）
        pane.redraw();
        tui::flush();

        tui::KeyEvent k = tui::readKey();
        switch (k.key) {
        case tui::Key::Up:
            sel = (sel - 1 + static_cast<int>(menu.size())) % static_cast<int>(menu.size());
            break;
        case tui::Key::Down:
            sel = (sel + 1) % static_cast<int>(menu.size());
            break;
        case tui::Key::Enter:
            if (!executeTask(sel, hand, pane)) running = false;
            break;
        case tui::Key::Esc:
            running = false;
            break;
        case tui::Key::Char:
            if (k.ch == 'q' || k.ch == 'Q') { running = false; }
            else if (k.ch == '0') { running = false; }
            else if (k.ch >= '1' && k.ch <= '9') {
                sel = k.ch - '1';
                if (!executeTask(sel, hand, pane)) running = false;
            }
            break;
        default:
            break;
        }
    }
}

int main(int argc, char* argv[])
{
    std::string errMsg;

    { // ---- TUI 生命周期：RawMode RAII 保证任何路径都还原终端 ----
        tui::RawMode guard;

        // 1) 选型号
        const std::vector<std::string> modelItems = {
            "L6", "L7", "L10", "L20", "L21", "L25", "O6", "L20_10", "O20"};
        const std::array<LINKER_HAND, 9> modelEnum = {
            LINKER_HAND::L6, LINKER_HAND::L7, LINKER_HAND::L10, LINKER_HAND::L20,
            LINKER_HAND::L21, LINKER_HAND::L25, LINKER_HAND::O6, LINKER_HAND::L20_10,
            LINKER_HAND::O20};
        int mi = tui::runMenu("选择 LinkerHand 型号", modelItems, "↑/↓ 选择   ⏎ 确认   ESC/q 退出");
        if (mi < 0) return 0;
        LINKER_HAND linkerhand = modelEnum[mi];
        std::string modelName = modelItems[mi];

        // 2) 选手别
        int si = tui::runMenu("选择手别", {"左手 (Left)", "右手 (Right)"}, "↑/↓ 选择   ⏎ 确认   ESC/q 退出");
        if (si < 0) return 0;
        HAND_TYPE handType = (si == 0) ? HAND_TYPE::LEFT : HAND_TYPE::RIGHT;
        std::string sideName = (si == 0) ? "Left" : "Right";

        // 3) 选总线（O20 走 CANFD，跳过）
        std::string busName;
        if (linkerhand == LINKER_HAND::O20) {
            channel = COMM_TYPE::CAN;
            busName = "CAN-FD";
        } else {
            int bi = tui::runMenu("选择通信总线", {"CAN", "Modbus", "EtherCAT"},
                                  "↑/↓ 选择   ⏎ 确认   ESC/q 退出");
            if (bi < 0) return 0;
            const std::array<COMM_TYPE, 3> busEnum = {COMM_TYPE::CAN, COMM_TYPE::MODBUS, COMM_TYPE::ETHERCAT};
            const std::array<const char*, 3> busNames = {"CAN", "Modbus", "EtherCAT"};
            channel = busEnum[bi];
            busName = busNames[bi];
        }

        // 调用API接口
        LinkerHandApi hand(linkerhand, handType, channel);

#ifdef _WIN32
        std::shared_ptr<Communication::PCANBus> bus = nullptr;
#else
        std::shared_ptr<Communication::CanBus> bus = nullptr;
#endif
        std::shared_ptr<Communication::CanFD> canfd = nullptr;
        std::shared_ptr<Modbus> modbus = nullptr;

        std::string channel_name;

        if (channel == COMM_TYPE::CAN) {
            (argc > 1) ? channel_name = argv[1] : channel_name = "can0";
#ifdef _WIN32
            TPCANHandle pcan_channel = PCAN_USBBUS1;
            if (channel_name == "can1") pcan_channel = PCAN_USBBUS2;
            else if (channel_name.find("PCAN_USBBUS") != std::string::npos) {
                pcan_channel = static_cast<TPCANHandle>(std::stoi(channel_name.substr(12)) + 0x20);
            }
            if (linkerhand == LINKER_HAND::O20) {
                canfd = std::make_shared<Communication::CanFD>(0, 0);
                if (!canfd->init()) { errMsg = "Failed to initialize CANFD"; }
            } else {
                bus = std::make_shared<Communication::PCANBus>(handType);
            }
#else
            if (linkerhand == LINKER_HAND::O20) {
                canfd = std::make_shared<Communication::CanFD>(0, 0);
                if (!canfd->init()) { errMsg = "Failed to initialize CANFD"; }
            } else {
                bus = std::make_shared<Communication::CanBus>(handType);
            }
#endif
            if (errMsg.empty()) {
                if (linkerhand == LINKER_HAND::O20) {
                    hand.setCanTxCallback([canfd](uint32_t can_id, const uint8_t* data, uintptr_t data_len) -> int32_t {
                        std::vector<uint8_t> data_vec(data, data + data_len);
                        canfd->send(data_vec, can_id, true);
                        return 0;
                    });
                    hand.setCanRxCallback([canfd](uint32_t* can_id_out, uint8_t* data_out, uint8_t* len_out) -> int32_t {
                        try {
                            auto frame = canfd->recv(10);
                            if (!frame.valid) return -1;
                            *can_id_out = frame.can_id;
                            *len_out = Communication::CanFD::dlcToLen(frame.can_dlc);
                            memcpy(data_out, frame.data, *len_out);
                            return 0;
                        } catch (const std::exception&) {
                            return -1;
                        }
                    });
                } else {
                    hand.setCanTxCallback([bus](uint32_t can_id, const uint8_t* data, uintptr_t data_len) -> int32_t {
                        std::vector<uint8_t> data_vec(data, data + data_len);
                        bus->send(data_vec, can_id);
                        return 0;
                    });
                    hand.setCanRxCallback([bus](uint32_t* can_id_out, uint8_t* data_out, uint8_t* len_out) -> int32_t {
                        try {
                            auto frame = bus->recv();
                            if (frame.can_id == 0 && frame.can_dlc == 0) return -1;
                            *can_id_out = frame.can_id;
                            *len_out = frame.can_dlc;
                            memcpy(data_out, frame.data, frame.can_dlc);
                            return 0;
                        } catch (const std::exception&) {
                            return -1;
                        }
                    });
                }
            }
        } else if (channel == COMM_TYPE::MODBUS) {
            (argc > 1) ? channel_name = argv[1] : channel_name = "/dev/ttyUSB0";
            modbus = std::make_shared<Modbus>(handType);

            hand.setModbusTxCallback([modbus](uint8_t sid, uint16_t addr, const uint8_t* data, uintptr_t len) -> int32_t {
                if (!modbus) return -1;
                return modbus->sendRawFrame(data, len) ? 0 : -1;
            });
            hand.setModbusRxCallback([modbus](uint8_t sid, uint16_t* addr_out, uint8_t* data_out, uint8_t* len_out) -> int32_t {
                if (!modbus) return -1;
                int len = modbus->receiveCompleteFrame(data_out, 256, 500);
                if (len <= 0 || data_out[0] != sid) return -1;
                *len_out = static_cast<uint8_t>(len);
                if (addr_out) *addr_out = 0;
                return 0;
            });
        } else if (channel == COMM_TYPE::ETHERCAT) {
            errMsg = "EtherCAT 暂不支持 !";
        }

        if (errMsg.empty()) runDashboard(hand, modelName, sideName, busName);
    } // guard 析构：还原终端

    if (!errMsg.empty()) std::cerr << errMsg << std::endl;
    return 0;
}
