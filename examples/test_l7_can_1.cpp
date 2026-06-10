// L7 / CAN / 右手 —— 多线程并发测试（版本/参数/触觉/运动并行）
#include <array>
#include <iostream>
#include <thread>
#include <vector>
#include <iomanip>
#include <future>
#include <mutex>
#include <memory>

#include "LinkerHandApi.h"
#include "CommFactory.h"

// 格式化当前时间，用于 TX/RX 日志
std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    #if defined(_WIN32) || defined(_WIN64)
        std::tm tm_info = {};
        localtime_s(&tm_info, &now_time);
        std::tm* tm = &tm_info;
    #else
        std::tm* tm = std::localtime(&now_time);
    #endif
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);

    std::ostringstream oss;
    oss << buffer << "." << std::setfill('0') << std::setw(3) << now_ms.count();
    return oss.str();
}

// 全局互斥锁，用于控制输出顺序
std::mutex cout_mutex;

void printWithLock(const std::string& message) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << message << std::endl;
}

// 线程1：获取版本信息（独立运行，不依赖其他数据）
void threadGetVersion(LinkerHandApi& hand, int iteration) {
    try {
        auto start = std::chrono::steady_clock::now();
        std::string version = hand.getVersion();
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::stringstream ss;
        ss << "[Thread1-Iter" << iteration << "] 版本获取完成 (" << duration.count() << "ms): " << version;
        printWithLock(ss.str());
        
        // 版本获取后等待3ms
        //std::this_thread::sleep_for(std::chrono::milliseconds(3));
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "[Thread1-Iter" << iteration << "] 版本获取失败: " << e.what();
        printWithLock(ss.str());
    }
}

// 线程2：设置扭矩和速度（这两个可以并行）
void threadSetParameters(LinkerHandApi& hand, int iteration) {
    try {
        auto start = std::chrono::steady_clock::now();
        
        // 同时设置扭矩和速度
        std::vector<uint8_t> torque_values = {200,200,200,200,200,200,200,};
        std::vector<uint8_t> speed_values = {200,200,200,200,200,200,200};
        
        hand.setTorque(torque_values);
        // 设置扭矩后等待3ms
        //std::this_thread::sleep_for(std::chrono::milliseconds(3));
        
        hand.setSpeed(speed_values);
        // 设置速度后等待3ms
        //std::this_thread::sleep_for(std::chrono::milliseconds(3));
        
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::stringstream ss;
        ss << "[Thread2-Iter" << iteration << "] 参数设置完成 (" << duration.count() << "ms)";
        printWithLock(ss.str());
        
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "[Thread2-Iter" << iteration << "] 参数设置失败: " << e.what();
        printWithLock(ss.str());
    }
}

// 线程3：获取扭矩、速度、状态（这些可以并行获取）
void threadGetParameters(LinkerHandApi& hand, int iteration) {
    try {
        auto start = std::chrono::steady_clock::now();
        
        std::vector<uint8_t> torque;
        std::vector<uint8_t> speed;
        std::vector<uint8_t> state;
        
        // 并行获取三个参数
        auto future_torque = std::async(std::launch::async, [&hand]() {
            return hand.getTorque();
        });
        
        auto future_speed = std::async(std::launch::async, [&hand]() {
            return hand.getSpeed();
        });
        
        auto future_state = std::async(std::launch::async, [&hand]() {
            return hand.getState();
        });
        
        // 等待所有结果
        torque = future_torque.get();
        //std::this_thread::sleep_for(std::chrono::milliseconds(3));  // 获取扭矩后等待3ms
        
        speed = future_speed.get();
        //std::this_thread::sleep_for(std::chrono::milliseconds(3));  // 获取速度后等待3ms
        
        state = future_state.get();
        //std::this_thread::sleep_for(std::chrono::milliseconds(3));  // 获取状态后等待3ms
        
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::stringstream ss;
        ss << "[Thread3-Iter" << iteration << "] 参数获取完成 (" << duration.count() << "ms)\n";
        ss << "  扭矩大小: " << torque.size() << " 数据: ";
        for (size_t i = 0; i < torque.size() && i < 5; ++i) {
            ss << (int)torque[i] << " ";
        }
        if (torque.size() > 5) ss << "...";
        
        ss << "\n  速度大小: " << speed.size() << " 数据: ";
        for (size_t i = 0; i < speed.size() && i < 5; ++i) {
            ss << (int)speed[i] << " ";
        }
        if (speed.size() > 5) ss << "...";
        
        ss << "\n  状态大小: " << state.size() << " 数据: ";
        for (size_t i = 0; i < state.size() && i < 5; ++i) {
            ss << (int)state[i] << " ";
        }
        if (state.size() > 5) ss << "...";
        
        printWithLock(ss.str());
        
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "[Thread3-Iter" << iteration << "] 参数获取失败: " << e.what();
        printWithLock(ss.str());
    }
}

// 线程4：获取压感数据（5个手指可以并行获取）
void threadGetTouchData(LinkerHandApi& hand, int iteration) {
    try {
        auto start = std::chrono::steady_clock::now();
        
       std::vector<std::vector<std::vector<uint8_t>>> touch_mats = hand.getForce();
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
        
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::stringstream ss;
        ss << "[Thread4-Iter" << iteration << "] 压感数据获取完成 (" << duration.count() << "ms)";
        printWithLock(ss.str());
        
        // 打印压感数据
        for (size_t n = 0; n < touch_mats.size(); ++n) {
            if (!touch_mats[n].empty()) {
                std::stringstream ss2;
                ss2 << "  " << finger_name[n] << " 矩阵 " 
                    << touch_mats[n].size() << "x" 
                    << (touch_mats[n].empty() ? 0 : touch_mats[n][0].size())
                    << " 第一个值: " << (int)touch_mats[n][0][0];
                printWithLock(ss2.str());
            }
        }
        
    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "[Thread4-Iter" << iteration << "] 压感获取失败: " << e.what();
        printWithLock(ss.str());
    }
}

// 线程5：运动控制（握拳和张开）
void threadMotionControl(LinkerHandApi& hand, int iteration) {
    try {
        auto start = std::chrono::steady_clock::now();

        // 握拳
        std::vector<uint8_t> fist_pose = {255, 255, 0, 0, 0, 0, 255};
        hand.fingerMove(fist_pose);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // 张开
        std::vector<uint8_t> open_pose = {255, 255, 255, 255, 255, 255, 255};
        hand.fingerMove(open_pose);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::stringstream ss;
        ss << "[Thread5-Iter" << iteration << "] 运动控制完成 (" << duration.count() << "ms)";
        printWithLock(ss.str());

    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "[Thread5-Iter" << iteration << "] 运动控制失败: " << e.what();
        printWithLock(ss.str());
    }
}

int main() {
    try {

    // 调用API接口
    LinkerHandApi hand(LINKER_HAND::L7, HAND_TYPE::RIGHT);

    // 创建CAN总线对象用于通信
    std::shared_ptr<Communication::ICanBus> bus = Communication::CommFactory::createCanBus("can0", 1000000);
    
    hand.setCanTxCallback([bus](uint32_t can_id, const uint8_t *data, uintptr_t data_len) -> int32_t {
        
        // 打印发送信息
        std::cout << "\033[32m[TX #" << "]\033[0m " 
                  << getCurrentTime() 
                  << " | CAN_ID: 0x" << std::hex << std::setw(8) << std::setfill('0') << can_id
                  << std::dec << " (" << can_id << ")"
                  << " | Len: " << data_len
                  << " | Data: ";
        
        // 打印数据
        for (uintptr_t i = 0; i < data_len; i++) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') 
                      << (int)data[i] << " ";
        }
        std::cout << std::dec << std::endl;

        std::vector<uint8_t> data_vec(data, data + data_len);
        bus->send(data_vec, can_id);
        return 0;
    });
    
    hand.setCanRxCallback([bus](uint32_t* can_id_out, uint8_t* data_out, uint8_t* len_out) -> int32_t {
        try {

            // 接收CAN帧
            auto frame = bus->recv();
            
            // 检查是否为空帧
            if (frame.can_id == 0 && frame.can_dlc == 0) {
                return -1;  // 忽略空帧
            }
            
            // 填充输出参数
            *can_id_out = frame.can_id;
            *len_out = frame.can_dlc;
            // 添加安全检查以避免缓冲区溢出
            if (frame.can_dlc <= 8) {  // CAN帧的最大数据长度为8字节
                memcpy(data_out, frame.data, frame.can_dlc);
            } else {
                return -1;  // 数据长度超出范围
            }
            
            
            static int rx_count = 0;
            rx_count++;
            
            // 打印接收信息
            std::cout << "\033[34m[RX #" << rx_count << "]\033[0m " 
                      << getCurrentTime() 
                      << " | CAN_ID: 0x" << std::hex << std::setw(8) << std::setfill('0') << frame.can_id
                      << std::dec << " (" << frame.can_id << ")"
                      << " | DLC: " << (int)frame.can_dlc
                      << " | Data: ";
            
            // 打印数据
            for (int i = 0; i < frame.can_dlc; i++) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') 
                          << (int)frame.data[i] << " ";
            }
            std::cout << std::dec << std::endl;
            
            
            return 0; // 成功
        } catch (const std::exception& e) {
            std::cerr << "CAN接收错误: " << e.what() << std::endl;
            return -1; // 失败
        }
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::cout << "=== 开始多线程并发测试 ===\n";
    auto total_start = std::chrono::steady_clock::now();
    
    for (size_t iter = 0; iter < 1; ++iter) {
        printWithLock("\n=== 迭代 " + std::to_string(iter + 1) + " ===");
        auto iter_start = std::chrono::steady_clock::now();
        
        // 创建并启动所有线程
        std::vector<std::thread> threads;
        
        // 线程1：获取版本
        threads.emplace_back(threadGetVersion, std::ref(hand), iter + 1);
        
        // 线程2：设置参数
        threads.emplace_back(threadSetParameters, std::ref(hand), iter + 1);
        
        // 线程3：获取参数
        threads.emplace_back(threadGetParameters, std::ref(hand), iter + 1);
        
        // 线程4：获取压感
        threads.emplace_back(threadGetTouchData, std::ref(hand), iter + 1);
        
        // 线程5：运动控制
        threads.emplace_back(threadMotionControl, std::ref(hand), iter + 1);
        
        // 等待所有线程完成
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
        
        auto iter_end = std::chrono::steady_clock::now();
        auto iter_duration = std::chrono::duration_cast<std::chrono::milliseconds>(iter_end - iter_start);
        
        std::stringstream ss;
        ss << "迭代 " << iter + 1 << " 完成，耗时: " << iter_duration.count() << "ms";
        printWithLock(ss.str());
        
        // 迭代之间等待100ms
        if (iter < 2) {  // 修改循环次数判断
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    auto total_end = std::chrono::steady_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_end - total_start);
    
    std::cout << "\n=== 测试完成 ===\n";
    std::cout << "总耗时: " << total_duration.count() << "ms\n";
    std::cout << "平均每迭代: " << total_duration.count() / 3.0 << "ms\n";  // 修改平均计算
    
    // 等待一段时间确保所有回调完成
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}