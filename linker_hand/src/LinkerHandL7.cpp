#include "LinkerHandL7.h"

namespace LinkerHandL7
{
LinkerHand::LinkerHand(uint32_t handId, const std::string &canChannel, int baudrate)
    : handId(handId), running(true)
{
    bus = Communication::CanBusFactory::createCanBus(handId, canChannel, baudrate, LINKER_HAND::L7);

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
    if (jointAngles.size() == 7)
    { 
        int JointEffort = 100; // Effort 0～255
        int JointVelocity = 180; // Velocity 0～255

        uint8_t send_data[8];

        send_data[0] = static_cast<uint8_t>(JointEffort);
        send_data[1] = static_cast<uint8_t>(JointEffort);
        send_data[2] = static_cast<uint8_t>(JointEffort);
        send_data[3] = static_cast<uint8_t>(JointEffort);
        send_data[4] = static_cast<uint8_t>(JointEffort);
        std::vector<uint8_t> pressure_data_vector(send_data, send_data + 5);
        // setTorque(pressure_data_vector);

        send_data[0] = static_cast<uint8_t>(JointVelocity);
        send_data[1] = static_cast<uint8_t>(JointVelocity);
        send_data[2] = static_cast<uint8_t>(JointVelocity);
        send_data[3] = static_cast<uint8_t>(JointVelocity);
        send_data[4] = static_cast<uint8_t>(JointVelocity);
        send_data[5] = static_cast<uint8_t>(JointVelocity);
        send_data[6] = static_cast<uint8_t>(JointVelocity);
        std::vector<uint8_t> speed_data_vector(send_data, send_data + 7);
        // setSpeed(speed_data_vector);

        std::vector<uint8_t> result(jointAngles.begin(), jointAngles.end());
        result.insert(result.begin(), FRAME_PROPERTY::JOINT_POSITION);
        bus->send(result, handId);
    } else {
        std::cout << "Joint position size is not 10" << std::endl;
    }
}

void LinkerHand::setJointPositionArc(const std::vector<double> &jointAngles)
{
    if (jointAngles.size() == 7) {
        if (handId == HAND_TYPE::LEFT) {
            setJointPositions(arc_to_range(7, "left", jointAngles));
        } else if (handId == HAND_TYPE::RIGHT) {
            setJointPositions(arc_to_range(7, "right", jointAngles));
        }
    } else {
        std::cout << "Joint position size is not 10" << std::endl;
    }
}

std::vector<uint8_t> LinkerHand::getCurrentStatus()
{
    bus->send({FRAME_PROPERTY::JOINT_POSITION}, handId);
    return IHand::getSubVector(joint_position);
}

std::vector<double> LinkerHand::getCurrentStatusArc()
{
    if (handId == HAND_TYPE::LEFT) {
        return range_to_arc(7, "left", getCurrentStatus());
    } else if (handId == HAND_TYPE::RIGHT) {
        return range_to_arc(7, "right", getCurrentStatus());
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
    if (torque.size() == 7) {
        std::vector<uint8_t> result = {FRAME_PROPERTY::TORQUE_LIMIT};
        result.insert(result.end(), torque.begin(), torque.end());
        bus->send(result, handId);
    } else {
        std::cout << "Torque size is not 7" << std::endl;
    }
}

void LinkerHand::setSpeed(const std::vector<uint8_t> &speed)
{
    if (speed.size() == 7) {
        std::vector<uint8_t> result = {FRAME_PROPERTY::JOINT_SPEED};
        result.insert(result.end(), speed.begin(), speed.end());

        joint_speed = result;
        bus->send(result, handId);
    } else {
        std::cout << "Joint speed size is not 7" << std::endl;
    }
}

std::vector<uint8_t> LinkerHand::getSpeed()
{
    bus->send({FRAME_PROPERTY::JOINT_SPEED}, handId);
    return IHand::getSubVector(joint_speed);
}

std::vector<uint8_t> LinkerHand::getTorque()
{
    bus->send({FRAME_PROPERTY::TORQUE_LIMIT}, handId);
    return IHand::getSubVector(max_torque);
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
    bus->send({FRAME_PROPERTY::MOTOR_TEMPERATURE}, handId);
    return IHand::getSubVector(motorTemperature_1);
}

std::vector<uint8_t> LinkerHand::getFaultCode()
{
    bus->send({FRAME_PROPERTY::MOTOR_FAULT_CODE}, handId);
    return IHand::getSubVector(motorFaultCode_1);
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
                std::cout << "# L7-Recv " << getCurrentTime() << " | can_id:" << std::hex << frame.can_id << std::dec << " can_dlc:" << (int)frame.can_dlc << " data:";
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
                case FRAME_PROPERTY::JOINT_POSITION:
                    joint_position = payload;
                    break;
                case FRAME_PROPERTY::TORQUE_LIMIT:
                    max_torque = payload;
                    break;
                case FRAME_PROPERTY::JOINT_SPEED:
                    joint_speed = payload;
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
                case FRAME_PROPERTY::MOTOR_TEMPERATURE:
                    motorTemperature_1 = payload;
                    break;
                case FRAME_PROPERTY::MOTOR_FAULT_CODE:
                    motorFaultCode_1 = payload;
                    break;
                    case FRAME_PROPERTY::TOUCH_SENSOR_TYPE:
                    if (payload.size() >= 2) {
                        if (payload[1] <= 0x03 && payload[1] >= 0x01) {
                            sensor_type = payload[1];
                        }
                    }
                    break;
                default:
                    if (RECV_DEBUG) std::cout << "L7 Unknown data type: " << std::hex << (int)frame_property << std::endl;
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

