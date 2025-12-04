#include "ModbusLinkerHandL10.h"
#if USE_RMAN
namespace ModbusLinkerHandL10
{

LinkerHand::LinkerHand(uint32_t handId): handId(handId), bus(handId) {

}

LinkerHand::~LinkerHand() {

}

void LinkerHand::setJointPositions(const std::vector<uint8_t> &data) {
	if(data.size() == 10) {
		bus.send(data, handId, 0, 10);
	} else {
		printf("Error: Input vector must contain exactly 5 values.\n");
	}
}

void LinkerHand::setJointPositionArc(const std::vector<double> &data) {
	if (handId == HAND_TYPE::LEFT) {
		setJointPositions(arc_to_range(10, "left", data));
	} else if (handId == HAND_TYPE::RIGHT) {
		setJointPositions(arc_to_range(10, "right", data));
	}
}

void LinkerHand::setTorque(const std::vector<uint8_t> &data) {
	if(data.size() == 10) {
		bus.send(data, handId, 15, 10);
	} else {
		printf("Error: Input vector must contain exactly 5 values.\n");
	}
}

void LinkerHand::setSpeed(const std::vector<uint8_t> &data) {
	if(data.size() == 5) {
		bus.send(data, handId, 10, 5);
	} else {
		printf("Error: Input vector must contain exactly 5 values.\n");
	}
}

std::vector<uint8_t> LinkerHand::getSpeed() {
	return bus.recv(handId, 10, 5);
}

std::vector<uint8_t> LinkerHand::getTorque() {
	return bus.recv(handId, 15, 10);
}

std::vector<uint8_t> LinkerHand::getCurrentStatus() {
	return bus.recv(handId, 0, 10);
}
std::vector<double> LinkerHand::getCurrentStatusArc() {
	if (handId == HAND_TYPE::LEFT) {
        return range_to_arc(10, "left", getCurrentStatus());
    } else if (handId == HAND_TYPE::RIGHT) {
        return range_to_arc(10, "right", getCurrentStatus());
    }
    return {};
}

void LinkerHand::receiveResponse()
{
	
}

}
#endif
