#include "LinkerHandL25.h"

namespace LinkerHandL25
{
LinkerHand::LinkerHand(uint32_t handId, const std::string &canChannel, int baudrate, const int currentHandType) : handId(handId), running(true), current_hand_type(currentHandType)
{
    bus = Communication::CanBusFactory::createCanBus(handId, canChannel, baudrate, currentHandType == 0 ? LINKER_HAND::L25 : LINKER_HAND::L21);

    thumb_pressure = std::vector<uint8_t>(72, 0);
    index_finger_pressure = std::vector<uint8_t>(72, 0);
    middle_finger_pressure = std::vector<uint8_t>(72, 0);
    ring_finger_pressure = std::vector<uint8_t>(72, 0);
    little_finger_pressure = std::vector<uint8_t>(72, 0);

    // 1. 定义并初始化
    touch_mats.assign(
        5,
        std::vector<std::vector<uint8_t>>(
            12,
            std::vector<uint8_t>(6, 0)
        )
    );

    // // 2. 打印
    // for (size_t n = 0; n < touch_mats.size(); ++n)
    // {
    //     std::cout << "Matrix #" << n << ":\n";
    //     for (const auto &row : touch_mats[n])
    //     {
    //         for (uint8_t val : row)
    //             std::cout << std::setw(3) << static_cast<int>(val) << ' ';
    //         std::cout << '\n';
    //     }
    //     std::cout << '\n'; // 两个矩阵之间空一行
    // }

    receiveThread = std::thread(&LinkerHand::receiveResponse, this);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    bus->send({TOUCH_SENSOR_TYPE}, handId);
    bus->send({FRAME_PROPERTY::THUMB_TOUCH}, handId);


    
}

LinkerHand::~LinkerHand()
{
    running = false;
    if (receiveThread.joinable())
    {
        receiveThread.join();
    }
}

void LinkerHand::setJointPositions(const std::vector<uint8_t> &jointAngles)
{
    if (jointAngles.size() == 25)
    {
        std::vector<uint8_t> joint_array;
        joint_array.push_back(jointAngles[10]);
        joint_array.push_back(jointAngles[5]);
        joint_array.push_back(jointAngles[0]);
        joint_array.push_back(jointAngles[15]);
        joint_array.push_back(0);
        joint_array.push_back(jointAngles[20]);
        joint_array.push_back(0);
        joint_array.push_back(jointAngles[6]);
        joint_array.push_back(jointAngles[1]);
        joint_array.push_back(jointAngles[16]);
        joint_array.push_back(0);
        joint_array.push_back(jointAngles[21]);
        joint_array.push_back(0);
        joint_array.push_back(jointAngles[7]); // 05-12
        joint_array.push_back(jointAngles[2]);
        joint_array.push_back(jointAngles[17]);
        joint_array.push_back(0);
        joint_array.push_back(jointAngles[22]);
        joint_array.push_back(0);
        joint_array.push_back(jointAngles[8]);
        joint_array.push_back(jointAngles[3]);
        joint_array.push_back(jointAngles[18]);
        joint_array.push_back(0);
        joint_array.push_back(jointAngles[23]);
        joint_array.push_back(0);
        joint_array.push_back(jointAngles[9]);
        joint_array.push_back(jointAngles[4]);
        joint_array.push_back(jointAngles[19]);
        joint_array.push_back(0);
        joint_array.push_back(jointAngles[24]);

        // std::cout << "---------------------------------------" << std::endl;

        int i = 0;
        for (auto it = joint_array.begin(); it != joint_array.end(); it += 6)
        {
            std::vector<uint8_t> joint_position(it, it + 6);
            switch(i / 6)
            {
                case 0:
                    joint_position.insert(joint_position.begin(), FRAME_PROPERTY::THUMB_POS);
                    break;
                case 1:
                    joint_position.insert(joint_position.begin(), FRAME_PROPERTY::INDEX_POS);
                    break;
                case 2:
                    joint_position.insert(joint_position.begin(), FRAME_PROPERTY::MIDDLE_POS);
                    break;
                case 3:
                    joint_position.insert(joint_position.begin(), FRAME_PROPERTY::RING_POS);
                    break;
                case 4:
                    joint_position.insert(joint_position.begin(), FRAME_PROPERTY::LITTLE_POS);
                    break;
            }
            bus->send(joint_position, handId);
            i += 6;

            // for (auto &item : joint_position)
            // {
            //     std::cout << std::hex << (int)item << " ";
            // }
        }
        // std::cout << std::endl;
    }  else {
        std::cout << "Joint position size is not 25" << std::endl;
    }
}

void LinkerHand::setJointPositionArc(const std::vector<double> &jointAngles)
{
    if (jointAngles.size() == 25) {
        static int joints_num;
        if (current_hand_type == 0) {// L25
            joints_num = 25;
        } else if (current_hand_type == 1) {// L21
            joints_num = 21;
        }
        if (handId == HAND_TYPE::LEFT) {
            setJointPositions(arc_to_range(joints_num, "left", jointAngles));
        } else if (handId == HAND_TYPE::RIGHT) {
            setJointPositions(arc_to_range(joints_num, "right", jointAngles));
        }
    } else {
        std::cout << "Joint position size is not 25" << std::endl;
    }
}

std::vector<uint8_t> LinkerHand::state_to_cmd(const std::vector<uint8_t>& l25_state) 
{
    std::vector<uint8_t> pose(25, 0.0);

    std::map<int, int> mapping = {
        {0, 10},  {1, 5},   {2, 0},   {3, 15}, {5, 20}, {7, 6},
        {8, 1},   {9, 16},  {11, 21}, {13, 7}, {14, 2}, {15, 17}, {17, 22},
        {19, 8},  {20, 3},  {21, 18}, {23, 23}, {25, 9}, {26, 4},
        {27, 19}, {29, 24}
    };

    for (const auto& pair : mapping) {
        int l25_idx = pair.first;
        int pose_idx = pair.second;

        if (l25_idx < l25_state.size() && pose_idx < pose.size()) {
            pose[pose_idx] = l25_state[l25_idx];
        }
    }
    return pose;
}

std::vector<uint8_t> LinkerHand::getCurrentStatus()
{
    bus->send({FRAME_PROPERTY::THUMB_POS}, handId);
    bus->send({FRAME_PROPERTY::INDEX_POS}, handId);
    bus->send({FRAME_PROPERTY::MIDDLE_POS}, handId);
    bus->send({FRAME_PROPERTY::RING_POS}, handId);
    bus->send({FRAME_PROPERTY::LITTLE_POS}, handId);

    std::vector<uint8_t> result_vec;
    std::vector<uint8_t> joint_position;
    if (thumb_pos.size() > 1 && index_pos.size() > 1 && middle_pos.size() > 1 && ring_pos.size() > 1 && little_pos.size() > 1)
    {
        joint_position.insert(joint_position.end(), thumb_pos.begin() + 1, thumb_pos.end());
        joint_position.insert(joint_position.end(), index_pos.begin() + 1, index_pos.end());
        joint_position.insert(joint_position.end(), middle_pos.begin() + 1, middle_pos.end());
        joint_position.insert(joint_position.end(), ring_pos.begin() + 1, ring_pos.end());
        joint_position.insert(joint_position.end(), little_pos.begin() + 1, little_pos.end());
    }

    if (joint_position.size() == 30) {
        result_vec = state_to_cmd(joint_position);
    }

    return result_vec;
}

std::vector<double> LinkerHand::getCurrentStatusArc()
{
    static int joints_num;
    if (current_hand_type == 0) {// L25
        joints_num = 25;
    } else if (current_hand_type == 1) {// L21
        joints_num = 21;
    }

    if (handId == HAND_TYPE::LEFT) {
        return range_to_arc(joints_num, "left", getCurrentStatus());
    } else if (handId == HAND_TYPE::RIGHT) {
        return range_to_arc(joints_num, "right", getCurrentStatus());
    }
    return {};
}

void LinkerHand::setSpeed(const std::vector<uint8_t> &speed)
{
    if (speed.size() == 25) {
        std::vector<uint8_t> joint_speed1(speed.begin(), speed.begin() + 5);
        std::vector<uint8_t> joint_speed2(speed.begin() + 5, speed.begin() + 10);
        std::vector<uint8_t> joint_speed3(speed.begin() + 10, speed.begin() + 15);
        std::vector<uint8_t> joint_speed4(speed.begin() + 15, speed.begin() + 20);
        std::vector<uint8_t> joint_speed5(speed.begin() + 20, speed.begin() + 25);

        joint_speed1.insert(joint_speed1.begin(), FRAME_PROPERTY::THUMB_SPEED);
        joint_speed2.insert(joint_speed2.begin(), FRAME_PROPERTY::INDEX_SPEED);
        joint_speed3.insert(joint_speed3.begin(), FRAME_PROPERTY::MIDDLE_SPEED);
        joint_speed4.insert(joint_speed4.begin(), FRAME_PROPERTY::RING_SPEED);
        joint_speed5.insert(joint_speed5.begin(), FRAME_PROPERTY::LITTLE_SPEED);

        bus->send(joint_speed1, handId);
        bus->send(joint_speed2, handId);
        bus->send(joint_speed3, handId);
        bus->send(joint_speed4, handId);
        bus->send(joint_speed5, handId);
    } else {
        std::cout << "Joint speed size is not 25" << std::endl;
    }
}

std::vector<uint8_t> LinkerHand::getSpeed() 
{
    bus->send({FRAME_PROPERTY::THUMB_SPEED}, handId);
    bus->send({FRAME_PROPERTY::INDEX_SPEED}, handId);
    bus->send({FRAME_PROPERTY::MIDDLE_SPEED}, handId);
    bus->send({FRAME_PROPERTY::RING_SPEED}, handId);
    bus->send({FRAME_PROPERTY::LITTLE_SPEED}, handId);

    std::vector<uint8_t> result_vec;
    std::vector<uint8_t> joint_speed;

    if (thumb_speed.size() > 1 && index_speed.size() > 1 && middle_speed.size() > 1 && ring_speed.size() > 1 && little_speed.size() > 1) {
        joint_speed.insert(joint_speed.end(), thumb_speed.begin() + 1, thumb_speed.end());
        joint_speed.insert(joint_speed.end(), index_speed.begin() + 1, index_speed.end());
        joint_speed.insert(joint_speed.end(), middle_speed.begin() + 1, middle_speed.end());
        joint_speed.insert(joint_speed.end(), ring_speed.begin() + 1, ring_speed.end());
        joint_speed.insert(joint_speed.end(), little_speed.begin() + 1, little_speed.end());
    }

    if (joint_speed.size() == 30) {
        result_vec = state_to_cmd(joint_speed);
    }

    return result_vec;
}

void LinkerHand::setTorque(const std::vector<uint8_t> &torque) 
{
    if (torque.size() == 25) {
        std::vector<uint8_t> joint_torque1(torque.begin(), torque.begin() + 5);
        std::vector<uint8_t> joint_torque2(torque.begin() + 5, torque.begin() + 10);
        std::vector<uint8_t> joint_torque3(torque.begin() + 10, torque.begin() + 15);
        std::vector<uint8_t> joint_torque4(torque.begin() + 15, torque.begin() + 20);
        std::vector<uint8_t> joint_torque5(torque.begin() + 20, torque.begin() + 25);

        joint_torque1.insert(joint_torque1.begin(), FRAME_PROPERTY::THUMB_TORQUE);
        joint_torque2.insert(joint_torque2.begin(), FRAME_PROPERTY::INDEX_TORQUE);
        joint_torque3.insert(joint_torque3.begin(), FRAME_PROPERTY::MIDDLE_TORQUE);
        joint_torque4.insert(joint_torque4.begin(), FRAME_PROPERTY::RING_TORQUE);
        joint_torque5.insert(joint_torque5.begin(), FRAME_PROPERTY::LITTLE_TORQUE);

        bus->send(joint_torque1, handId);
        bus->send(joint_torque2, handId);
        bus->send(joint_torque3, handId);
        bus->send(joint_torque4, handId);
        bus->send(joint_torque5, handId);
    } else {
        std::cout << "Joint torque size is not 25" << std::endl;
    }
}

std::vector<uint8_t> LinkerHand::getTorque() 
{
    bus->send({FRAME_PROPERTY::THUMB_TORQUE}, handId);
    bus->send({FRAME_PROPERTY::INDEX_TORQUE}, handId);
    bus->send({FRAME_PROPERTY::MIDDLE_TORQUE}, handId);
    bus->send({FRAME_PROPERTY::RING_TORQUE}, handId);
    bus->send({FRAME_PROPERTY::LITTLE_TORQUE}, handId);

    std::vector<uint8_t> result_vec;
    std::vector<uint8_t> joint_torque;

    if (thumb_torque.size() > 1 && index_torque.size() > 1 && middle_torque.size() > 1 && ring_torque.size() > 1 && little_torque.size() > 1)
    {
        joint_torque.insert(joint_torque.end(), thumb_torque.begin() + 1, thumb_torque.end());
        joint_torque.insert(joint_torque.end(), index_torque.begin() + 1, index_torque.end());
        joint_torque.insert(joint_torque.end(), middle_torque.begin() + 1, middle_torque.end());
        joint_torque.insert(joint_torque.end(), ring_torque.begin() + 1, ring_torque.end());
        joint_torque.insert(joint_torque.end(), little_torque.begin() + 1, little_torque.end());
    }

    if (joint_torque.size() == 30) {
        result_vec = state_to_cmd(joint_torque);
    }
    
    return result_vec;
}

std::vector<uint8_t> LinkerHand::getFaultCode() 
{
    bus->send({FRAME_PROPERTY::THUMB_FAULT}, handId);
    bus->send({FRAME_PROPERTY::INDEX_FAULT}, handId);
    bus->send({FRAME_PROPERTY::MIDDLE_FAULT}, handId);
    bus->send({FRAME_PROPERTY::RING_FAULT}, handId);
    bus->send({FRAME_PROPERTY::LITTLE_FAULT}, handId);

    std::vector<uint8_t> fault_code;
    if (thumb_fault.size() > 1 && index_fault.size() > 1 && middle_fault.size() > 1 && ring_fault.size() > 1 && little_fault.size() > 1)
    {
        fault_code.insert(fault_code.end(), thumb_fault.begin() + 1, thumb_fault.end());
        fault_code.insert(fault_code.end(), index_fault.begin() + 1, index_fault.end());
        fault_code.insert(fault_code.end(), middle_fault.begin() + 1, middle_fault.end());
        fault_code.insert(fault_code.end(), ring_fault.begin() + 1, ring_fault.end());
        fault_code.insert(fault_code.end(), little_fault.begin() + 1, little_fault.end());
    }
    return fault_code;
}

std::vector<uint8_t> LinkerHand::getTemperature()
{
    bus->send({FRAME_PROPERTY::THUMB_TEMPERATURE}, handId);
    bus->send({FRAME_PROPERTY::INDEX_TEMPERATURE}, handId);
    bus->send({FRAME_PROPERTY::MIDDLE_TEMPERATURE}, handId);
    bus->send({FRAME_PROPERTY::RING_TEMPERATURE}, handId);
    bus->send({FRAME_PROPERTY::LITTLE_TEMPERATURE}, handId);

    std::vector<uint8_t> temperature;
    if (thumb_temperature.size() > 1 && index_temperature.size() > 1 && middle_temperature.size() > 1 && ring_temperature.size() > 1 && little_temperature.size() > 1)
    {
        temperature.insert(temperature.end(), thumb_temperature.begin() + 1, thumb_temperature.end());
        temperature.insert(temperature.end(), index_temperature.begin() + 1, index_temperature.end());
        temperature.insert(temperature.end(), middle_temperature.begin() + 1, middle_temperature.end());
        temperature.insert(temperature.end(), ring_temperature.begin() + 1, ring_temperature.end());
        temperature.insert(temperature.end(), little_temperature.begin() + 1, little_temperature.end());
    }
    return temperature;
}

std::vector<std::vector<std::vector<uint8_t>>> LinkerHand::getForce() {
    bus->send({FRAME_PROPERTY::THUMB_TOUCH, 0xC6}, handId);
    bus->send({FRAME_PROPERTY::INDEX_TOUCH, 0xC6}, handId);
    bus->send({FRAME_PROPERTY::MIDDLE_TOUCH, 0xC6}, handId);
    bus->send({FRAME_PROPERTY::RING_TOUCH, 0xC6}, handId);
    bus->send({FRAME_PROPERTY::LITTLE_TOUCH, 0xC6}, handId);

    return touch_mats;
}

std::vector<std::vector<uint8_t>> LinkerHand::getForce(const int type) 
{
    std::vector<std::vector<uint8_t>> result_vec;

    if (current_hand_type == 0) // L25
    {
        result_vec.push_back(IHand::getSubVector(getNormalForce()));
        result_vec.push_back(IHand::getSubVector(getTangentialForce()));
        result_vec.push_back(IHand::getSubVector(getTangentialForceDir()));
        result_vec.push_back(IHand::getSubVector(getApproachInc()));
    }

    if (current_hand_type == 1) // L21
    {
        if (sensor_type == 0x02) { 
            bus->send({FRAME_PROPERTY::THUMB_TOUCH, 0xC6}, handId);
            bus->send({FRAME_PROPERTY::INDEX_TOUCH, 0xC6}, handId);
            bus->send({FRAME_PROPERTY::MIDDLE_TOUCH, 0xC6}, handId);
            bus->send({FRAME_PROPERTY::RING_TOUCH, 0xC6}, handId);
            bus->send({FRAME_PROPERTY::LITTLE_TOUCH, 0xC6}, handId);

            result_vec.push_back(thumb_pressure);
            result_vec.push_back(index_finger_pressure);
            result_vec.push_back(middle_finger_pressure);
            result_vec.push_back(ring_finger_pressure);
            result_vec.push_back(little_finger_pressure);
        } else {
            if (type == 0) {
                result_vec.push_back(IHand::getSubVector(getThumbForce()));
                result_vec.push_back(IHand::getSubVector(getIndexForce()));
                result_vec.push_back(IHand::getSubVector(getMiddleForce()));
                result_vec.push_back(IHand::getSubVector(getRingForce()));
                result_vec.push_back(IHand::getSubVector(getLittleForce()));
            } else {
                result_vec.push_back(IHand::getSubVector(getNormalForce()));
                result_vec.push_back(IHand::getSubVector(getTangentialForce()));
                result_vec.push_back(IHand::getSubVector(getTangentialForceDir()));
                result_vec.push_back(IHand::getSubVector(getApproachInc()));
            }
        }
    }
    
    return result_vec;
}

std::vector<uint8_t> LinkerHand::getThumbForce()
{
    bus->send({FRAME_PROPERTY::THUMB_ALL_DATA}, handId);
    return thumb_pressure;
}

std::vector<uint8_t> LinkerHand::getIndexForce()
{
    bus->send({FRAME_PROPERTY::INDEX_ALL_DATA}, handId);
    return index_finger_pressure;
}

std::vector<uint8_t> LinkerHand::getMiddleForce()
{
    bus->send({FRAME_PROPERTY::MIDDLE_ALL_DATA}, handId);
    return middle_finger_pressure;
}

std::vector<uint8_t> LinkerHand::getRingForce()
{
    bus->send({FRAME_PROPERTY::RING_ALL_DATA}, handId);
    return ring_finger_pressure;
}

std::vector<uint8_t> LinkerHand::getLittleForce()
{
    bus->send({FRAME_PROPERTY::LITTLE_ALL_DATA}, handId);
    return little_finger_pressure;
}

std::vector<uint8_t> LinkerHand::getNormalForce()
{
    bus->send({FRAME_PROPERTY::HAND_NORMAL_FORCE}, handId);
    return normal_force;
}

std::vector<uint8_t> LinkerHand::getTangentialForce()
{
    bus->send({FRAME_PROPERTY::HAND_TANGENTIAL_FORCE}, handId);
    return tangential_force;
}

std::vector<uint8_t> LinkerHand::getTangentialForceDir()
{
    bus->send({FRAME_PROPERTY::HAND_TANGENTIAL_FORCE_DIR}, handId);
    return tangential_force_dir;
}

std::vector<uint8_t> LinkerHand::getApproachInc()
{
    bus->send({FRAME_PROPERTY::HAND_APPROACH_INC}, handId);
    return approach_inc;
}

std::string LinkerHand::getVersion()
{
    bus->send({FRAME_PROPERTY::HAND_HARDWARE_VERSION}, handId);
    bus->send({FRAME_PROPERTY::HAND_SOFTWARE_VERSION}, handId);

    // getUID();
    getCommID();

    std::stringstream ss;
    if (hand_comm_id.size() >= 2)
    {
        if (hand_comm_id[1] == 0x28) {
            ss << "Hand direction: Left hand" << std::endl;
        } else if (hand_comm_id[1] == 0x27) {
            ss << "Hand direction: Right hand" << std::endl;
        } else {
            ss << "Hand direction: unknown" << std::endl;
        }
    }
    if (hand_hardware_version.size() == 5)
    {
        ss << "Software Version: " << std::hex << (int)hand_hardware_version[1] << "." << (int)hand_hardware_version[2] << "." << (int)hand_hardware_version[3] << "." << (int)hand_hardware_version[4] << std::endl;
    }
    if (hand_software_version.size() == 5)
    {
        ss << "Hardware Version: " << std::hex << (int)hand_software_version[1] << "." << (int)hand_software_version[2] << "." << (int)hand_software_version[3] << "." << (int)hand_software_version[4] << std::endl;
    }
    return ss.str();
}

void LinkerHand::setMotorEnable(const std::vector<uint8_t> &enable)
{
    std::vector<uint8_t> result = {FRAME_PROPERTY::MOTOR_ENABLE};
    result.insert(result.end(), enable.begin(), enable.end());
    bus->send(result, handId);
}

void LinkerHand::setMotorDisable(const std::vector<uint8_t> &disable)
{
    std::vector<uint8_t> result = {FRAME_PROPERTY::MOTOR_ENABLE};
    result.insert(result.end(), disable.begin(), disable.end());
    bus->send(result, handId);
}

std::vector<uint8_t> LinkerHand::getUID()
{
    bus->send({FRAME_PROPERTY::HAND_UID}, handId);
    return hand_uid;
}

std::vector<uint8_t> LinkerHand::getCommID()
{
    bus->send({FRAME_PROPERTY::HAND_COMM_ID}, handId);
    return hand_comm_id;
}

void LinkerHand::factoryReset()
{
    bus->send({FRAME_PROPERTY::HAND_FACTORY_RESET}, handId);
}

void LinkerHand::saveParameter()
{
    bus->send({FRAME_PROPERTY::HAND_SAVE_PARAMETER}, handId);
}

void LinkerHand::receiveResponse()
{
    while (running)
    {
        try
        {
            auto frame = bus->recv(handId);
            std::vector<uint8_t> data(frame.data, frame.data + frame.can_dlc);
            if (data.size() <= 0) continue;
            
            if (RECV_DEBUG) {
                static std::string hand_str;
                (current_hand_type == 0) ? hand_str = "L25" : hand_str = "L21";
                std::cout << "# " << hand_str << "-Recv " << getCurrentTime() << " | can_id:" << std::hex << frame.can_id << std::dec << " can_dlc:" << (int)frame.can_dlc << " data:";
                for (auto &can : data) std::cout << std::hex << (int)can << std::dec << " ";
                std::cout << std::endl;
            }

            uint8_t frame_property = data[0];
            std::vector<uint8_t> payload(data.begin(), data.end());

            if (frame_property >= THUMB_TOUCH && frame_property <= LITTLE_TOUCH) {
                if (data.size() == 3) {
                    if (sensor_type != 0x02) {
                        sensor_type = 0x02;
                        continue;
                    }
                }
                if (sensor_type == 0x02) {
                    if (data.size() == 8) {
                        uint8_t index = ((data[1] >> 4) + 1) * 6;
                        if (index <= 0x48) {
                            std::vector<uint8_t> payload(data.begin() + 2, data.end());

                            const std::size_t index_1 = (data[0] & 0x0F) - 1;
                            const std::size_t index_2 = (data[1] >> 4) & 0x0F;

                            if (index_1 < touch_mats.size() && index_2 < touch_mats[index_1].size() && payload.size() <= touch_mats[index_1][index_2].size()) {
                                std::memcpy(touch_mats[index_1][index_2].data(), payload.data(), payload.size());
                            }

                            // for (uint8_t i = index - 6, p = 0; i < index; ++i, ++p) {
                            //     if (data[0] == FRAME_PROPERTY::THUMB_TOUCH) thumb_pressure[i] = payload[p];
                            //     if (data[0] == FRAME_PROPERTY::INDEX_TOUCH) index_finger_pressure[i] = payload[p];
                            //     if (data[0] == FRAME_PROPERTY::MIDDLE_TOUCH) middle_finger_pressure[i] = payload[p];
                            //     if (data[0] == FRAME_PROPERTY::RING_TOUCH) ring_finger_pressure[i] = payload[p];
                            //     if (data[0] == FRAME_PROPERTY::LITTLE_TOUCH) little_finger_pressure[i] = payload[p];
                            // }
                        }
                    }
                } else {
                    // std::cout << "sensor type error !" << std::endl;
                }
                continue;
            }

            switch (frame_property)
            {
            case FRAME_PROPERTY::THUMB_POS:
                thumb_pos = payload;
                break;
            case FRAME_PROPERTY::INDEX_POS:
                index_pos = payload;
                break;
            case FRAME_PROPERTY::MIDDLE_POS:
                middle_pos = payload;
                break;
            case FRAME_PROPERTY::RING_POS:
                ring_pos = payload;
                break;
            case FRAME_PROPERTY::LITTLE_POS:
                little_pos = payload;
                break;
            case FRAME_PROPERTY::THUMB_SPEED:
                thumb_speed = payload;
                break;
            case FRAME_PROPERTY::INDEX_SPEED:
                index_speed = payload;
                break;
            case FRAME_PROPERTY::MIDDLE_SPEED:
                middle_speed = payload;
                break;
            case FRAME_PROPERTY::RING_SPEED:
                ring_speed = payload;
                break;
            case FRAME_PROPERTY::LITTLE_SPEED:
                little_speed = payload;
                break;
            case FRAME_PROPERTY::THUMB_TORQUE:
                thumb_torque = payload;
                break;
            case FRAME_PROPERTY::INDEX_TORQUE:
                index_torque = payload;
                break;
            case FRAME_PROPERTY::MIDDLE_TORQUE:
                middle_torque = payload;
                break;
            case FRAME_PROPERTY::RING_TORQUE:
                ring_torque = payload;
                break;
            case FRAME_PROPERTY::LITTLE_TORQUE:
                little_torque = payload;
                break;
            case FRAME_PROPERTY::THUMB_TEMPERATURE:
                thumb_temperature = payload;
                break;
            case FRAME_PROPERTY::INDEX_TEMPERATURE:
                index_temperature = payload;
                break;
            case FRAME_PROPERTY::MIDDLE_TEMPERATURE:
                middle_temperature = payload;
                break;
            case FRAME_PROPERTY::RING_TEMPERATURE:
                ring_temperature = payload;
                break;
            case FRAME_PROPERTY::LITTLE_TEMPERATURE:
                little_temperature = payload;
                break;
            case FRAME_PROPERTY::THUMB_FAULT:
                thumb_fault = payload;
                break;
            case FRAME_PROPERTY::INDEX_FAULT:
                index_fault = payload;
                break;
            case FRAME_PROPERTY::MIDDLE_FAULT:
                middle_fault = payload;
                break;
            case FRAME_PROPERTY::RING_FAULT:
                ring_fault = payload;
                break;
            case FRAME_PROPERTY::LITTLE_FAULT:
                little_fault = payload;
                break;
            case FRAME_PROPERTY::HAND_NORMAL_FORCE:
                normal_force = payload;
                break;
            case FRAME_PROPERTY::HAND_TANGENTIAL_FORCE:
                tangential_force = payload;
                break;
            case FRAME_PROPERTY::HAND_TANGENTIAL_FORCE_DIR:
                tangential_force_dir = payload;
                break;
            case FRAME_PROPERTY::HAND_APPROACH_INC:
                approach_inc = payload;
                break;
            case FRAME_PROPERTY::TOUCH_SENSOR_TYPE:
                if (payload.size() >= 2)
                {
                    if (payload[1] <= 0x03 && payload[1] >= 0x01)
                    {
                        sensor_type = payload[1];
                    }
                }
                break;
            case FRAME_PROPERTY::PALM_TOUCH:
                palm_force_data = payload;
                break;
            case FRAME_PROPERTY::HAND_HARDWARE_VERSION:
                hand_hardware_version = payload;
                break;
            case FRAME_PROPERTY::HAND_SOFTWARE_VERSION:
                hand_software_version = payload;
                break;
            case FRAME_PROPERTY::HAND_UID:
                hand_uid = payload;
                break;
            case FRAME_PROPERTY::HAND_COMM_ID:
                hand_comm_id = payload;
                break;
            default:
                if (RECV_DEBUG) std::cout << "L25 Unknown data type: " << std::hex << (int)frame_property << std::endl;
                continue;
            }
        }
        catch (const std::runtime_error &e)
        {
            // std::cerr << "Error receiving data: " << e.what() << std::endl;
        }
    }
}

} // namespace LinkerHandL25

