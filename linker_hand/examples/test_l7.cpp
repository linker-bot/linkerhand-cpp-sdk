// main.cpp
#include "LinkerHandApi.h"

int main() {

    // 调用API接口
    LinkerHandApi hand(LINKER_HAND::L21, HAND_TYPE::LEFT);

    // 获取版本信息
    //std::cout << hand.getVersion() << std::endl;

    hand.setTorque({0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0});
    hand.setSpeed({0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0});

    // hand.setTorque({255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255});
    // hand.setSpeed({255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255});

    std::vector<uint8_t> torque = hand.getTorque();
    std::cout << "torque size:" << torque.size() << std::endl;
    for (int i = 0; i < torque.size(); i++) {
        std::cout << std::hex << (int)torque[i] << std::dec << std::endl;
    }

	for (size_t i = 0; i < 3; i++) {
		std::vector<uint8_t> speed = hand.getSpeed();
		std::cout << "speed size:" << speed.size() << std::endl;
		for (int i = 0; i < speed.size(); i++) {
		    std::cout << std::hex << (int)speed[i] << std::dec << std::endl;
		}
    }


    
    // 握拳
    std::vector<uint8_t> pos2_1 = {230, 0, 0, 15, 5, 250, 55, 80, 210, 202, 85, 0, 0, 0, 0, 80, 0, 40, 35, 5, 250, 0, 5, 0, 0};
    std::vector<uint8_t> pos2_2 = {230, 0, 0, 15, 5, 42, 55, 80, 210, 202, 85, 0, 0, 0, 0, 90, 0, 40, 35, 5, 120, 0, 5, 0, 0};
    hand.fingerMove(pos2_1);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    hand.fingerMove(pos2_2);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 张开
    std::vector<uint8_t> open_pose = {75, 255, 255, 255, 255, 176, 51, 80, 210, 202, 202, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
    hand.fingerMove(open_pose);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    
    std::this_thread::sleep_for(std::chrono::seconds(3));

    return 0;
}
