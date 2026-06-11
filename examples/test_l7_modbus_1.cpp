// L7 / Modbus / 右手 —— 多线程并发：手势 + 状态监控 + 压力 + 故障检测
#define _USE_MATH_DEFINES
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
#include <array>
#include <mutex>
#include <atomic>
#include <cmath>
#include <memory>

#include "LinkerHandApi.h"
#include "CommunicationCallbacks.h"
#include "Modbus.h"

// 全局变量
static std::unique_ptr<Modbus> g_modbus = nullptr;
static std::atomic<bool> g_running{true};
static std::mutex g_console_mutex;
static std::mutex g_hand_mutex;  // 手部操作互斥锁

// 线程安全日志输出
void safePrint(const std::string& message) {
    std::lock_guard<std::mutex> lock(g_console_mutex);
    std::cout << message << std::endl;
}

void threadSafePrintVector(const std::string& label, const std::vector<uint8_t>& vec) {
    std::lock_guard<std::mutex> lock(g_console_mutex);
    std::cout << label << ": [";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << static_cast<int>(vec[i]);
        if (i < vec.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

// 手部状态监控线程
void handStateMonitorThread(LinkerHandApi& hand) {
    safePrint("[状态监控线程] 启动");
    
    int iteration = 0;
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 10Hz更新
        
        std::lock_guard<std::mutex> hand_lock(g_hand_mutex);
        
        try {
            // 读取关节位置
            std::vector<uint8_t> state = hand.getState();
            if (!state.empty()) {
                std::lock_guard<std::mutex> console_lock(g_console_mutex);
                std::cout << "\033[1;36m[状态监控 " << std::setw(4) << iteration << "] ";
                std::cout << "关节位置: [";
                for (size_t i = 0; i < state.size(); ++i) {
                    std::cout << std::setw(3) << static_cast<int>(state[i]);
                    if (i < state.size() - 1) std::cout << " ";
                }
                std::cout << "]\033[0m" << std::endl;
            }
            
            // 每10次读取一次温度
            if (iteration % 10 == 0) {
                std::vector<uint8_t> temperature = hand.getTemperature();
                if (!temperature.empty()) {
                    std::lock_guard<std::mutex> console_lock(g_console_mutex);
                    std::cout << "\033[1;33m[温度监控] ";
                    std::cout << "电机温度: [";
                    for (size_t i = 0; i < temperature.size(); ++i) {
                        std::cout << std::setw(3) << static_cast<int>(temperature[i]);
                        if (i < temperature.size() - 1) std::cout << " ";
                    }
                    std::cout << "]\033[0m" << std::endl;
                }
            }
            
            iteration++;
            
        } catch (const std::exception& e) {
            safePrint("[状态监控线程] 异常: " + std::string(e.what()));
        }
    }
    
    safePrint("[状态监控线程] 停止");
}

// 压力传感器数据采集线程
void pressureSensorThread(LinkerHandApi& hand) {
    safePrint("[压力传感器线程] 启动");
    
    int finger_index = 0;
    const std::array<const char*, 5> finger_names = {
        "拇指", "食指", "中指", "无名指", "小指"
    };
    
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 2Hz更新
        
        std::lock_guard<std::mutex> hand_lock(g_hand_mutex);
        
        try {
            // 读取所有手指的压力数据
            std::vector<std::vector<std::vector<uint8_t>>> touch_mats = hand.getForce();
            
            if (!touch_mats.empty() && !touch_mats[0].empty()) {
                std::lock_guard<std::mutex> console_lock(g_console_mutex);
                
                // 选择当前手指显示详细数据
                if (finger_index < touch_mats.size() && !touch_mats[finger_index].empty()) {
                    std::cout << "\033[1;32m[压力传感器 " << finger_names[finger_index] << "] ";
                    std::cout << "矩阵 " << touch_mats[finger_index].size() 
                             << "x" << touch_mats[finger_index][0].size() << "\033[0m" << std::endl;
                    
                    // 只显示前几行以避免控制台混乱
                    int rows_to_show = (4 < static_cast<int>(touch_mats[finger_index].size())) ? 4 : static_cast<int>(touch_mats[finger_index].size());
                    for (int r = 0; r < rows_to_show; ++r) {
                        std::cout << "    ";
                        for (size_t c = 0; c < touch_mats[finger_index][r].size(); ++c) {
                            int val = static_cast<int>(touch_mats[finger_index][r][c]);
                            // 根据压力值使用不同颜色
                            if (val > 200) std::cout << "\033[1;31m";
                            else if (val > 100) std::cout << "\033[1;33m";
                            else std::cout << "\033[1;37m";
                            
                            std::cout << std::setw(3) << val << "\033[0m ";
                        }
                        std::cout << std::endl;
                    }
                }
                
                // 计算并显示最大压力值
                int max_pressure = 0;
                int finger_with_max = 0;
                for (size_t f = 0; f < touch_mats.size(); ++f) {
                    for (const auto& row : touch_mats[f]) {
                        for (uint8_t val : row) {
                            if (val > max_pressure) {
                                max_pressure = val;
                                finger_with_max = static_cast<int>(f);
                            }
                        }
                    }
                }
                
                std::cout << "\033[1;35m[最大压力] " << finger_names[finger_with_max] 
                         << ": " << max_pressure << "\033[0m" << std::endl;
            }
            
            // 轮询下一个手指
            finger_index = (finger_index + 1) % 5;
            
        } catch (const std::exception& e) {
            safePrint("[压力传感器线程] 异常: " + std::string(e.what()));
        }
    }
    
    safePrint("[压力传感器线程] 停止");
}

// 手势控制线程
void gestureControlThread(LinkerHandApi& hand) {
    safePrint("[手势控制线程] 启动");
    
    // L7手势序列（7个关节）
    std::vector<std::vector<uint8_t>> gestures = {
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
    
    const std::vector<std::string> gesture_names = {
        "张开", "握拳", "捏取", "三指捏", "拇指向上", "胜利", "OK"
    };
    
    int gesture_index = 0;
    
    while (g_running) {
        // 每个手势保持2秒
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        std::lock_guard<std::mutex> hand_lock(g_hand_mutex);
        
        try {
            // 执行当前手势
            std::vector<uint8_t> pose = gestures[gesture_index];
            
            {
                std::lock_guard<std::mutex> console_lock(g_console_mutex);
                std::cout << "\033[1;34m[手势控制] 执行手势: " << "\033[0m" << std::endl;
            }
            
            hand.fingerMove(pose);
            
            // 下一个手势
            gesture_index = (gesture_index + 1) % gestures.size();
            
        } catch (const std::exception& e) {
            safePrint("[手势控制线程] 异常: " + std::string(e.what()));
        }
    }
    
    safePrint("[手势控制线程] 停止");
}

// 故障检测线程
void faultDetectionThread(LinkerHandApi& hand) {
    safePrint("[故障检测线程] 启动");
    
    int check_count = 0;
    bool last_fault_state = false;
    
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));  // 1Hz检测
        
        std::lock_guard<std::mutex> hand_lock(g_hand_mutex);
        
        try {
            std::vector<uint8_t> fault_codes = hand.getFaultCode();
            
            // 检查是否有故障
            bool has_fault = false;
            for (uint8_t code : fault_codes) {
                if (code != 0) {
                    has_fault = true;
                    break;
                }
            }
            
            if (has_fault) {
                std::lock_guard<std::mutex> console_lock(g_console_mutex);
                std::cout << "\033[1;41m[故障检测 " << std::setw(4) << check_count << "] ";
                std::cout << "检测到故障! 故障码: [";
                for (size_t i = 0; i < fault_codes.size(); ++i) {
                    std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') 
                             << static_cast<int>(fault_codes[i]) << std::dec;
                    if (i < fault_codes.size() - 1) std::cout << ", ";
                }
                std::cout << "]\033[0m" << std::endl;
                
                last_fault_state = true;
                
                // 尝试自动恢复 - 发送张开手势
                if (last_fault_state) {
                    std::cout << "\033[1;33m[故障恢复] 尝试恢复: 发送张开手势\033[0m" << std::endl;
                    std::vector<uint8_t> recovery_pose = {255, 255, 255, 255, 255, 255, 255};
                    hand.fingerMove(recovery_pose);
                }
                
            } else if (last_fault_state) {
                // 故障已清除
                std::lock_guard<std::mutex> console_lock(g_console_mutex);
                std::cout << "\033[1;42m[故障恢复] 故障已清除!\033[0m" << std::endl;
                last_fault_state = false;
            }
            
            check_count++;
            
        } catch (const std::exception& e) {
            safePrint("[故障检测线程] 异常: " + std::string(e.what()));
        }
    }
    
    safePrint("[故障检测线程] 停止");
}

// 性能测试线程（高频率位置控制）
void performanceTestThread(LinkerHandApi& hand) {
    safePrint("[性能测试线程] 启动");
    
    // 测试不同的控制频率
    const std::vector<int> test_durations = {5000, 5000, 5000};  // 5秒每个测试
    const std::vector<int> test_frequencies = {10, 20, 50};      // 10Hz, 20Hz, 50Hz
    const std::vector<std::string> test_names = {"10Hz", "20Hz", "50Hz"};
    
    int test_phase = 0;
    auto test_start_time = std::chrono::steady_clock::now();
    
    while (g_running && test_phase < test_durations.size()) {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            current_time - test_start_time).count();
        
        if (elapsed >= test_durations[test_phase]) {
            // 切换到下一个测试阶段
            test_phase++;
            test_start_time = std::chrono::steady_clock::now();
            
            if (test_phase < test_durations.size()) {
                std::lock_guard<std::mutex> console_lock(g_console_mutex);
                std::cout << "\033[1;36m[性能测试] 切换到 " << test_names[test_phase] 
                         << " 控制频率\033[0m" << std::endl;
            }
            continue;
        }
        
        if (test_phase < test_frequencies.size()) {
            int frequency = test_frequencies[test_phase];
            int period_ms = 1000 / frequency;
            
            // 生成正弦波位置控制信号
            double time_sec = elapsed / 1000.0;
            double angle = 127.5 + 127.5 * sin(2.0 * M_PI * 0.5 * time_sec);  // 0.5Hz正弦波
            
            std::vector<uint8_t> position(7, static_cast<uint8_t>(angle));
            
            {
                std::lock_guard<std::mutex> hand_lock(g_hand_mutex);
                hand.fingerMove(position);
            }
            
            // 控制频率
            std::this_thread::sleep_for(std::chrono::milliseconds(period_ms));
        }
    }
    
    safePrint("[性能测试线程] 测试完成");
}

// 主测试线程（执行一次性测试）
void mainTestThread(LinkerHandApi& hand) {
    safePrint("[主测试线程] 启动");
    
    // 等待其他线程启动
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 设置初始速度和扭矩
    {
        std::lock_guard<std::mutex> hand_lock(g_hand_mutex);
        std::vector<uint8_t> speed(7, 100);
        std::vector<uint8_t> torque(7, 150);
        hand.setSpeed(speed);
        hand.setTorque(torque);
        
        std::lock_guard<std::mutex> console_lock(g_console_mutex);
        std::cout << "\033[1;32m[主测试] 设置速度: 100, 扭矩: 150\033[0m" << std::endl;
    }
    
    // 读取版本信息
    std::this_thread::sleep_for(std::chrono::seconds(1));
    {
        std::lock_guard<std::mutex> hand_lock(g_hand_mutex);
        std::string version = hand.getVersion();
        
        std::lock_guard<std::mutex> console_lock(g_console_mutex);
        std::cout << "\033[1;32m[主测试] 版本信息:\033[0m" << std::endl;
        std::cout << version << std::endl;
    }
    
    // 运行30秒后停止
    std::this_thread::sleep_for(std::chrono::seconds(30));
    
    safePrint("[主测试线程] 测试完成，准备停止所有线程");
    g_running = false;
}

int main(int argc, char* argv[]) {
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif
    
    std::cout << "===============================================" << std::endl;
    std::cout << "    L7 Modbus 多线程并发测试" << std::endl;
    std::cout << "    模拟真实控制场景：手势 + 状态监控 + 压力检测" << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << std::endl;

    try {

    // 2. 初始化手部 API
    LinkerHandApi hand(LINKER_HAND::L7, HAND_TYPE::RIGHT, COMM_TYPE::MODBUS);

    // 1. 初始化 Modbus 设备
    g_modbus = std::make_unique<Modbus>("/dev/ttyUSB0", 115200);
    if (!g_modbus->isOpen()) {
        std::cerr << "错误: 串口打开失败，退出。" << std::endl;
        return -1;
    }
    
    safePrint("串口初始化成功: /dev/ttyUSB0 @ 115200 baud");
    
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
    
    safePrint("手部API初始化完成");
    safePrint("开始多线程测试...");
    std::cout << std::endl;
    
    // 4. 启动所有工作线程
    std::vector<std::thread> threads;
    
    threads.emplace_back(handStateMonitorThread, std::ref(hand));
    threads.emplace_back(pressureSensorThread, std::ref(hand));
    threads.emplace_back(gestureControlThread, std::ref(hand));
    threads.emplace_back(faultDetectionThread, std::ref(hand));
    // threads.emplace_back(performanceTestThread, std::ref(hand));  // 可选：性能测试
    threads.emplace_back(mainTestThread, std::ref(hand));
    
    // 5. 等待所有线程完成
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    // 6. 发送停止手势（张开）
    {
        std::lock_guard<std::mutex> hand_lock(g_hand_mutex);
        std::vector<uint8_t> stop_pose = {255, 255, 255, 255, 255, 255, 255};
        hand.fingerMove(stop_pose);
        safePrint("发送停止手势");
    }
    
    hand.freeModbusCallback();

    std::cout << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << "    多线程测试完成" << std::endl;
    std::cout << "===============================================" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}