#include "LinkerHandL6.h"

namespace LinkerHandL6
{
LinkerHand::LinkerHand(uint32_t handId, const std::string &canChannel, int baudrate)
    : handId(handId), running(true)
{
    bus = Communication::CanBusFactory::createCanBus(handId, canChannel, baudrate, LINKER_HAND::L6);

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
    if (jointAngles.size() == 6) {
        std::vector<uint8_t> result(jointAngles.begin(), jointAngles.end());
        result.insert(result.begin(), FRAME_PROPERTY::JOINT_POSITION);
        bus->send(result, handId);
    } else {
        std::cout << "Joint position size is not 6" << std::endl;
    }
}

void LinkerHand::setJointPositionArc(const std::vector<double> &jointAngles)
{
    if (jointAngles.size() == 6) {
        if (handId == HAND_TYPE::LEFT) {
            setJointPositions(arc_to_range(6, "left", jointAngles));
        } else if (handId == HAND_TYPE::RIGHT) {
            setJointPositions(arc_to_range(6, "right", jointAngles));
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
        return range_to_arc(6, "left", getCurrentStatus());
    } else if (handId == HAND_TYPE::RIGHT) {
        return range_to_arc(6, "right", getCurrentStatus());
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

std::vector<uint8_t> LinkerHand::getTemperature()
{
    bus->send({FRAME_PROPERTY::MOTOR_TEMPERATURE}, handId);
    return IHand::getSubVector(motorTemperature_1);
}

std::vector<std::vector<std::vector<uint8_t>>> LinkerHand::getForce() {
    if (sensor_type == 0x02) {
        bus->send({FRAME_PROPERTY::THUMB_TOUCH, 0xC6}, handId);
        bus->send({FRAME_PROPERTY::INDEX_TOUCH, 0xC6}, handId);
        bus->send({FRAME_PROPERTY::MIDDLE_TOUCH, 0xC6}, handId);
        bus->send({FRAME_PROPERTY::RING_TOUCH, 0xC6}, handId);
        bus->send({FRAME_PROPERTY::LITTLE_TOUCH, 0xC6}, handId);
    }
    return touch_mats;
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
                case FRAME_PROPERTY::LINKER_HAND_VERSION:
                    version = payload;
        			Common::current_hand_version = (float)((int)(version[5] >> 4) + (int)(version[5] & 0x0F) / 10.0);
                    break;
                case FRAME_PROPERTY::MOTOR_TEMPERATURE:
                    motorTemperature_1 = payload;
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

