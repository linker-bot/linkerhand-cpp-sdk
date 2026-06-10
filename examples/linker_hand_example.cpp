// 全型号 / CAN·CAN-FD·Modbus —— 交互式功能巡检示例（菜单选择型号/手别/总线）
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <atomic>
#include <array>
#include <thread>
#include <chrono>
#include <iomanip>
#include "LinkerHandApi.h"
#include "Modbus.h"

#ifdef _WIN32
#include "PCANBus.h"
#include "CanFD.h"
#else
#include "CanBus.h"
#include "CanFD.h"
#endif

// 定义ANSI转义序列常量
const std::string RESET = "\033[0m";
const std::string BLACK = "\033[30m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string WHITE = "\033[37m";

const std::string BG_BLACK = "\033[40m";
const std::string BG_RED = "\033[41m";
const std::string BG_GREEN = "\033[42m";
const std::string BG_YELLOW = "\033[43m";
const std::string BG_BLUE = "\033[44m";
const std::string BG_MAGENTA = "\033[45m";
const std::string BG_CYAN = "\033[46m";
const std::string BG_WHITE = "\033[47m";

const std::string BOLD = "\033[1m";
const std::string UNDERLINE = "\033[4m";
const std::string REVERSE = "\033[7m";

#define HAND_SPEED 200
#define HAND_TORQUE 255

// 用于标记程序是否应该退出
std::atomic<bool> running(true);

COMM_TYPE channel;

// 封装函数
void printColorLine2() {
#ifdef _WIN32
    std::cout << YELLOW << "----------------------------------------------\n" << RESET;
#else
    std::cout << YELLOW << "——————————————————————————————————————————————\n" << RESET;
#endif
}

void printColorLine1() {
#ifdef _WIN32
    std::cout << YELLOW << "-------------------------\n" << RESET;
#else
    std::cout << YELLOW << "—————————————————————————\n" << RESET;
#endif
}

void printNoColorLine() {
#ifdef _WIN32
    std::cout << "----------------------------------------------" << std::endl;
#else
    std::cout << "——————————————————————————————————————————————" << std::endl;
#endif
}


void exit()
{
    std::cout << "Exit the program ..." << std::endl;
    running = false;
    exit(0);
}

// 辅助函数：将字节向量转换为十六进制字符串
std::string bytesToHex(const std::vector<uint8_t> &bytes)
{
    std::string hexStr;
    for (uint8_t byte : bytes)
    {
        hexStr += std::to_string(byte) + " ";
    }
    return hexStr;
}

// 辅助函数：将二维字节向量转换为十六进制字符串
std::string bytesToHex(const std::vector<std::vector<uint8_t>> &bytes)
{
    std::string hexStr;
    for (const auto &vec : bytes)
    {
        for (uint8_t byte : vec)
        {
            hexStr += std::to_string(byte) + " ";
        }
        hexStr += "\n";
    }
    return hexStr;
}

// 全局变量
int show_count = 0;
int show_count_obj = 0;
int show_step = 0;
std::map<std::string, int> hand = {
    {"joint1", 250}, {"joint2", 250}, {"joint3", 250}, {"joint4", 250}, {"joint5", 250},
    {"joint6", 250}, {"joint7", 250}, {"joint8", 250}, {"joint9", 250}, {"joint10", 250},
    {"joint11", 250}, {"joint12", 0}, {"joint13", 0}, {"joint14", 0}, {"joint15", 0},
    {"joint16", 250}, {"joint17", 250}, {"joint18", 250}, {"joint19", 250}, {"joint20", 250},
    {"joint21", 250}, {"joint22", 250}, {"joint23", 250}, {"joint24", 250}, {"joint25", 250}
};

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

// O20 - 执行动作 - 张开/握拳
std::vector<uint8_t> O20_POSE_OPEN = {
    255, 255, 127, 255, 127, 255, 255, 127, 255, 255, 127, 255, 255, 127, 255, 255, 0
};
std::vector<uint8_t> O20_POSE_CLOSE = {
    0, 0, 127, 0, 127, 0, 0, 127, 0, 0, 127, 0, 0, 127, 0, 0, 0
};

void interactiveMode(LinkerHandApi &hand)
{
    int choice;
    while (running)
    {
        // std::cout << YELLOW << "——————————————————————————————————————————————\n" << RESET;
        std::cout << GREEN << "Run Choose Task:\n" << RESET;
        printColorLine2();
        std::cout << BLUE << " 1. Obtain version information\n" << RESET;
        std::cout << BLUE << " 2. Obtain temperature\n" << RESET;
        std::cout << BLUE << " 3. Obtain fault codes\n" << RESET;
        std::cout << BLUE << " 4. Obtain torque\n" << RESET;
        std::cout << BLUE << " 5. Obtain speed\n" << RESET;
        std::cout << BLUE << " 6. Obtain pressure sensing information\n" << RESET;
        std::cout << BLUE << " 7. Clear fault codes\n" << RESET;
        std::cout << BLUE << " 8. Loop to obtain the current finger joint status 10 times\n" << RESET;
        std::cout << BLUE << " 9. Execute preset actions\n" << RESET;
        std::cout << RED << " 0. Exit\n" << RESET;
        printColorLine2();
        std::cout << GREEN << "Please enter options: " << RESET;
        std::cin >> choice;


        if (choice != 1)
            printNoColorLine();

        std::string pose_str;
        std::vector<uint8_t> pose;

        switch (choice)
        {
        case 1:
            std::cout << hand.getVersion() << std::endl;
            break;
        case 2:
            std::cout << "Obtain temperature: " << bytesToHex(hand.getTemperature()) << std::endl;
            break;
        case 3:
            std::cout << "Obtain fault codes: " << bytesToHex(hand.getFaultCode()) << std::endl;
            break;
        case 4:
            std::cout << "Obtain torque: " << std::endl;
            for (size_t i = 0; i < 10; i++)
            {
                std::cout << bytesToHex(hand.getTorque()) << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            break;
        case 5:
            std::cout << "Obtain speed: " << bytesToHex(hand.getSpeed()) << std::endl;
            break;
        case 6:
            for (size_t i = 0; i < 30; i++)
            {
                std::vector<std::vector<std::vector<uint8_t>>> touch_mats = hand.getForce();
                // 手指名称表，顺序对应 touch_mats[0..4]
                const std::array<const char*, 5> finger_name = {
                    "THUMB_TOUCH", "INDEX_TOUCH", "MIDDLE_TOUCH", "RING_TOUCH", "LITTLE_TOUCH"
                };

                for (size_t n = 0; n < touch_mats.size(); ++n) {
                    std::cout << finger_name[n] << ":\n";
                    for (const auto &row : touch_mats[n])
                    {
                        for (uint8_t val : row)
                            std::cout << std::setw(3) << static_cast<int>(val) << ' ';
                        std::cout << '\n';
                    }
                    std::cout << '\n';
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            break;
        case 7:
            std::cout << "Clear fault codes\n" << std::endl;
            hand.clearFaultCode();
            break;
        case 8:
            for (size_t i = 0; i < 10; i++)
            {
                std::cout << bytesToHex(hand.getState()) << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            break;
        case 9:
            if (hand.handJoint_ == LINKER_HAND::L6) {
                std::cout << "L6 - Execute action" << std::endl;
                for (size_t i = 0; i < 10; i++) {
                    for (const auto &pose : L6_POSE_1) {
                        hand.fingerMove(pose);
                        std::this_thread::sleep_for(std::chrono::milliseconds(150));
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(150));
                }
            } else if (hand.handJoint_ == LINKER_HAND::O6) {
                std::cout << "O6 - Execute action" << std::endl;
                for (const auto &pose : L6_POSE_1) {
                    hand.fingerMove(pose);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
                }
            } else if (hand.handJoint_ == LINKER_HAND::L7) {
                hand.setSpeed(std::vector<uint8_t>(7, HAND_SPEED)); // L7 need 7 speed
                hand.setTorque(std::vector<uint8_t>(7, HAND_TORQUE)); // L7 need 7 torque
                //---------------------------------------------------------
                std::cout << "L7 - Execute action - Make a fist" << std::endl;
                for (const auto &pose : L7_POSE_1)
                {
                    hand.fingerMove(pose);
                    std::this_thread::sleep_for(std::chrono::seconds(1)); // 等待1秒
                }
                //---------------------------------------------------------
                std::cout << "L7 - Execute action - Open hand" << std::endl;
                hand.fingerMove(L7_POSE_OPEN);
                std::this_thread::sleep_for(std::chrono::seconds(1));
                //---------------------------------------------------------
            }
            else if (hand.handJoint_ == LINKER_HAND::L10)
            {
                hand.setSpeed(std::vector<uint8_t>(10, HAND_SPEED)); // L10 need 10 speed
                hand.setTorque(std::vector<uint8_t>(10, HAND_TORQUE)); // L10 need 10 torque
                //---------------------------------------------------------
                std::cout << "L10 - Execute action - Finger cycle bending" << std::endl;
                for (const auto &pose : L10_POSE_1)
                {
                    hand.fingerMove(pose);
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
                //---------------------------------------------------------
                std::cout << "L10 - Execute action - Thumb and Circular Fingers" << std::endl;
                for (const auto &pose : L10_POSE_2)
                {
                    hand.fingerMove(pose);
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
                //---------------------------------------------------------
                std::cout << "L10 - Execute action - Finger side swing" << std::endl;
                for (const auto &pose : L10_POSE_3)
                {
                    hand.fingerMove(pose);
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
                //---------------------------------------------------------
                std::cout << "L10 - Execute action - Make a fist" << std::endl;
                hand.fingerMove(L10_POSE_CLOSE);
                std::this_thread::sleep_for(std::chrono::seconds(1));
                //---------------------------------------------------------
                std::cout << "L10 - Execute action - Open hand" << std::endl;
                hand.fingerMove(L10_POSE_OPEN);
                std::this_thread::sleep_for(std::chrono::seconds(1));
                //---------------------------------------------------------
            }
            else if (hand.handJoint_ == LINKER_HAND::L20)
            {
                hand.setSpeed(std::vector<uint8_t>(5, HAND_SPEED)); // L20 need 5 speed
                //---------------------------------------------------------
                std::cout << "L20 - Execute action - Make a fist" << std::endl;
                std::vector<uint8_t> L20_POSE_CLOSE = {69, 0, 0, 0, 0, 32, 10, 100, 180, 240, 145, 255, 255, 255, 255, 0, 0, 0, 0, 0};
                hand.fingerMove(L20_POSE_CLOSE);
                std::this_thread::sleep_for(std::chrono::seconds(1));
                //---------------------------------------------------------
                std::cout << "L20 - Execute action - Open hand" << std::endl;
                std::vector<uint8_t> L20_POSE_OPEN = {255, 255, 230, 255, 255, 255, 10, 100, 180, 240, 245, 255, 255, 255, 255, 255, 255, 255, 255, 255};
                hand.fingerMove(L20_POSE_OPEN);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            else if (hand.handJoint_ == LINKER_HAND::L25 || hand.handJoint_ == LINKER_HAND::L21)
            {
                hand.setSpeed(std::vector<uint8_t>(25, HAND_SPEED));
                hand.setTorque(std::vector<uint8_t>(25, HAND_TORQUE));
                //---------------------------------------------------------

                // Open hand
                std::vector<uint8_t> pos1_1 = {230, 0, 0, 15, 5, 250, 55, 80, 210, 202, 85, 0, 0, 0, 0, 250, 0, 40, 35, 5, 250, 0, 5, 0, 0};
                std::vector<uint8_t> pos1_2 = {80, 255, 255, 255, 255, 180, 51, 51, 72, 202, 202, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
                std::vector<uint8_t> pos1_3 = {75, 255, 255, 255, 255, 176, 51, 80, 210, 202, 202, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

                // Make a fist
                std::vector<uint8_t> pos2_1 = {230, 0, 0, 15, 5, 250, 55, 80, 210, 202, 85, 0, 0, 0, 0, 80, 0, 40, 35, 5, 250, 0, 5, 0, 0};
                std::vector<uint8_t> pos2_2 = {230, 0, 0, 15, 5, 42, 55, 80, 210, 202, 85, 0, 0, 0, 0, 90, 0, 40, 35, 5, 120, 0, 5, 0, 0};

                std::cout << "L25/L21 - Execute action - Make a fist" << std::endl;
                hand.fingerMove(pos2_1);
                std::this_thread::sleep_for(std::chrono::seconds(1));
                hand.fingerMove(pos2_2);
                std::this_thread::sleep_for(std::chrono::seconds(1));

                std::cout << "L25/L21 - Execute action - Open hand" << std::endl;
                hand.fingerMove(pos1_1);
                std::this_thread::sleep_for(std::chrono::seconds(1));
                // hand.fingerMove(pos1_2);
                // std::this_thread::sleep_for(std::chrono::seconds(1));

                hand.fingerMove(pos1_3);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            else if (hand.handJoint_ == LINKER_HAND::G20)
            {
                hand.setSpeed(std::vector<uint8_t>(5, HAND_SPEED)); // L20 need 5 speed
                //---------------------------------------------------------
                std::cout << "G20 - Execute action - Make a fist" << std::endl;
                std::vector<uint8_t> G20_POSE_CLOSE_1 = {255, 0, 0, 0, 0,255,255,178,84,0,255,255,0,0,0,0};
                hand.fingerMove(G20_POSE_CLOSE_1);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                std::vector<uint8_t> G20_POSE_CLOSE_2 = {117, 0, 0, 0, 0,47,255,178,84,0,115,104,0,0,0,0};
                hand.fingerMove(G20_POSE_CLOSE_2);
                std::this_thread::sleep_for(std::chrono::seconds(1));
                //---------------------------------------------------------
                std::cout << "G20 - Execute action - Open hand" << std::endl;
                std::vector<uint8_t> G20_POSE_OPEN = {255, 255, 255, 255, 255, 255, 255, 172, 74, 0, 255, 255, 255, 255, 255, 255};
                hand.fingerMove(G20_POSE_OPEN);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            else if (hand.handJoint_ == LINKER_HAND::O20)
            {
                std::cout << "暂无动作！" << std::endl;
            }
            break;
        case 0:
            exit();
            break;
        default:
            std::cout << "Invalid option, please re-enter!\n";
        }
        if (choice != 1)
            printNoColorLine();
    }
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    // 设置控制台输出编码为 UTF-8，解决中文乱码问题
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::cout << YELLOW << "=========================\n" << RESET;
    std::cout << YELLOW << "        Example\n" << RESET;
    std::cout << YELLOW << "=========================\n" << RESET;

    int choice;
    LINKER_HAND linkerhand;
    while (true)
    {
        std::cout << GREEN << "\nRun Choose LinkerHand:\n" << RESET;
        printColorLine1();
        std::cout << BLUE << "[1]: L6\n" << RESET;
        printColorLine1();
        std::cout << BLUE << "[2]: L7\n" << RESET;
        printColorLine1();
        std::cout << BLUE << "[3]: L10\n" << RESET;
        printColorLine1();
        std::cout << BLUE << "[4]: L20\n" << RESET;
        printColorLine1();
        std::cout << BLUE << "[5]: L21\n" << RESET;
        printColorLine1();
        std::cout << BLUE << "[6]: L25\n" << RESET;
        printColorLine1();
        std::cout << BLUE << "[7]: O6\n" << RESET;
        printColorLine1();
        std::cout << BLUE << "[8]: G20\n" << RESET;
        printColorLine1();
        std::cout << BLUE << "[9]: O20\n" << RESET;
        printColorLine1();
        std::cout << RED << "[0]: Exit\n" << RESET;
        printColorLine1();
        std::cout << GREEN << "Please enter options: " << RESET;
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            linkerhand = LINKER_HAND::L6;
            break;
        case 2:
            linkerhand = LINKER_HAND::L7;
            break;
        case 3:
            linkerhand = LINKER_HAND::L10;
            break;
        case 4:
            linkerhand = LINKER_HAND::L20;
            break;
        case 5:
            linkerhand = LINKER_HAND::L21;
            break;
        case 6:
            linkerhand = LINKER_HAND::L25;
            break;
        case 7:
            linkerhand = LINKER_HAND::O6;
            break;
        case 8:
            linkerhand = LINKER_HAND::G20;
            break;
        case 9:
            linkerhand = LINKER_HAND::O20;
            break;
        case 0:
            exit();
            break;
        default:
            std::cout << "Invalid option, please re-enter!\n";
            continue;
        }
        break;
    }

    HAND_TYPE handType;
    while (true)
    {
        printColorLine2();
        std::cout << GREEN << "Run Choose Hand Direction:\n" << RESET;
        printColorLine1();
        std::cout << BLUE << "[1]: Left Hand\n" << RESET;
        printColorLine1();
        std::cout << BLUE << "[2]: Right Hand\n" << RESET;
        printColorLine1();
        std::cout << RED << "[0]: Exit\n" << RESET;
        printColorLine1();
        std::cout << GREEN << "Please enter options: " << RESET;
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            handType = HAND_TYPE::LEFT;
            break;
        case 2:
            handType = HAND_TYPE::RIGHT;
            break;
        case 0:
            exit();
            break;
        default:
            std::cout << "Invalid option, please re-enter!\n";
            continue;
        }
        break;
    }

    if (linkerhand == LINKER_HAND::O20) {
        channel = COMM_TYPE::CAN;
        std::cout << "O20 uses CANFD transport, skip bus selection." << std::endl;
    } else {
        while (true)
        {
            printColorLine2();
            std::cout << GREEN << "Run Choose CANBus:\n" << RESET;
            printColorLine1();
            std::cout << BLUE << "[1]: CAN\n" << RESET;
            printColorLine1();
            std::cout << BLUE << "[2]: Modbus\n" << RESET;
            printColorLine1();
            std::cout << BLUE << "[3]: EtherCAT\n" << RESET;
            printColorLine1();
            std::cout << RED << "[0]: Exit\n" << RESET;
            printColorLine1();
            std::cout << GREEN << "Please enter options: " << RESET;
            std::cin >> choice;

            switch (choice)
            {
            case 1:
                channel = COMM_TYPE::CAN;
                break;
            case 2:
                channel = COMM_TYPE::MODBUS;
                break;
            case 3:
                channel = COMM_TYPE::ETHERCAT;
                break;
            case 0:
                exit();
                break;
            default:
                std::cout << "Invalid option, please re-enter!\n";
                continue;
            }
            break;
        }
    }

    // 调用API接口
    LinkerHandApi hand(linkerhand, handType, channel);

#ifdef _WIN32
    std::shared_ptr<Communication::PCANBus> bus = nullptr;
    std::shared_ptr<Communication::CanFD> canfd = nullptr;
#else
    std::shared_ptr<Communication::CanBus> bus = nullptr;
    std::shared_ptr<Communication::CanFD> canfd = nullptr;
#endif
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
            if (!canfd->init()) {
                std::cerr << "Failed to initialize CANFD" << std::endl;
                return 1;
            }
        } else {
            bus = std::make_shared<Communication::PCANBus>(handType);
        }
#else
        if (linkerhand == LINKER_HAND::O20) {
            canfd = std::make_shared<Communication::CanFD>(0, 0);
            if (!canfd->init()) {
                std::cerr << "Failed to initialize CANFD" << std::endl;
                return 1;
            }
        } else {
            bus = std::make_shared<Communication::CanBus>(handType);
        }
#endif

        if (linkerhand == LINKER_HAND::O20) {
            hand.setCanTxCallback([canfd](uint32_t can_id, const uint8_t *data, uintptr_t data_len) -> int32_t {
                std::vector<uint8_t> data_vec(data, data + data_len);
                canfd->send(data_vec, can_id, true);
                return 0;
            });

            hand.setCanRxCallback([canfd](uint32_t* can_id_out, uint8_t* data_out, uint8_t* len_out) -> int32_t {
                try {
                    auto frame = canfd->recv(10);
                    if (!frame.valid) {
                        return -1;
                    }
                    *can_id_out = frame.can_id;
                    *len_out = Communication::CanFD::dlcToLen(frame.can_dlc);
                    memcpy(data_out, frame.data, *len_out);
                    return 0;
                } catch (const std::exception& e) {
                    std::cerr << "CANFD接收错误: " << e.what() << std::endl;
                    return -1;
                }
            });
        } else {
            hand.setCanTxCallback([bus](uint32_t can_id, const uint8_t *data, uintptr_t data_len) -> int32_t {
                std::vector<uint8_t> data_vec(data, data + data_len);
                bus->send(data_vec, can_id);
                return 0;
            });

            hand.setCanRxCallback([bus](uint32_t* can_id_out, uint8_t* data_out, uint8_t* len_out) -> int32_t {
                try {
                    auto frame = bus->recv();
                    if (frame.can_id == 0 && frame.can_dlc == 0) {
                        return -1;
                    }
                    *can_id_out = frame.can_id;
                    *len_out = frame.can_dlc;
                    memcpy(data_out, frame.data, frame.can_dlc);
                    return 0;
                } catch (const std::exception& e) {
                    std::cerr << "CAN接收错误: " << e.what() << std::endl;
                    return -1;
                }
            });
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
        std::cout << "EtherCAT 暂不支持 !" << std::endl;
    }
    printNoColorLine();
    // 启动交互模式
    interactiveMode(hand);

    return 0;
}
