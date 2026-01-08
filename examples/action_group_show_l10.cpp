// 本demo为适配球形拇指根部关节版本L10灵巧手，默认支持右手，如需左手，请将 HAND_TYPE::RIGHT 改为 HAND_TYPE::LEFT

#include <vector>
#include "LinkerHandApi.h"

using namespace std;

vector<string> joint_order = {"joint1", "joint2", "joint3", "joint4", "joint5", "joint6", "joint7", "joint8", "joint9", "joint10"};

map<string, int> hand = {
    {"joint1", 255},   //拇指根部弯曲
    {"joint2", 128},   //拇指侧摆
    {"joint3", 255},   //食指根部弯曲  
    {"joint4", 255},   //中指根部弯曲
    {"joint5", 255},   //无名指根部弯曲
    {"joint6", 255},   //小指根部弯曲
    {"joint7", 128},   //食指侧摆
    {"joint8", 128},   //中指侧摆
    {"joint9", 128},   //无名指侧摆
    {"joint10", 255}   //拇指旋转
};

int show_count = 0;
int show_count_obj = 0;
int show_step = 0;
bool running = true;

vector<uint8_t> showLeft() {
    static vector<uint8_t> position;
    show_count++;
    if (show_count >= show_count_obj) {
        show_count = 0;
        switch (show_step) {
            case 0:
                show_step++;
                show_count_obj = 100;
                hand["joint1"] = 250;
                hand["joint2"] = 250;
                hand["joint3"] = 250;
                hand["joint4"] = 250;
                hand["joint5"] = 250;
                hand["joint6"] = 250;
                hand["joint7"] = 128;
                hand["joint8"] = 128;
                hand["joint9"] = 128;
                hand["joint10"] = 250;
                break;
            case 1:
                show_step++;
                show_count_obj = 10;
                hand["joint1"] = 250;
                hand["joint2"] = 250;
                hand["joint5"] = 0;
                hand["joint6"] = 0;
                hand["joint10"] = 250;
                break;
            case 2:
                show_step++;
                show_count_obj = 30;
                hand["joint1"] = 40;
                hand["joint2"] = 240;
                hand["joint10"] = 80;
                break;
            case 3:
                show_step++;
                show_count_obj = 10;
                hand["joint7"] = 200;
                break;
            case 4:
                show_step++;
                show_count_obj = 10;
                hand["joint7"] = 50;
                break;
            case 5:
                show_step++;
                show_count_obj = 15;
                hand["joint7"] = 128;
                break;
            case 6:
                show_step++;
                show_count_obj = 2;
                hand["joint7"] = 50;
                break;
            case 7:
                show_step++;
                show_count_obj = 10;
                hand["joint7"] = 128;
                break;
            case 8:
                show_step++;
                show_count_obj = 10;
                hand["joint7"] = 50;
                break;
            case 9:
                show_step++;
                show_count_obj = 10;
                hand["joint7"] = 128;
                break;
            case 10:
                show_step++;
                show_count_obj = 15;
                hand["joint3"] = 100;
                hand["joint4"] = 100;
                break;
            case 11:
                show_step++;
                show_count_obj = 15;
                hand["joint3"] = 250;
                hand["joint4"] = 250;
                break;
            case 12:
                show_step++;
                show_count_obj = 15;
                hand["joint3"] = 100;
                hand["joint4"] = 100;
                break;
            case 13:
                show_step++;
                show_count_obj = 15;
                hand["joint1"] = 250;
                hand["joint2"] = 250;
                hand["joint3"] = 250;
                hand["joint4"] = 250;
                hand["joint5"] = 250;
                hand["joint6"] = 250;
                hand["joint7"] = 128;
                hand["joint8"] = 128;
                hand["joint9"] = 128;
                hand["joint10"] = 250;
                break;
            case 14:
                show_step++;
                show_count_obj = 40;
                hand["joint1"] = 40;
                hand["joint2"] = 240;
                hand["joint10"] = 80;
                break;
            case 15:
                show_step++;
                break;
            case 16:
                show_step++;
                show_count_obj = 30;
                hand["joint3"] = 10;
                hand["joint4"] = 10;
                hand["joint5"] = 10;
                hand["joint6"] = 10;
                break;
            case 17:
                show_step++;
                show_count_obj = 15;
                hand["joint6"] = 250;
                break;
            case 18:
                show_step++;
                show_count_obj = 15;
                hand["joint5"] = 250;
                break;
            case 19:
                show_step++;
                show_count_obj = 15;
                hand["joint4"] = 250;
                break;
            case 20:
                show_step++;
                show_count_obj = 15;
                hand["joint3"] = 250;
                break;
            case 21:
                show_step++;
                show_count_obj = 20;
                hand["joint1"] = 250;
                hand["joint2"] = 110;
                hand["joint10"] = 240;
                break;
            case 22:
                show_step++;
                show_count_obj = 20;
                hand["joint1"] = 250;
                hand["joint2"] = 10;
                hand["joint10"] = 110;
                break;
            case 23:
                show_step++;
                show_count_obj = 40;
                hand["joint1"] = 0;
                hand["joint2"] = 10;
                hand["joint10"] = 110;
                break;
            case 24:
                show_step++;
                show_count_obj = 30;
                hand["joint1"] = 0;
                hand["joint2"] = 240;
                hand["joint10"] = 110;
                break;
            case 25:
                show_step++;
                show_count_obj = 50;
                hand["joint1"] = 250;
                hand["joint2"] = 250;
                hand["joint10"] = 110;
                break;
            case 26:
                show_step++;
                show_count_obj = 10;
                hand["joint7"] = 200;
                hand["joint8"] = 200;
                hand["joint9"] = 200;
                break;
            case 27:
                show_step++;
                show_count_obj = 15;
                hand["joint7"] = 80;
                hand["joint8"] = 80;
                hand["joint9"] = 80;
                break;
            case 28:
                show_step++;
                show_count_obj = 20;
                hand["joint7"] = 128;
                hand["joint8"] = 128;
                hand["joint9"] = 128;
                break;
            case 29:
                show_step++;
                show_count_obj = 15;
                hand["joint1"] = 250;
                hand["joint2"] = 250;
                hand["joint10"] = 250;
                break;
            case 30:
                show_step++;
                break;
            case 31:
                show_step++;
                break;
            case 32:
                show_step++;
                break;
            case 33:
                show_step++;
                show_count_obj = 15;
                hand["joint3"] = 0;
                hand["joint4"] = 250;
                hand["joint5"] = 250;
                hand["joint6"] = 250;
                hand["joint1"] = 250;
                hand["joint2"] = 250;
                hand["joint10"] = 250;
                break;
            case 34:
                show_step++;
                show_count_obj = 15;
                hand["joint4"] = 0;
                break;
            case 35:
                show_step++;
                show_count_obj = 15;
                hand["joint5"] = 0;
                break;
            case 36:
                show_step++;
                show_count_obj = 15;
                hand["joint6"] = 0;
                break;
            case 37:
                show_step++;
                show_count_obj = 40;
                hand["joint1"] = 0;
                break;
            case 38:
                show_step++;
                show_count_obj = 40;
                hand["joint1"] = 250;
                hand["joint2"] = 230;
                hand["joint10"] = 250;
                break;
            case 39:
                show_step++;
                show_count_obj = 30;
                hand["joint3"] = 250;
                hand["joint6"] = 250;
                break;
            case 40:
                show_step++;
                show_count_obj = 40;
                hand["joint1"] = 10;
                hand["joint2"] = 40;
                hand["joint10"] = 60;
                break;
            case 41:
                show_step++;
                show_count_obj = 15;
                hand["joint7"] = 80;
                hand["joint9"] = 200;
                break;
            case 42:
                show_step++;
                show_count_obj = 15;
                hand["joint7"] = 200;
                hand["joint9"] = 80;
                break;
            case 43:
                show_step++;
                show_count_obj = 15;
                hand["joint7"] = 80;
                hand["joint9"] = 200;
                break;
            case 44:
                show_step++;
                show_count_obj = 15;
                hand["joint7"] = 200;
                hand["joint9"] = 80;
                break;
            case 45:
                show_step++;
                show_count_obj = 15;
                hand["joint7"] = 128;
                hand["joint9"] = 128;
                break;
            case 46:
                show_step++;
                show_count_obj = 50;
                hand["joint1"] = 250;
                hand["joint2"] = 250;
                hand["joint3"] = 250;
                hand["joint4"] = 250;
                hand["joint5"] = 250;
                hand["joint6"] = 250;
                hand["joint7"] = 128;
                hand["joint8"] = 128;
                hand["joint9"] = 128;
                hand["joint10"] = 250;
                break;
            case 47:
                show_step++;
                show_count_obj = 50;
                hand["joint1"] = 130;
                hand["joint2"] = 130;
                hand["joint3"] = 130;
                hand["joint4"] = 250;
                hand["joint5"] = 250;
                hand["joint6"] = 250;
                hand["joint10"] = 90;
                break;
            case 48:
                show_step++;
                show_count_obj = 20;
                hand["joint1"] = 250;
                hand["joint3"] = 250;
                hand["joint4"] = 120;
                break;
            case 49:
                show_step++;
                show_count_obj = 35;
                hand["joint1"] = 120;
                hand["joint4"] = 130;
                hand["joint10"] = 60;
                break;
            case 50:
                show_step++;
                show_count_obj = 30;
                hand["joint1"] = 250;
                hand["joint4"] = 250;
                hand["joint5"] = 145;
                break;
            case 51:
                show_step++;
                show_count_obj = 35;
                hand["joint1"] = 113;
                hand["joint2"] = 103;
                hand["joint5"] = 128;
                hand["joint10"] = 42;
                break;
            case 52:
                show_step++;
                show_count_obj = 30;
                hand["joint1"] = 250;
                hand["joint5"] = 250;
                break;
            case 53:
                show_step++;
                show_count_obj = 40;
                hand["joint1"] = 118;
                hand["joint2"] = 103;
                hand["joint6"] = 120;
                hand["joint10"] = 22;
                break;
            case 54:
                show_step++;
                show_count_obj = 20;
                hand["joint1"] = 250;
                hand["joint2"] = 250;
                hand["joint3"] = 250;
                hand["joint4"] = 250;
                hand["joint5"] = 250;
                hand["joint6"] = 250;
                hand["joint7"] = 128;
                hand["joint8"] = 128;
                hand["joint9"] = 128;
                hand["joint10"] = 250;
                running = false;
                break;
            default:
                break;
        }
        position.clear();
        for (const auto& joint : joint_order) { // 按照固定的顺序提取关节值
            std::cout << joint << " " << hand[joint] << std::endl;
            position.push_back(static_cast<double>(hand[joint]));
        }
    }
    return position;
}

int main() {

    // 调用API接口
    LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::LEFT);
    hand.setSpeed({100, 100, 100, 100, 100});
    hand.setTorque({200, 200, 200, 200, 200});

    while (running) {
	    std::vector<uint8_t> action = showLeft();
        hand.fingerMove(action);
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}

    return 0;
}
