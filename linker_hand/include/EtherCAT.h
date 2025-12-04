#ifdef __linux__
#ifndef ETHERCAT_H
#define ETHERCAT_H

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <atomic>
#include <chrono>
#include <vector>
#include <thread>
#include <signal.h>
#include <mutex>
#include <ecrt.h>

#include "Common.h"
#include "ICanBus.h"

// constexpr uint32_t PERIOD_US = 10000; // 10ms
constexpr uint32_t PERIOD_US = 1000; // 10ms
/* ------------- EtherCAT 常量 ------------- */
constexpr unsigned int MASTER_INDEX = 0;
constexpr uint32_t cycle_ns = 8000000; // 8ms 周期

namespace Communication //Communicator
{

	class EtherCAT : public ICanBus {

	public:
		EtherCAT(const uint32_t handId);
		~EtherCAT();

		bool init();
		void start();
		void stop();
		void send_can_data(const unsigned int id, const std::vector<uint8_t> &data);

		void send(const std::vector<uint8_t>& data, uint32_t can_id, const bool wait = false) override;
        CANFrame recv(uint32_t& id) override;
        
	private:
		void configure_dc();
		void register_pdo_entries();
		void run_cycle();
		void ec_write(const uint32_t can_id, const std::vector<uint8_t> &data);

		/* ------------- PDO 条目 ------------- */
		ec_pdo_entry_info_t slave_pdo_entries[12] = {
		    // 主站 → 从站（RxPDO）
		    {0x7000, 0x01, 16}, // Control_cmd
		    {0x7000, 0x02, 32}, // Position_Target_1
		    {0x7000, 0x03, 32}, // Velocity_Target_1
		    {0x7000, 0x04, 32}, // Torque_Target_1
		    {0x7000, 0x05, 32}, // KP_1
		    {0x7000, 0x06, 32}, // KD_1
		    // 从站 → 主站（TxPDO）
		    {0x6000, 0x01, 16}, // MOTOR_NUM
		    {0x6000, 0x02, 16}, // MOTOR_ID_1
		    {0x6000, 0x03, 16}, // State_1
		    {0x6000, 0x04, 32}, // Position_Actual_1
		    {0x6000, 0x05, 32}, // Velocity_Actual_1
		    {0x6000, 0x06, 32}, // Torque_Actual_1
		};

		ec_pdo_info_t slave_pdos[2] = {
		    {0x1600, 6, slave_pdo_entries + 0},
		    {0x1A00, 6, slave_pdo_entries + 6},
		};

		ec_sync_info_t slave_syncs[5] = {
		    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
		    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
		    {2, EC_DIR_OUTPUT, 1, slave_pdos + 0, EC_WD_ENABLE},
		    {3, EC_DIR_INPUT, 1, slave_pdos + 1, EC_WD_DISABLE},
		    {0xff}};

		/* ------------- PDO 偏移量 ------------- */
		unsigned int offset_control_cmd = 0;
		unsigned int offset_position_target = 0;
		unsigned int offset_velocity_target = 0;
		unsigned int offset_torque_target = 0;
		unsigned int offset_kp = 0;
		unsigned int offset_kd = 0;

		unsigned int offset_motor_num = 0;
		unsigned int offset_motor_id = 0;
		unsigned int offset_state = 0;
		unsigned int offset_position_actual = 0;
		unsigned int offset_velocity_actual = 0;
		unsigned int offset_torque_actual = 0;

		/* ------------- EtherCAT 句柄 ------------- */
		ec_master_t *master = nullptr;
		ec_domain_t *domain = nullptr;
		uint8_t *domain_pd = nullptr;

		bool running = true;
		uint16_t cmd = 0x04;

		uint32_t can_id_;
		std::vector<uint8_t> can_data;
		
		std::mutex mutex_send; // 互斥锁
		
		uint16_t slave_alias;
		uint16_t slave_position;
		uint32_t vendor_id;
		uint32_t product_code;
		
		bool is_detection_hand;
		uint32_t handId_;
	};
}

#endif // ETHERCAT_H
#endif
