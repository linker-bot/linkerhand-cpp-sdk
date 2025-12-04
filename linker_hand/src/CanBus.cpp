
#ifdef __linux__
#include "CanBus.h"

namespace Communication
{
    CanBus::CanBus(const std::string& interface, int bitrate, const LINKER_HAND linker_hand)
        : interface(interface), bitrate(bitrate), linker_hand(linker_hand)
    {
    	#if 0
        // 创建 CAN 套接字
        socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (socket_fd < 0)
        {
            throw std::runtime_error("Failed to create CAN socket");
        }

        // 获取接口索引
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);
        if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) < 0)
        {
            throw std::runtime_error("Failed to get CAN interface index");
        }

        // 配置 CAN 地址
        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        // 绑定套接字到 CAN 接口
        if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        {
            throw std::runtime_error("Failed to bind CAN socket");
        }

        // 设置 CAN 波特率
        std::string command = "sudo ip link set " + interface + " up type can bitrate " + std::to_string(bitrate);
        
        std::cout << "command : " << command << std::endl;
        
        if (system(command.c_str()) != 0)
        {
            throw std::runtime_error("Failed to set CAN bitrate");
        }
        #endif
        
        socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW); // 打开套接字
        
        struct ifreq ifr = {0};
        strcpy(ifr.ifr_name, interface.c_str());            // 绑定 socket 到 can0 接口
        ioctl(socket_fd, SIOCGIFINDEX, &ifr); // 获取接口索引

        // 设置 CAN 接口的波特率, 替代命令行命令 sudo ip link set can0 up type can bitrate 1000000
        // struct can_bittiming can_bt;
        // can_bt.bitrate = 1000000;
        // struct ifreq ifr_bt = {0};
        // strcpy(ifr_bt.ifr_name, "can0");
        // ifr_bt.ifr_ifru.ifru_data = static_cast<void*>(&can_bt);
        // if (ioctl(socket_fd, SIOCSCANBITTIMING, &ifr_bt) < 0) // 使接口"up"
        // {
        //     std::cerr << "Error setting CAN bitrate: " << strerror(errno) << std::endl;
        //     close(socket_fd);
        //     return false;
        // }
        
        // 停止
        int result;// = system(std::string("sudo ip link set "+ interface +" down").c_str());

        result = system(std::string("sudo ip link set " + interface + " up type can bitrate " + std::to_string(bitrate)).c_str());
        if (result == 0) {
            std::cout << ((interface == "can0") ? "CAN0" : "CAN1") << " interface configured successfully." << std::endl;
        } else {
            std::cerr << "Failed to configure CAN interface." << std::endl;
        }

        struct sockaddr_can addr;
        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;
        if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            std::cerr << "Error in socket bind." << std::endl;
            close(socket_fd);
        }
    }

    CanBus::~CanBus()
    {
    	// 停止CAN接口
        int result = system(std::string("sudo ip link set " + interface + " down").c_str());
    
        // 关闭 CAN 套接字
        // close(socket_fd);
    }

    std::string CanBus::printMillisecondTime() {
        // 获取当前时间点
        auto now = std::chrono::high_resolution_clock::now();
    
        // 使用std::put_time格式化时间点为字符串
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    
        // 获取微秒部分并添加到字符串中
        auto duration = now.time_since_epoch();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration % std::chrono::seconds(1)).count();
        // 只保留毫秒部分
        int milliseconds = microseconds / 1000;
        ss << "." << std::setfill('0') << std::setw(3) << milliseconds;
        // 微妙
        // ss << "." << std::setfill('0') << std::setw(6) << microseconds;

        // 打印格式化的时间字符串
        // std::cout << "send time: " << ss.str() << std::endl;
        return ss.str();
    }

    void CanBus::send(const std::vector<uint8_t>& data, uint32_t can_id, const bool wait)
    {
        std::unique_lock<std::mutex> lock(mutex_send); // 使用 unique_lock 替代 lock_guard

        if (SEND_DEBUG) {
            static std::string hand_str;
            if (linker_hand == LINKER_HAND::L7) {
                hand_str = "L7";
            } else if (linker_hand == LINKER_HAND::L10) {
                hand_str = "L10";
            } else if (linker_hand == LINKER_HAND::L20) {
                hand_str = "L20";
            } else if (linker_hand == LINKER_HAND::L21) {
                hand_str = "L21";
            } else if (linker_hand == LINKER_HAND::L25) {
                hand_str = "L25";
            }
            std::cout << "\033[1;32m# " << hand_str << "-Send\033[0m " << printMillisecondTime() << " | can_id:" << std::hex << can_id << std::dec << " can_dlc:" << data.size() << " data:";
            for (auto &can : data) std::cout << std::hex << (int)can << std::dec << " ";
            std::cout << std::endl;
        }
        
        struct can_frame frame;
        frame.can_id = can_id; // 左手/右手
        frame.can_dlc = data.size();
        memcpy(frame.data, data.data(), frame.can_dlc); // 指令+数据

        if (write(socket_fd, &frame, sizeof(frame)) != sizeof(frame))
        {
            std::cout << "Failed to send CAN frame" << std::endl;
            // std::this_thread::sleep_for(std::chrono::milliseconds(100));
            // if (write(socket_fd, &frame, sizeof(frame)) != sizeof(frame)) {
            //     close(socket_fd);
            //     system("sudo /usr/sbin/ip link set can0 down");
            //     throw std::runtime_error("Failed to send CAN frame");
            // }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(3));

        // updateSendRate();

        #if 1
        // 提前解锁
        lock.unlock();

        if (linker_hand == LINKER_HAND::L10) {
            // 压感
            if (data[0] >= 0xb1 && data[0] <= 0xb6) {
            	if (Common::current_hand_version > 1.4f) {
            		std::this_thread::sleep_for(std::chrono::milliseconds(1));
            	} else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(25)); // 获取每个指头的压感数据最少25毫秒等待，完整指令需要125毫秒，新压感的频率最高是8hz
                }
            }
        }

        if (linker_hand == LINKER_HAND::L7) {
            // 压感
            if (data[0] >= 0xb1 && data[0] <= 0xb6) {
            	if (Common::current_hand_version > 3.2f) {
            		std::this_thread::sleep_for(std::chrono::milliseconds(1));
            	} else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(25));
                }
            }
        }

        if (linker_hand == LINKER_HAND::L20) {
            // 压感
            if (data[0] >= 0xb1 && data[0] <= 0xb6) {
            	std::this_thread::sleep_for(std::chrono::milliseconds(6));
            }
        }

        if (linker_hand == LINKER_HAND::L21 || linker_hand == LINKER_HAND::L25) {
            // std::cout << "L21 or L25" << std::endl;
            // 压感
            if (data[0] >= 0xb1 && data[0] <= 0xb6) {
                // std::this_thread::sleep_for(std::chrono::milliseconds(40));
                std::this_thread::sleep_for(std::chrono::milliseconds(6));
            }
            // 扭矩
            if (data[0] >= 0x51 && data[0] <= 0x55) {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
            // 速度
            if (data[0] >= 0x49 && data[0] <= 0x4D) {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
            // 获取位置
            if (data[0] >= 0x41 && data[0] <= 0x45) {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }
        #endif
    }

    CANFrame CanBus::recv(uint32_t& id)
    {
        struct can_frame frame;
        if (read(socket_fd, &frame, sizeof(frame)) != sizeof(frame)) {
            throw std::runtime_error("Failed to receive CAN frame");
        }

		if(frame.can_id == id) {
			CANFrame result;
			result.can_id = frame.can_id;
			result.can_dlc = frame.can_dlc;
			for (int i = 0; i < frame.can_dlc; ++i){
                result.data[i] = frame.data[i];
            }
			return result;
		}
		return CANFrame{};
    }

    void CanBus::updateSendRate() {
        std::lock_guard<std::mutex> lock(send_mutex);
        send_count++;

        // 每秒计算一次发送速率
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - last_time).count();

        if (elapsed >= 1) {
            std::cout << "CAN帧发送速率: " << send_count << " 帧/秒" << std::endl;
            send_count = 0;
            last_time = current_time;
        }
    }
    
    void CanBus::updateReceiveRate() {
        std::lock_guard<std::mutex> lock(receive_mutex);
        receive_count++;

        // 每秒计算一次发送速率
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - receive_last_time).count();

        if (elapsed >= 1) {
            std::cout << "CAN帧接收速率: " << receive_count << " 帧/秒" << std::endl;
            receive_count = 0;
            receive_last_time = current_time;
        }
    }
    
    void CanBus::setReceiveTimeout(int seconds, int microseconds)
    {
        struct timeval timeout;
        timeout.tv_sec = seconds;
        timeout.tv_usec = microseconds;

        if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0)
        {
            throw std::runtime_error("Failed to set receive timeout");
        }
    }
}
#endif
