#include "ModBus.h"
#if USE_RMAN
namespace Communication
{
ModBus::ModBus(uint32_t handId): handId(handId)
{
    // robotic_arm.rm_set_log_call_back(custom_api_log, 3);
    result = robotic_arm.rm_init(RM_TRIPLE_MODE_E);
    if (result != 0) {
        printf("Initialization failed with error code: %d.\n", result);
    }

    char *api_version = robotic_arm.rm_api_version();
    printf("API Version: %s.\n", api_version);

    const char *robot_ip_address = "192.168.1.18";
    int robot_port = 8080;
    robot_handle = robotic_arm.rm_create_robot_arm(robot_ip_address, robot_port);

    if (robot_handle == NULL) {
        printf("Failed to create robot handle.\n");
    } else {
        printf("Robot handle created successfully: %d\n", robot_handle->id);
    }

    result = rm_set_tool_rs485_mode(robot_handle, 0, 115200);
    if (result != 0) {
        printf("Failed to set tool rs485 mode with error code: %d.\n", result);
    }
}

ModBus::~ModBus()
{
    // Disconnect from the robot arm
    result = robotic_arm.rm_delete_robot_arm(robot_handle);
    if (result != 0) {
        printf("Failed to delete robot handle with error code: %d.\n", result);
    }
}

void ModBus::send(const std::vector<uint8_t>& data, uint32_t &id, const int &start_address, const int &num) 
{
    std::unique_lock<std::mutex> lock(send_mutex);
    writeHoldingRegister(id, start_address, num, data);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
}

std::vector<uint8_t> ModBus::recv(uint32_t& id, const int &start_address, const int &num)
{
    return readHoldingRegister(id, start_address, num);
}

bool ModBus::writeHoldingRegister(const int &id, const int &start_address, const int &num, const std::vector<uint8_t> &data) {
	rm_modbus_rtu_write_params_t param_write;
	param_write.address = start_address;
	param_write.num = num;
	param_write.type = 1;
	param_write.device = id;

	for (int i = 0; i < data.size(); i++) {
		param_write.data[i] = data[i]; 
	}
	result = rm_write_modbus_rtu_registers(robot_handle, param_write);

	if (result != 0) {
		printf("write modbus rtu registers result : %d\n", result);
		return false;
	}
	return true;
}

std::vector<uint8_t> ModBus::readHoldingRegister(const int &id, const int &start_address, const int &num) {
	std::vector<uint8_t> data(num);
	int param_read_data[num] = {0};
	rm_modbus_rtu_read_params_t param_read = {0};
	param_read.address = start_address;
	param_read.num = num;
	param_read.type = 1;
	param_read.device = id;
	result = rm_read_modbus_rtu_holding_registers(robot_handle, param_read, param_read_data);

	if (result != 0) {
		printf("read modbus rtu holding_registers result : %d\n", result);
		return {};
	}

	for (int i = 0; i < num; i++) {
		data[i] = param_read_data[i];
	}
	return data;
}

}
#endif
