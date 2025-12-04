#include "LinkerHandL20.h"

namespace LinkerHandL20
{
LinkerHand::LinkerHand(uint32_t handId, const std::string &canChannel, int baudrate) : handId(handId), running(true)
{
    bus = Communication::CanBusFactory::createCanBus(handId, canChannel, baudrate, LINKER_HAND::L20);

    touch_mats.assign(5, std::vector<std::vector<uint8_t>>(12, std::vector<uint8_t>(6, 0)));

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
    if (jointAngles.size() == 20)
    {
        // R T P Y
        std::vector<uint8_t> joint_position1(jointAngles.begin(), jointAngles.begin() + 5);
        std::vector<uint8_t> joint_position2(jointAngles.begin() + 5, jointAngles.begin() + 10);
        std::vector<uint8_t> joint_position3(jointAngles.begin() + 10, jointAngles.begin() + 15);
        std::vector<uint8_t> joint_position4(jointAngles.begin() + 15, jointAngles.begin() + 20);

        joint_position3.insert(joint_position3.begin(), FRAME_PROPERTY::JOINT_ROLL_R);
        joint_position4.insert(joint_position4.begin(), FRAME_PROPERTY::JOINT_TIP_R);
        joint_position1.insert(joint_position1.begin(), FRAME_PROPERTY::JOINT_PITCH_R);
        joint_position2.insert(joint_position2.begin(), FRAME_PROPERTY::JOINT_YAW_R);

        bus->send(joint_position3, handId);
        bus->send(joint_position4, handId);
        bus->send(joint_position1, handId);
        bus->send(joint_position2, handId);
    } else {
        std::cout << "Joint position size is not 20" << std::endl;
    }
}

void LinkerHand::setJointPositionArc(const std::vector<double> &jointAngles)
{
    if (jointAngles.size() == 20) {
        if (handId == HAND_TYPE::LEFT) {
            setJointPositions(arc_to_range(20, "left", jointAngles));
        } else if (handId == HAND_TYPE::RIGHT) {
            setJointPositions(arc_to_range(20, "right", jointAngles));
        }
    } else {
        std::cout << "Joint position size is not 20" << std::endl;
    }
}

void LinkerHand::setSpeed(const std::vector<uint8_t> &speed)
{
    if (speed.size() == 5) {
        std::vector<uint8_t> data;
        data.push_back(FRAME_PROPERTY::JOINT_SPEED_R);
        data.insert(data.end(), speed.begin(), speed.end());
        bus->send(data, handId); 
    } else {
        std::cout << "Speed size is not 5" << std::endl;
    }
}

std::vector<uint8_t> LinkerHand::getSpeed()
{
    bus->send({FRAME_PROPERTY::JOINT_SPEED_R}, handId);
    return IHand::getSubVector(motor_speed);
}

std::vector<uint8_t> LinkerHand::getCurrentStatus()
{
    // bus->send({FRAME_PROPERTY::REQUEST_DATA_RETURN}, handId); // callback 1 2 3
    
    bus->send({FRAME_PROPERTY::JOINT_PITCH_R}, handId);
    bus->send({FRAME_PROPERTY::JOINT_YAW_R}, handId);
    bus->send({FRAME_PROPERTY::JOINT_ROLL_R}, handId);
    bus->send({FRAME_PROPERTY::JOINT_TIP_R}, handId);

    std::vector<uint8_t> joint_position;
    std::vector<uint8_t> joint_position1_ = IHand::getSubVector(joint_position1);
    std::vector<uint8_t> joint_position2_ = IHand::getSubVector(joint_position2);
    std::vector<uint8_t> joint_position3_ = IHand::getSubVector(joint_position3);
    std::vector<uint8_t> joint_position4_ = IHand::getSubVector(joint_position4);

    joint_position.insert(joint_position.end(), joint_position1_.begin(), joint_position1_.end());
    joint_position.insert(joint_position.end(), joint_position2_.begin(), joint_position2_.end());
    joint_position.insert(joint_position.end(), joint_position3_.begin(), joint_position3_.end());
    joint_position.insert(joint_position.end(), joint_position4_.begin(), joint_position4_.end());

    return joint_position;
}

std::vector<double> LinkerHand::getCurrentStatusArc()
{
    if (handId == HAND_TYPE::LEFT) {
        return range_to_arc(20, "left", getCurrentStatus());
    } else if (handId == HAND_TYPE::RIGHT) {
        return range_to_arc(20, "right", getCurrentStatus());
    }
    return {};
}

std::vector<std::vector<std::vector<uint8_t>>> LinkerHand::getForce() {
    if (sensor_type == 0x02) {
        bus->send({FRAME_PROPERTY::THUMB_TOUCH, 0xC6}, handId);
        bus->send({FRAME_PROPERTY::INDEX_TOUCH, 0xC6}, handId);
        bus->send({FRAME_PROPERTY::MIDDLE_TOUCH, 0xC6}, handId);
        bus->send({FRAME_PROPERTY::RING_TOUCH, 0xC6}, handId);
        bus->send({FRAME_PROPERTY::LITTLE_TOUCH, 0xC6}, handId);
    } else if (sensor_type == 0x01) {
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
    } else {
        return {};
    }
    return touch_mats;
}

#if 0
std::vector<uint8_t> LinkerHand::getThumbForce()
{
    bus->send({FRAME_PROPERTY::THUMB_ALL_DATA}, handId);
    return IHand::getSubVector(thumb_pressure);
}

std::vector<uint8_t> LinkerHand::getIndexForce()
{
    bus->send({FRAME_PROPERTY::INDEX_FINGER_ALL_DATA}, handId);
    return IHand::getSubVector(index_finger_pressure);
}

std::vector<uint8_t> LinkerHand::getMiddleForce()
{
    bus->send({FRAME_PROPERTY::MIDDLE_FINGER_ALL_DATA}, handId);
    return IHand::getSubVector(middle_finger_pressure);
}

std::vector<uint8_t> LinkerHand::getRingForce()
{
    bus->send({FRAME_PROPERTY::RING_FINGER_ALL_DATA}, handId);
    return IHand::getSubVector(ring_finger_pressure);
}

std::vector<uint8_t> LinkerHand::getLittleForce()
{
    bus->send({FRAME_PROPERTY::LITTLE_FINGER_ALL_DATA}, handId);
    return IHand::getSubVector(little_finger_pressure);
}
#endif

// 获取五指法向压力
std::vector<uint8_t> LinkerHand::getNormalForce()
{
    bus->send({FRAME_PROPERTY::HAND_NORMAL_FORCE}, handId);
    return normal_force;
}

// 获取五指切向压力
std::vector<uint8_t> LinkerHand::getTangentialForce()
{
    bus->send({FRAME_PROPERTY::HAND_TANGENTIAL_FORCE}, handId);
    return tangential_force;
}

// 获取五指切向方向
std::vector<uint8_t> LinkerHand::getTangentialForceDir()
{
    bus->send({FRAME_PROPERTY::HAND_TANGENTIAL_FORCE_DIR}, handId);
    return tangential_force_dir;
}

// 获取五指接近感应
std::vector<uint8_t> LinkerHand::getApproachInc()
{
    bus->send({FRAME_PROPERTY::HAND_APPROACH_INC}, handId);
    return approach_inc;
}

std::vector<uint8_t> LinkerHand::getFaultCode()
{
    bus->send({FRAME_PROPERTY::JOINT_FAULT_R}, handId);
    return IHand::getSubVector(motor_fault_code);
}

std::vector<uint8_t> LinkerHand::getCurrent()
{
    bus->send({FRAME_PROPERTY::JOINT_CURRENT_R}, handId);
    return IHand::getSubVector(motor_current);
}

void LinkerHand::clearFaultCode(const std::vector<uint8_t> &code)
{
    if (code.size() == 5) {
        std::vector<uint8_t> data;
        data.push_back(FRAME_PROPERTY::JOINT_FAULT_R);
        data.insert(data.end(), code.begin(), code.end());
        bus->send(data, handId);
    } else {
        std::cout << "clearFaultCode size is not 5" << std::endl;
    }
}

void LinkerHand::setCurrent(const std::vector<uint8_t> &current)
{
    if (current.size() == 5) {
        std::vector<uint8_t> data;
        data.push_back(FRAME_PROPERTY::JOINT_CURRENT_R);
        data.insert(data.end(), current.begin(), current.end());
        bus->send(data, handId);
    } else {
        std::cout << "Current size is not 5" << std::endl;
    }
}

std::string LinkerHand::getVersion()
{
    bus->send({FRAME_PROPERTY::HAND_HARDWARE_VERSION}, handId);
    bus->send({FRAME_PROPERTY::HAND_SOFTWARE_VERSION}, handId);
    bus->send({FRAME_PROPERTY::HAND_COMM_ID}, handId);

    // IHand::getSubVector(hand_software_version, hand_hardware_version);

    std::stringstream ss;

    if (hand_software_version.size() > 0)
    {
        if (hand_comm_id[1] == 0x27) {
            ss << "Hand direction: Right hand" << std::endl;
        } else if (hand_comm_id[1] == 0x28) {
            ss << "Hand direction: Left hand" << std::endl;
        }
        ss << "Software Version: " << ((int)(hand_software_version[0] >> 4) + (int)(hand_software_version[0] & 0x0F) / 10.0) << std::endl;
        ss << "Hardware Version: " << ((int)(hand_hardware_version[0] >> 4) + (int)(hand_hardware_version[0] & 0x0F) / 10.0) << std::endl;
    }
    return ss.str();
}

std::vector<uint8_t> LinkerHand::getUID()
{
    bus->send({FRAME_PROPERTY::HAND_UID}, handId);
    return IHand::getSubVector(hand_uid);
}

std::vector<uint8_t> LinkerHand::getRotorLockCount()
{
    bus->send({FRAME_PROPERTY::ROTOR_LOCK_COUNT}, handId);
    return IHand::getSubVector(rotor_lock_count);
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
                std::cout << "# L20-Recv " << getCurrentTime() << " | can_id:" << std::hex << frame.can_id << std::dec << " can_dlc:" << (int)frame.can_dlc << " data:";
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
                    }
                    continue;
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
                case FRAME_PROPERTY::JOINT_PITCH_R:
                    joint_position1 = payload;
                    break;
                case FRAME_PROPERTY::JOINT_YAW_R:
                    joint_position2 = payload;
                    break;
                case FRAME_PROPERTY::JOINT_ROLL_R:
                    joint_position3 = payload;
                    break;
                case FRAME_PROPERTY::JOINT_TIP_R:
                    joint_position4 = payload;
                    break;
                case FRAME_PROPERTY::JOINT_FAULT_R:
                    motor_fault_code = payload;
                    break;
                case FRAME_PROPERTY::JOINT_CURRENT_R:
                    motor_current = payload;
                    break;
                case FRAME_PROPERTY::JOINT_SPEED_R:
                    motor_speed = payload;
                    break;
                case FRAME_PROPERTY::TOUCH_SENSOR_TYPE:
                    if (payload.size() > 1) sensor_type = payload[1];
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
                case FRAME_PROPERTY::HAND_HARDWARE_VERSION:
                    hand_hardware_version = payload;
                    break;
                case FRAME_PROPERTY::HAND_SOFTWARE_VERSION:
                    hand_software_version = payload;
                    break;
                case FRAME_PROPERTY::HAND_COMM_ID:
                    hand_comm_id = payload;
                    break;
                case FRAME_PROPERTY::HAND_UID:
                    hand_uid = payload;
                    break;
                case FRAME_PROPERTY::ROTOR_LOCK_COUNT:
                    rotor_lock_count = payload;
                    break;
                default:
                    if (RECV_DEBUG) std::cout << "L20 Unknown data type: " << std::hex << (int)frame_property << std::endl;
                    continue;
            }
        }
        catch (const std::runtime_error &e)
        {
            // std::cerr << "Error receiving data: " << e.what() << std::endl;
        }
    }
}

} // namespace LinkerHandL20

