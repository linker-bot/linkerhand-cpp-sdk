#ifdef __linux__
#include "EtherCAT.h"

namespace Communication
{
	EtherCAT::EtherCAT(const uint32_t handId) : can_data(8, 0x00), is_detection_hand(false), handId_(handId)
	{
		if (handId == HAND_TYPE::LEFT) {
			vendor_id = 0x00000002;
			product_code = 0x00010000;
		} else if (handId == HAND_TYPE::RIGHT) {
			vendor_id = 0x00000003;
			product_code = 0x00020000;
		} else {
			throw std::runtime_error("Hand type init failed");
		}
	
		if (!init()) {
		    throw std::runtime_error("EtherCAT init failed");
		}
		start();
	}

	EtherCAT::~EtherCAT() 
	{

	}
	
	void EtherCAT::send(const std::vector<uint8_t>& data, uint32_t can_id, const bool wait) 
	{
		std::cout << "can_id_: 0x" << std::hex << can_id << std::dec << std::endl;
	
		std::unique_lock<std::mutex> lock(mutex_send);
		
		uint32_t temp = 0;
		uint8_t id = (int)can_id;
		uint8_t dlc = data.size();
		temp = (temp & 0xFFFF0000) | ((uint32_t)dlc << 24); // 将id放到前16位的前8位
		temp = (temp & 0xFF00FFFF) | ((uint32_t)id << 16); // 将length放到前16位的后8位
		temp = (temp & 0xFFFF0000) | 0x00000000; // 确保后16位都为0
		temp = temp >> 16;
		
		// std::cout << "can_id_: 0x" << std::hex << temp << std::dec << std::endl;
		
		can_id_ = temp;
		can_data = data;
		
		std::this_thread::sleep_for(std::chrono::milliseconds(3));
	}
	
    CANFrame EtherCAT::recv(uint32_t& id) 
    {
    	// CANFrame result;
		// result.can_id = frame.can_id;
		// result.can_dlc = frame.can_dlc;
		// result.data[i] = frame.data[i];
		
    	return CANFrame{};
    }
    
    // 配置 DC 参数
	void EtherCAT::configure_dc() {
	/*
		// 选择参考时钟（使用从站 0 作为参考时钟）
		if (ecrt_master_select_reference_clock(master, slave_position) < 0) {
		    fprintf(stderr, "无法选择参考时钟。\n");
		    exit(EXIT_FAILURE);
		}
		printf("参考时钟已选择。\n");

		// 配置从站的 DC 参数
		for (int i = 0; i < ecrt_master_slave_count(master); i++) {
		    ec_slave_config_t *sc = ecrt_master_slave_config(master, i, 0, 0, 0);
		    if (!sc) {
		        fprintf(stderr, "无法获取从站配置。\n");
		        continue;
		    }

		    // 配置分布式时钟
		    if (ecrt_slave_config_dc(sc, 0x0001, 1000000, 0, 1000000, 0) < 0) {
		        fprintf(stderr, "无法配置从站 DC 参数。\n");
		        continue;
		    }
		    printf("从站 %d DC 参数已配置。\n", i);
		}
		*/
	}

	bool EtherCAT::init() 
	{
		master = ecrt_request_master(MASTER_INDEX);
		if (!master)
		    return false;
		
		ec_master_info_t master_info;
		if (ecrt_master(master, &master_info) < 0) 
			return false;

		for (int i = 0; i < master_info.slave_count; i++) {
		    ec_slave_info_t slave_info;
		    if (ecrt_master_get_slave(master, i, &slave_info) < 0) {
		        fprintf(stderr, "Unable to obtain information for slave station number %d\n", i);
		        continue;
		    }
		    
		    if (slave_info.vendor_id == vendor_id && slave_info.product_code == product_code) {
		    	slave_alias = slave_info.alias;
				slave_position = slave_info.position;
				is_detection_hand = true;
		    }
		    /*
		    auto *sc = ecrt_master_slave_config(master, slave_info.alias, slave_info.position, slave_info.vendor_id, slave_info.product_code);
			if (!sc)
				return false;
				
			// 选择参考时钟（使用从站 0 作为参考时钟）
			if (ecrt_master_select_reference_clock(master, sc) < 0) {
				fprintf(stderr, "无法选择参考时钟。\n");
				exit(EXIT_FAILURE);
			}
			printf("参考时钟已选择。\n");
			
			// 配置 SDO
		    ecrt_slave_config_sdo16(sc, 0x1c32, 1, 2);
		    ecrt_slave_config_sdo16(sc, 0x1c33, 1, 2);

			// 配置分布式时钟
			if (ecrt_slave_config_dc(sc, 0x0300, cycle_ns, cycle_ns / 4, 0, 0) < 0) {
				fprintf(stderr, "无法配置从站 DC 参数。\n");
				return false;
			}
			printf("从站DC 参数已配置。\n");
		    */
		}
		
		if (!is_detection_hand) {
			std::cout << "Hand " << ((handId_ == HAND_TYPE::LEFT) ? "left" : "right") << " not detected!" << std::endl;
			return false;
		}
		
		// configure_dc();

		domain = ecrt_master_create_domain(master);
		
		if (!domain)
		    return false;

		auto *sc = ecrt_master_slave_config(master, slave_alias, slave_position, vendor_id, product_code);
		if (!sc)
			return false;
		
		
		if (ecrt_slave_config_pdos(sc, EC_END, slave_syncs))
		    return false;
		register_pdo_entries();

		if (ecrt_master_activate(master))
		    return false;
		domain_pd = ecrt_domain_data(domain);
		return domain_pd != nullptr;
	}

	void EtherCAT::register_pdo_entries() 
	{
		ec_pdo_entry_reg_t reg[] = {
		    {slave_alias, slave_position, vendor_id, product_code, 0x7000, 0x01, &offset_control_cmd},
		    {slave_alias, slave_position, vendor_id, product_code, 0x7000, 0x02, &offset_position_target},
		    {slave_alias, slave_position, vendor_id, product_code, 0x7000, 0x03, &offset_velocity_target},
		    {slave_alias, slave_position, vendor_id, product_code, 0x7000, 0x04, &offset_torque_target},
		    {slave_alias, slave_position, vendor_id, product_code, 0x7000, 0x05, &offset_kp},
		    {slave_alias, slave_position, vendor_id, product_code, 0x7000, 0x06, &offset_kd},
		    {slave_alias, slave_position, vendor_id, product_code, 0x6000, 0x01, &offset_motor_num},
		    {slave_alias, slave_position, vendor_id, product_code, 0x6000, 0x02, &offset_motor_id},
		    {slave_alias, slave_position, vendor_id, product_code, 0x6000, 0x03, &offset_state},
		    {slave_alias, slave_position, vendor_id, product_code, 0x6000, 0x04, &offset_position_actual},
		    {slave_alias, slave_position, vendor_id, product_code, 0x6000, 0x05, &offset_velocity_actual},
		    {slave_alias, slave_position, vendor_id, product_code, 0x6000, 0x06, &offset_torque_actual},
		    {}};
		ecrt_domain_reg_pdo_entry_list(domain, reg);
	}

	void EtherCAT::ec_write(const uint32_t can_id, const std::vector<uint8_t> &data) 
	{
		static uint32_t data_1;
		static uint32_t data_2;

		// 大端 (Big-Endian)：直接按顺序复制
		memcpy(&data_1, &data[0], sizeof(data_1));
		memcpy(&data_2, &data[4], sizeof(data_2));

		// std::cout << "Big-Endian data_1: 0x" << std::hex << data_1 << std::endl;
		// std::cout << "Big-Endian data_2: 0x" << std::hex << data_2 << std::endl;

		// // 小端 (Little-Endian)：需要反转字节顺序
		// uint32_t data_1_le = 0;
		// uint32_t data_2_le = 0;

		// // 手动反转字节顺序
		// for (int i = 0; i < sizeof(uint32_t); i++) {
		//     data_1_le |= static_cast<uint32_t>(data[i]) << (8 * (3 - i));
		//     data_2_le |= static_cast<uint32_t>(data[4 + i]) << (8 * (3 - i));
		// }

		// std::cout << "Little-Endian data_1_le: 0x" << std::hex << data_1_le << std::endl;
		// std::cout << "Little-Endian data_2_le: 0x" << std::hex << data_2_le << std::endl;

		EC_WRITE_U16(domain_pd + offset_control_cmd, cmd);
		EC_WRITE_U32(domain_pd + offset_position_target, data_1);
		EC_WRITE_U32(domain_pd + offset_velocity_target, data_2);
		EC_WRITE_U32(domain_pd + offset_kp, can_id_);

		// 把用户通过 EC_WRITE_* 宏写入过程映像的输出数据“排队”，准备在下一次发送时一并发出；相当于把 domain 的 datagram 重新挂到主站待发送列表
		ecrt_domain_queue(domain);
		// 把主站当前所有已排队的 datagram（包括 process data、SDO 请求等）一次性打包成以太网帧并发送到总线
		ecrt_master_send(master);
	}

	void EtherCAT::stop() {
		cmd = 0x00;
	}

	void EtherCAT::start() {
		std::thread thread(&EtherCAT::run_cycle, this);
		thread.detach();
	}

	void EtherCAT::send_can_data(const unsigned int id, const std::vector<uint8_t> &data) {
		can_id_ = id;
		can_data = data;
	}

	void EtherCAT::run_cycle() 
	{
		using namespace std::chrono;

		steady_clock::time_point exit_start_time;
		bool exit_timer_started = false;
		
		bool test = false;

		while (running) {
			if (!test) {
				std::cout << "run_cycle running ..." << std::endl;
				test = true;
			}
		    // 从以太网口接收一帧数据
		    if (ecrt_master_receive(master) != 0) { // 检查是否成功接收到数据
		        std::cerr << "Failed to receive data from master." << std::endl;
		    } else {
		        // 把刚收到的数据中属于该 domain 的那部分输入数据复制到用户态的过程映像区
		        ecrt_domain_process(domain);

		        // 检查 domain 的状态
		        ec_domain_state_t state;
		        if (ecrt_domain_state(domain, &state) != 0) {
		            std::cerr << "Failed to get domain state." << std::endl;
		        } else {
		            if (!state.wc_state) {
		                // std::cerr << "Working counter did not change. No new data received." << std::endl;
		            } else {
		                /* ---- 读取反馈 ---- */
		                /*
		                uint16_t motor_num = EC_READ_U16(domain_pd + offset_motor_num);
		                uint16_t motor_id = EC_READ_U16(domain_pd + offset_motor_id);
		                uint16_t state = EC_READ_U16(domain_pd + offset_state);
		                int32_t pos_act = EC_READ_S32(domain_pd + offset_position_actual);
		                int32_t vel_act = EC_READ_S32(domain_pd + offset_velocity_actual);
		                int32_t torq_act = EC_READ_S32(domain_pd + offset_torque_actual);
						*/
						
		                // 打印读取的数据
		                /*
		                std::cout << "Motor Num: " << motor_num 
		                        << ", Motor ID: " << motor_id
		                        << ", State: " << state 
		                        << ", Position: " << pos_act
		                        << ", Velocity: " << vel_act 
		                        << ", Torque: " << torq_act 
		                        << std::endl;
		            	*/
		            }
		        }
		    }

		    // LinkerHand CANData
		    ec_write(can_id_, can_data);
		    
		    if (cmd == 0x00) {
		        // if (!exit_timer_started) {
		        //     exit_start_time = steady_clock::now();
		        //     exit_timer_started = true;
		        // } else {
		        //     if (steady_clock::now() - exit_start_time >= milliseconds(1000)) {
		        //         running = false;
		        //     }
		        // }
		        running = false;
		    }

		    usleep(PERIOD_US);
		}
	}
}
#endif
