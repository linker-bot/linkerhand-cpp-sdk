#include "LinkerHandApi.h"

LinkerHandApi::LinkerHandApi(const LINKER_HAND &handJoint, const HAND_TYPE &handType, const COMM_TYPE commType) : handJoint_(handJoint), handType_(handType)
{
    hand = HandFactory::createHand(handJoint, handType, commType);
}

LinkerHandApi::~LinkerHandApi()
{
}

void LinkerHandApi::setTorque(const std::vector<uint8_t> &torque)
{
    hand->setTorque(torque);
}

void LinkerHandApi::setSpeed(const std::vector<uint8_t> &speed)
{
    hand->setSpeed(speed);
}

void LinkerHandApi::fingerMove(const std::vector<uint8_t> &pose)
{
    hand->setJointPositions(pose);
}

void LinkerHandApi::fingerMoveArc(const std::vector<double> &pose)
{
    hand->setJointPositionArc(pose);
}

std::vector<uint8_t> LinkerHandApi::getSpeed()
{
    return hand->getSpeed();
}

std::vector<uint8_t> LinkerHandApi::getState()
{
    return hand->getCurrentStatus();
}

std::vector<double> LinkerHandApi::getStateArc()
{
    return hand->getCurrentStatusArc();
}

std::vector<std::vector<std::vector<uint8_t>>> LinkerHandApi::getForce()
{
    return hand->getForce();
}

std::string LinkerHandApi::getVersion()
{
    return hand->getVersion();
}

std::vector<uint8_t> LinkerHandApi::getTorque()
{
    return hand->getTorque();
}

std::vector<uint8_t> LinkerHandApi::getTemperature()
{
    return hand->getTemperature();
}

std::vector<uint8_t> LinkerHandApi::getFaultCode()
{
    return hand->getFaultCode();
}

std::vector<uint8_t> LinkerHandApi::getCurrent()
{
    return hand->getCurrent();
}

void LinkerHandApi::setCurrent(const std::vector<uint8_t> &current)
{
    if (handJoint_ == LINKER_HAND::L20) {
        hand->setCurrent(current);
    } else {
        std::cout << "LinkerHandApi : Currently only supports L20 !" << std::endl;
    }
}

void LinkerHandApi::clearFaultCode(const std::vector<uint8_t> &torque)
{
    if (handJoint_ == LINKER_HAND::L20) {
        hand->clearFaultCode(torque);
    } else {
        std::cout << "LinkerHandApi : Currently only supports L20 !" << std::endl;
    }
}

void LinkerHandApi::setEnable(const std::vector<uint8_t> &enable)
{
    if (handJoint_ == LINKER_HAND::L25) {
        hand->setMotorEnable(enable);
    } else {
        std::cout << "LinkerHandApi : Currently only supports L25 !" << std::endl;
    }
}

void LinkerHandApi::setDisable(const std::vector<uint8_t> &disable)
{
    if (handJoint_ == LINKER_HAND::L25) {
        hand->setMotorDisable(disable);
    } else {
        std::cout << "LinkerHandApi : Currently only supports L25 !" << std::endl;
    }
}

