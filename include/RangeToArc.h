#ifndef LINKERHAND_RANGE_TO_ARC_H
#define LINKERHAND_RANGE_TO_ARC_H

#include <vector>
#include <stdexcept>
#include <cmath>
#include <condition_variable>

//---------------------------------------------------------------------------------------------------
// L7 L OK
const std::vector<double> l7_l_min = {0, 0, 0, 0, 0, 0, -0.52};
const std::vector<double> l7_l_max = {0.44, 1.43, 1.62, 1.62, 1.62, 1.62, 1.01};
const std::vector<int> l7_l_derict = {-1, -1, -1, -1, -1, -1, -1};
// L7 R OK (urdf后续会更改！！！)
const std::vector<double> l7_r_min = {0, -1.43, 0, 0, 0, 0, 0};
const std::vector<double> l7_r_max = {0.75, 0, 1.62, 1.62, 1.62, 1.62, 1.54};
const std::vector<int> l7_r_derict = {-1, 0, -1, -1, -1, -1, -1};
//---------------------------------------------------------------------------------------------------
// L10 L OK
const std::vector<double> l10_l_min = {0, 0, 0, 0, 0, 0, 0, -0.26, -0.26, -0.52};
const std::vector<double> l10_l_max = {1.45, 1.43, 1.62, 1.62, 1.62, 1.62, 0.26, 0, 0, 1.01};
const std::vector<int> l10_l_derict = {-1, -1, -1, -1, -1, -1, 0, -1, -1, -1};
// L10 R OK
const std::vector<double> l10_r_min = {0, 0, 0, 0, 0, 0, -0.26, 0, 0, -0.52};
const std::vector<double> l10_r_max = {0.75, 1.43, 1.62, 1.62, 1.62, 1.62, 0, 0.13, 0.26, 1.01};
const std::vector<int> l10_r_derict = {-1, -1, -1, -1, -1, -1, -1, 0, 0, -1};
//---------------------------------------------------------------------------------------------------
// L20 L OK
const std::vector<double> l20_l_min = {0, 0, 0, 0, 0, -0.297, -0.26, -0.26, -0.26, -0.26, 0.122, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const std::vector<double> l20_l_max = {0.87, 1.4, 1.4, 1.4, 1.4, 0.683, 0.26, 0.26, 0.26, 0.26, 1.78, 0, 0, 0, 0, 1.29, 1.08, 1.08, 1.08, 1.08};
const std::vector<int> l20_l_derict = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, -1};
// L20 R OK
const std::vector<double> l20_r_min = {0, 0, 0, 0, 0, -0.297, -0.26, -0.26, -0.26, -0.26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const std::vector<double> l20_r_max = {0.87, 1.4, 1.4, 1.4, 1.4, 0.683, 0.26, 0.26, 0.26, 0.26, 1.78, 0, 0, 0, 0, 1.29, 1.08, 1.08, 1.08, 1.08};
const std::vector<int> l20_r_derict = {-1, -1, -1, -1, -1, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, -1, -1, -1, -1};
//---------------------------------------------------------------------------------------------------
// L21 L OK
const std::vector<double> l21_l_min = {0, 0, 0, 0, 0, 0, 0, -0.18, -0.18, 0, -0.6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const std::vector<double> l21_l_max = {1, 1.57, 1.57, 1.57, 1.57, 1.6, 0.18, 0.18, 0.18, 0.18, 0.6, 0, 0, 0, 0, 1.57, 0, 0, 0, 0, 1.57, 1.57, 1.57, 1.57, 1.57};
const std::vector<int> l21_l_derict = {-1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, -1, -1, -1, -1};
// L21 R OK
const std::vector<double> l21_r_min = {0, 0, 0, 0, 0, 0, -0.18, -0.18, -0.18, -0.18, -0.6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const std::vector<double> l21_r_max = {1, 1.57, 1.57, 1.57, 1.57, 1.6, 0.18, 0.18, 0.18, 0.18, 0.6, 0, 0, 0, 0, 1.57, 0, 0, 0, 0, 1.57, 1.57, 1.57, 1.57, 1.57};
const std::vector<int> l21_r_derict = {-1, -1, -1, -1, -1, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, -1, -1, -1, -1};
//---------------------------------------------------------------------------------------------------
// L25 L OK
const std::vector<double> l25_l_min = {0, 0, 0, 0, 0, 0, -0.26, -0.26, -0.26, -0.26, -0.26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const std::vector<double> l25_l_max = {0.9, 1.57, 1.57, 1.57, 1.57, 1.3, 0.26, 0.26, 0.26, 0.26, 0.61, 0, 0, 0, 0, 1.57, 1.57, 1.57, 1.57, 1.57, 1.57, 1.57, 1.57, 1.57, 1.57};
const std::vector<int> l25_l_derict = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
// L25 R OK
const std::vector<double> l25_r_min = {0, 0, 0, 0, 0, 0, -0.26, -0.26, -0.26, -0.26, -0.26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const std::vector<double> l25_r_max = {0.9, 1.57, 1.57, 1.57, 1.57, 1.3, 0.26, 0.26, 0.26, 0.26, 0.61, 0, 0, 0, 0, 1.57, 1.57, 1.57, 1.57, 1.57, 1.57, 1.57, 1.57, 1.57, 1.57};
const std::vector<int> l25_r_derict = {-1, -1, -1, -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
//---------------------------------------------------------------------------------------------------


double is_within_range(double value, double min_value, double max_value);

double scale_value(double original_value, double a_min, double a_max, double b_min, double b_max);

bool should_skip_joint(int joints_type, int joint_index);

bool initialize_params(int joints_type, const std::string& left_or_right, std::vector<double>& min_limits, std::vector<double>& max_limits, std::vector<int>& derict);

std::vector<double> range_to_arc(const int &joints_type, const std::string &left_or_right, const std::vector<uint8_t> &hand_range);

std::vector<uint8_t> arc_to_range(const int &joints_type, const std::string &left_or_right, const std::vector<double> &hand_arc);

#endif // LINKERHAND_RANGE_TO_ARC_H
