// L7 / Modbus / 右手 —— 速度/扭矩/动作序列与各类状态读取
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
#include <array>
#include <memory>

#include "LinkerHandApi.h"
#include "CommunicationCallbacks.h"
#include "Modbus.h"

// 全局 Modbus 设备实例
static std::unique_ptr<Modbus> g_modbus = nullptr;

// 辅助函数：打印向量数据
void printVector(const std::string& label, const std::vector<uint8_t>& vec) {
    std::cout << label << ": [";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << static_cast<int>(vec[i]);
        if (i < vec.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

int main(int argc, char* argv[]) {
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif
    
    std::cout << "========================================" << std::endl;
    std::cout << "    L7 Modbus 测试示例 (RS485 Modbus Device 封装版)" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // 1. 初始化 Modbus 设备
    g_modbus = std::make_unique<Modbus>("/dev/ttyUSB0", 115200);
    if (!g_modbus->isOpen()) {
        std::cerr << "串口打开失败，退出。" << std::endl;
        return -1;
    }

    // 2. 初始化手部 API
    try {
    LinkerHandApi hand(LINKER_HAND::L7, HAND_TYPE::RIGHT, COMM_TYPE::MODBUS);
    
    // 3. 设置 Modbus 回调函数
    hand.setModbusTxCallback([](uint8_t sid, uint16_t addr, const uint8_t* data, uintptr_t len) -> int32_t {
        if (!g_modbus) return -1;
        return g_modbus->sendRawFrame(data, len) ? 0 : -1;
    });

    hand.setModbusRxCallback([](uint8_t sid, uint16_t* addr_out, uint8_t* data_out, uint8_t* len_out) -> int32_t {
        if (!g_modbus) return -1;
        int len = g_modbus->receiveCompleteFrame(data_out, 256, 500);
        if (len <= 0 || data_out[0] != sid) return -1;
        *len_out = static_cast<uint8_t>(len);
        if (addr_out) *addr_out = 0;
        return 0; 
    });
    
    std::cout << "初始化完成！" << std::endl;
    std::cout << std::endl;
    
    // 测试 1: 设置速度
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "测试 1: 设置速度" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::vector<uint8_t> speed(7, 128);  // 7个关节，速度值 128 (L7 有7个关节)
    hand.setSpeed(speed);
    printVector("设置速度", speed);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << std::endl;
    
    // 测试 2: 设置扭矩
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "测试 2: 设置扭矩" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::vector<uint8_t> torque(7, 200);  // 10个关节，扭矩值 200
    hand.setTorque(torque);
    printVector("设置扭矩", torque);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << std::endl;
    
    // 测试 3: 手指动作序列
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "测试 3: 手指动作序列" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // L7 有 7 个关节: 大拇指弯曲、大拇指偏航、食指弯曲、中指弯曲、无名指弯曲、小拇指弯曲、大拇指翻转
    std::vector<std::vector<uint8_t>> poses = {
        {255, 255, 255, 255, 255, 255, 255},  // 完全张开
        {0, 255, 255, 255, 255, 255, 255},    // 大拇指弯曲
        {255, 0, 255, 255, 255, 255, 255},    // 大拇指偏航
        {255, 255, 0, 255, 255, 255, 255},    // 食指弯曲
        {255, 255, 255, 0, 255, 255, 255},    // 中指弯曲
        {255, 255, 255, 255, 0, 255, 255},    // 无名指弯曲
        {255, 255, 255, 255, 255, 0, 255},    // 小拇指弯曲
        {255, 255, 255, 255, 255, 255, 0},    // 大拇指翻转
        {255, 255, 0, 0, 0, 0, 255},          // 握拳
        {255, 255, 255, 255, 255, 255, 255}   // 完全张开
    };
    
    for (size_t i = 0; i < poses.size(); ++i) {
        std::cout << "动作 " << (i + 1) << "/" << poses.size() << ": ";
        printVector("位置", poses[i]);
        hand.setPosition(poses[i]);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << std::endl;
    
    // 测试 4: 读取当前状态
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "测试 4: 读取当前状态" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::vector<uint8_t> state = hand.getPosition();
        if (!state.empty()) {
            printVector("当前关节位置", state);
        } else {
            std::cout << "读取状态失败（可能是回调未实现真实硬件通信）" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << std::endl;
    
    // 测试 5: 读取速度
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "测试 5: 读取速度" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::vector<uint8_t> current_speed = hand.getSpeed();
    if (!current_speed.empty()) {
        printVector("当前速度", current_speed);
    } else {
        std::cout << "读取速度失败（可能是回调未实现真实硬件通信）" << std::endl;
    }
    std::cout << std::endl;
    
    // 测试 6: 读取扭矩
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "测试 6: 读取扭矩" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::vector<uint8_t> current_torque = hand.getTorque();
    if (!current_torque.empty()) {
        printVector("当前扭矩", current_torque);
    } else {
        std::cout << "读取扭矩失败（可能是回调未实现真实硬件通信）" << std::endl;
    }
    std::cout << std::endl;
    
    // 测试 7: 读取温度
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "测试 7: 读取温度" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::vector<uint8_t> temperature = hand.getTemperature();
    if (!temperature.empty()) {
        printVector("电机温度", temperature);
    } else {
        std::cout << "读取温度失败（可能是回调未实现真实硬件通信）" << std::endl;
    }
    std::cout << std::endl;
    
    // 测试 8: 读取故障码
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "测试 8: 读取故障码" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::vector<uint8_t> fault_code = hand.getFaultCode();
    if (!fault_code.empty()) {
        printVector("故障码", fault_code);
    } else {
        std::cout << "读取故障码失败（可能是回调未实现真实硬件通信）" << std::endl;
    }
    std::cout << std::endl;
    
    // 测试 9: 读取版本信息
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "测试 9: 读取版本信息" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::string version = hand.getVersion();
    std::cout << version << std::endl;

    // 测试 10: 读取压感数据
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "测试 10: 读取压感数据" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    for (int i = 0; i < 3; ++i) {
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
                    std::cout << std::setw(2) << static_cast<int>(val) << ' ';
                    std::cout << '\n';
            }
            std::cout << '\n';
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "========================================" << std::endl;
    std::cout << "L7 测试完成！" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}