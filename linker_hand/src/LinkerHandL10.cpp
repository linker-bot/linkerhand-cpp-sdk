#include "LinkerHandL10.h"

namespace LinkerHandL10
{
LinkerHand::LinkerHand(uint32_t handId, const std::string &canChannel, int baudrate)
    : handId(handId), running(true)
{
    bus = Communication::CanBusFactory::createCanBus(handId, canChannel, baudrate, LINKER_HAND::L10);

    touch_mats.assign(5, std::vector<std::vector<uint8_t>>(12, std::vector<uint8_t>(6, 0)));

    receiveThread = std::thread(&LinkerHand::receiveResponse, this);
    
    bus->send({FRAME_PROPERTY::LINKER_HAND_VERSION}, handId);
    // bus->send({TOUCH_SENSOR_TYPE}, handId);
    bus->send({FRAME_PROPERTY::THUMB_TOUCH}, handId);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
    if (jointAngles.size() == 10) { 
        uint8_t send_data[8];
        send_data[0] = FRAME_PROPERTY::JOINT_POSITION_RCO;
        send_data[1] = jointAngles[0];
        send_data[2] = jointAngles[1];
        send_data[3] = jointAngles[2];
        send_data[4] = jointAngles[3];
        send_data[5] = jointAngles[4];
        send_data[6] = jointAngles[5];
        std::vector<uint8_t> position_data_vector(send_data, send_data + 7);
        bus->send(position_data_vector, handId);

        send_data[0] = FRAME_PROPERTY::JOINT_POSITION2_RCO;
        send_data[1] = jointAngles[6];
        send_data[2] = jointAngles[7];
        send_data[3] = jointAngles[8];
        send_data[4] = jointAngles[9];
        std::vector<uint8_t> position2_data_vector(send_data, send_data + 5);
        bus->send(position2_data_vector, handId);

    } else {
        std::cout << "Joint position size is not 10" << std::endl;
    }
}

void LinkerHand::setJointPositionArc(const std::vector<double> &jointAngles)
{
    if (jointAngles.size() == 10) {
        if (handId == HAND_TYPE::LEFT) {
            setJointPositions(arc_to_range(10, "left", jointAngles));
        } else if (handId == HAND_TYPE::RIGHT) {
            setJointPositions(arc_to_range(10, "right", jointAngles));
        }
    } else {
        std::cout << "Joint position size is not 10" << std::endl;
    }
}

std::vector<uint8_t> LinkerHand::getCurrentStatus()
{
    if (Common::current_hand_version > 1.4f) {
        bus->send({FRAME_PROPERTY::JOINT_POSITION_RCO}, handId);
        bus->send({FRAME_PROPERTY::JOINT_POSITION2_RCO}, handId);
    }
    return IHand::getSubVector(joint_position, joint_position2);
}

std::vector<double> LinkerHand::getCurrentStatusArc()
{
    if (handId == HAND_TYPE::LEFT) {
        return range_to_arc(10, "left", getCurrentStatus());
    } else if (handId == HAND_TYPE::RIGHT) {
        return range_to_arc(10, "right", getCurrentStatus());
    }
    return {};
}

std::string LinkerHand::getVersion()
{
    bus->send({FRAME_PROPERTY::LINKER_HAND_VERSION}, handId);
    
    std::stringstream ss;

    if (version.size() > 0) 
    {
        ss << "freedom: " << (int)version[1] << std::endl;
        ss << "Robot version: " << (int)version[2] << std::endl;
        ss << "Version Number: " << (int)version[3] << std::endl;
        if (version[4] == 0x52) {
            ss << "Hand direction: Right hand" << std::endl;
        } else if (version[4] == 0x4C) {
            ss << "Hand direction: Left hand" << std::endl;
        }
        ss << "Software Version: " << ((int)(version[5] >> 4) + (int)(version[5] & 0x0F) / 10.0) << std::endl;
        ss << "Hardware Version: " << ((int)(version[6] >> 4) + (int)(version[6] & 0x0F) / 10.0) << std::endl;
    }

    return ss.str();
}

void LinkerHand::setTorque(const std::vector<uint8_t> &torque)
{
    std::vector<uint8_t> result;
    if (torque.size() == 5) {
        result = {FRAME_PROPERTY::TORQUE_LIMIT};
        result.insert(result.end(), torque.begin(), torque.end());
        max_torque = result;

        bus->send(result, handId);
    } else if (torque.size() == 10) {
        result = {FRAME_PROPERTY::TORQUE_LIMIT};
        result.insert(result.end(), torque.begin(), torque.end() - 5);
        bus->send(result, handId);
        max_torque = result;
        // std::cout << "result_1:" << std::endl;
        // for (int i = 0; i < result.size(); i++) {
        //     std::cout << std::hex << (int)result[i] << std::dec << std::endl;
        // }

        result = {FRAME_PROPERTY::TORQUE_LIMIT_2};
        result.insert(result.end(), torque.begin() + 5, torque.end());
        bus->send(result, handId);
        max_torque_2 = result;
        // std::cout << "result_2:" << std::endl;
        // for (int i = 0; i < result.size(); i++) {
        //     std::cout << std::hex << (int)result[i] << std::dec << std::endl;
        // }
    } else {
        std::cout << "Torque size is not 5" << std::endl;
    }
}

void LinkerHand::setSpeed(const std::vector<uint8_t> &speed)
{
    std::vector<uint8_t> result;
    if (speed.size() == 5) {
        result = {FRAME_PROPERTY::JOINT_SPEED};
        result.insert(result.end(), speed.begin(), speed.end());

        joint_speed = result;
        bus->send(result, handId);
    } else if (speed.size() == 10) {
        result = {FRAME_PROPERTY::JOINT_SPEED};
        result.insert(result.end(), speed.begin(), speed.end() - 5);
        bus->send(result, handId);
        joint_speed = result;

        result = {FRAME_PROPERTY::JOINT_SPEED_2};
        result.insert(result.end(), speed.begin() + 5, speed.end());
        bus->send(result, handId);
        joint_speed_2 = result;
        
    } else {
        std::cout << "Joint speed size is not 5" << std::endl;
    }
}

std::vector<uint8_t> LinkerHand::getSpeed()
{
    if (Common::current_hand_version > 1.4f) {
        bus->send({FRAME_PROPERTY::JOINT_SPEED}, handId);
        bus->send({FRAME_PROPERTY::JOINT_SPEED_2}, handId);
    }
    return IHand::getSubVector(joint_speed, joint_speed_2);
}

std::vector<uint8_t> LinkerHand::getTorque()
{
    if (Common::current_hand_version > 1.4f) {
        bus->send({FRAME_PROPERTY::TORQUE_LIMIT}, handId);
        bus->send({FRAME_PROPERTY::TORQUE_LIMIT_2}, handId);
    }
    return IHand::getSubVector(max_torque, max_torque_2);
}

std::vector<std::vector<std::vector<uint8_t>>> LinkerHand::getForce() {
    if (sensor_type == 0x02) {
        bus->send({FRAME_PROPERTY::THUMB_TOUCH, 0xC6}, handId);
        bus->send({FRAME_PROPERTY::INDEX_TOUCH, 0xC6}, handId);
        bus->send({FRAME_PROPERTY::MIDDLE_TOUCH, 0xC6}, handId);
        bus->send({FRAME_PROPERTY::RING_TOUCH, 0xC6}, handId);
        bus->send({FRAME_PROPERTY::LITTLE_TOUCH, 0xC6}, handId);
    } else {
        bus->send({FRAME_PROPERTY::THUMB_ALL_DATA}, handId);
        bus->send({FRAME_PROPERTY::INDEX_FINGER_ALL_DATA}, handId);
        bus->send({FRAME_PROPERTY::MIDDLE_FINGER_ALL_DATA}, handId);
        bus->send({FRAME_PROPERTY::RING_FINGER_ALL_DATA}, handId);
        bus->send({FRAME_PROPERTY::LITTLE_FINGER_ALL_DATA}, handId);

        if (touch_mats.size() == 5) {
            touch_mats.resize(5);
            for (auto& mat : touch_mats) {
                mat.resize(1);
                for (auto& row : mat) {
                    row.resize(4);
                }
            }
        }
    }
    return touch_mats;
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

std::vector<uint8_t> LinkerHand::getTemperature()
{
    bus->send({FRAME_PROPERTY::MOTOR_TEMPERATURE_1}, handId);
    bus->send({FRAME_PROPERTY::MOTOR_TEMPERATURE_2}, handId);

    return IHand::getSubVector(motorTemperature_1, motorTemperature_2);
}

std::vector<uint8_t> LinkerHand::getFaultCode()
{
    bus->send({FRAME_PROPERTY::MOTOR_FAULT_CODE_1}, handId);
    bus->send({FRAME_PROPERTY::MOTOR_FAULT_CODE_2}, handId);
    
    return IHand::getSubVector(motorFaultCode_1, motorFaultCode_2);
}

std::vector<uint8_t> LinkerHand::getCurrent()
{
    return {0,0,0,0,0,0,0,0,0,0};
}

std::vector<uint8_t> LinkerHand::requestAllStatus()
{
    bus->send({FRAME_PROPERTY::REQUEST_DATA_RETURN}, handId);
    return {0,0,0,0,0,0,0,0,0,0};
}

void LinkerHand::receiveResponse()
{
    while (running)
    {
        try {
            auto frame = bus->recv(handId);
            std::vector<uint8_t> data(frame.data, frame.data + frame.can_dlc);
            if (data.size() <= 0) continue;
            
            if (RECV_DEBUG) {
                std::cout << "# L10-Recv " << getCurrentTime() << " | can_id:" << std::hex << frame.can_id << std::dec << " can_dlc:" << (int)frame.can_dlc << " data:";
                for (auto &can : data) std::cout << std::hex << (int)can << std::dec << " ";
                std::cout << std::endl;
            }

            uint8_t frame_property = data[0];
            std::vector<uint8_t> payload(data.begin(), data.end());

            if (frame_property >= THUMB_TOUCH && frame_property <= LITTLE_TOUCH) 
            {
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

            switch(frame_property) {
                case FRAME_PROPERTY::JOINT_POSITION_RCO:
                    joint_position = payload;
                    break;
                case FRAME_PROPERTY::TORQUE_LIMIT:
                    max_torque = payload;
                    break;
                case FRAME_PROPERTY::TORQUE_LIMIT_2:
                    max_torque_2 = payload;
                    break;
                case FRAME_PROPERTY::JOINT_POSITION2_RCO:
                    joint_position2 = payload;
                    break;
                case FRAME_PROPERTY::JOINT_SPEED:
                    joint_speed = payload;
                    break;
                case FRAME_PROPERTY::JOINT_SPEED_2:
                    joint_speed_2 = payload;
                    break;
                case FRAME_PROPERTY::THUMB_ALL_DATA:
                    if (payload.size() > 1) std::memcpy(touch_mats[0][0].data(), payload.data() + 1, payload.size() - 1);
                    break;
                case FRAME_PROPERTY::INDEX_FINGER_ALL_DATA:
                    if (payload.size() > 1) std::memcpy(touch_mats[1][0].data(), payload.data() + 1, payload.size() - 1);
                    break;
                case FRAME_PROPERTY::MIDDLE_FINGER_ALL_DATA:
                    if (payload.size() > 1) std::memcpy(touch_mats[2][0].data(), payload.data() + 1, payload.size() - 1);
                    break;
                case FRAME_PROPERTY::RING_FINGER_ALL_DATA:
                    if (payload.size() > 1) std::memcpy(touch_mats[3][0].data(), payload.data() + 1, payload.size() - 1);
                    break;
                case FRAME_PROPERTY::LITTLE_FINGER_ALL_DATA:
                    if (payload.size() > 1) std::memcpy(touch_mats[4][0].data(), payload.data() + 1, payload.size() - 1);
                    break;
                case FRAME_PROPERTY::TOUCH_SENSOR_TYPE:
                    if (payload.size() >= 2) {
                        if (payload[1] <= 0x03 && payload[1] >= 0x01) {
                            sensor_type = payload[1];
                        }
                    }
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
                case FRAME_PROPERTY::LINKER_HAND_VERSION:
                    version = payload;
                    Common::current_hand_version = (float)((int)(version[5] >> 4) + (int)(version[5] & 0x0F) / 10.0);
                    break;
                case FRAME_PROPERTY::MOTOR_TEMPERATURE_1:
                    motorTemperature_1 = payload;
                    break;
                case FRAME_PROPERTY::MOTOR_TEMPERATURE_2:
                    motorTemperature_2 = payload;
                    break;
                case FRAME_PROPERTY::MOTOR_FAULT_CODE_1:
                    motorFaultCode_1 = payload;
                    break;
                case FRAME_PROPERTY::MOTOR_FAULT_CODE_2:
                    motorFaultCode_2 = payload;
                    break;
                case FRAME_PROPERTY::REQUEST_DATA_RETURN:
                    break;
                case 0x45:
                    // can0  027   [1]  05
                    // can0  027   [6]  05 00 00 00 00 00
                    // can0  028   [1]  05
                    // can0  028   [5]  45 52 52 4F 52
                    break;
                default:
                    if (RECV_DEBUG) std::cout << "L10 Unknown data type: " << std::hex << (int)frame_property << std::endl;
                    continue;
            }
        }
        catch (const std::runtime_error &e)
        {
            // std::cerr << "Error receiving data: " << e.what() << std::endl;
        }
    }
}
}

