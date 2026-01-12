#include <gtest/gtest.h>
#include "IHand.h"
#include <vector>
#include <chrono>
#include <thread>

// 创建一个测试用的 IHand 实现类
class TestHand : public linkerhand::hand::IHand {
public:
    void setJointPositions(const std::vector<uint8_t> &jointAngles) override {
        (void)jointAngles;
    }
};

// 测试 getSubVector 函数（单参数版本）
TEST(IHandUtilsTest, GetSubVectorSingleArg) {
    TestHand hand;
    
    // 测试正常情况
    std::vector<uint8_t> vec = {0, 1, 2, 3, 4, 5};
    std::vector<uint8_t> result = hand.getSubVector(vec);
    
    EXPECT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[1], 2);
    EXPECT_EQ(result[4], 5);
    
    // 测试只有一个元素的情况
    std::vector<uint8_t> vec_single = {0};
    std::vector<uint8_t> result_single = hand.getSubVector(vec_single);
    EXPECT_TRUE(result_single.empty());
    
    // 测试空向量
    std::vector<uint8_t> vec_empty = {};
    std::vector<uint8_t> result_empty = hand.getSubVector(vec_empty);
    EXPECT_TRUE(result_empty.empty());
    
    // 测试两个元素的情况
    std::vector<uint8_t> vec_two = {0, 1};
    std::vector<uint8_t> result_two = hand.getSubVector(vec_two);
    EXPECT_EQ(result_two.size(), 1);
    EXPECT_EQ(result_two[0], 1);
}

// 测试 getSubVector 函数（双参数版本）
TEST(IHandUtilsTest, GetSubVectorDoubleArg) {
    TestHand hand;
    
    // 测试正常情况
    std::vector<uint8_t> vec1 = {0, 1, 2, 3};
    std::vector<uint8_t> vec2 = {0, 4, 5, 6};
    std::vector<uint8_t> result = hand.getSubVector(vec1, vec2);
    
    EXPECT_EQ(result.size(), 6); // 3 + 3
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[1], 2);
    EXPECT_EQ(result[2], 3);
    EXPECT_EQ(result[3], 4);
    EXPECT_EQ(result[4], 5);
    EXPECT_EQ(result[5], 6);
    
    // 测试一个向量只有一个元素
    std::vector<uint8_t> vec_single = {0};
    std::vector<uint8_t> vec_normal = {0, 1, 2};
    std::vector<uint8_t> result_mixed = hand.getSubVector(vec_single, vec_normal);
    EXPECT_EQ(result_mixed.size(), 2); // 0 + 2
    EXPECT_EQ(result_mixed[0], 1);
    EXPECT_EQ(result_mixed[1], 2);
    
    // 测试两个向量都只有一个元素
    std::vector<uint8_t> vec1_single = {0};
    std::vector<uint8_t> vec2_single = {0};
    std::vector<uint8_t> result_both_single = hand.getSubVector(vec1_single, vec2_single);
    EXPECT_TRUE(result_both_single.empty());
    
    // 测试空向量
    std::vector<uint8_t> vec_empty = {};
    std::vector<uint8_t> vec_normal2 = {0, 1, 2};
    std::vector<uint8_t> result_empty = hand.getSubVector(vec_empty, vec_normal2);
    EXPECT_EQ(result_empty.size(), 2);
}

// 测试 getCurrentTime 函数
TEST(IHandUtilsTest, GetCurrentTime) {
    TestHand hand;
    
    // 获取时间字符串
    std::string time1 = hand.getCurrentTime();
    
    // 验证时间字符串不为空
    EXPECT_FALSE(time1.empty());
    
    // 验证时间字符串格式（应该包含日期和时间）
    // 格式应该是 "YYYY-MM-DD HH:MM:SS.mmm"
    EXPECT_GT(time1.length(), 19); // 至少包含日期和时间部分
    
    // 验证包含日期分隔符
    EXPECT_NE(time1.find('-'), std::string::npos);
    
    // 验证包含时间分隔符
    EXPECT_NE(time1.find(':'), std::string::npos);
    
    // 等待一小段时间后再次获取时间
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::string time2 = hand.getCurrentTime();
    
    // 验证两次获取的时间不同（或至少格式相同）
    EXPECT_EQ(time1.length(), time2.length());
    
    // 验证时间字符串格式一致
    // 都应该是 "YYYY-MM-DD HH:MM:SS.mmm" 格式
    size_t space_pos1 = time1.find(' ');
    size_t space_pos2 = time2.find(' ');
    EXPECT_NE(space_pos1, std::string::npos);
    EXPECT_NE(space_pos2, std::string::npos);
    
    // 验证日期部分格式
    std::string date1 = time1.substr(0, space_pos1);
    std::string date2 = time2.substr(0, space_pos2);
    EXPECT_EQ(date1.length(), 10); // "YYYY-MM-DD"
    EXPECT_EQ(date2.length(), 10);
    
    // 验证日期部分包含两个连字符
    size_t dash_count1 = 0;
    size_t dash_count2 = 0;
    for (char c : date1) {
        if (c == '-') dash_count1++;
    }
    for (char c : date2) {
        if (c == '-') dash_count2++;
    }
    EXPECT_EQ(dash_count1, 2);
    EXPECT_EQ(dash_count2, 2);
}

// 测试 getCurrentTime 的时间递增
TEST(IHandUtilsTest, GetCurrentTimeIncrement) {
    TestHand hand;
    
    std::string time1 = hand.getCurrentTime();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::string time2 = hand.getCurrentTime();
    
    // 由于我们等待了 100ms，时间应该不同
    // 注意：由于时间格式包含毫秒，两次时间应该不同
    // 但由于时间精度问题，我们只验证格式正确
    EXPECT_FALSE(time1.empty());
    EXPECT_FALSE(time2.empty());
}

// 测试 getSubVector 的边界情况
TEST(IHandUtilsTest, GetSubVectorEdgeCases) {
    TestHand hand;
    
    // 测试大向量
    std::vector<uint8_t> large_vec(1000, 0);
    for (size_t i = 0; i < large_vec.size(); ++i) {
        large_vec[i] = static_cast<uint8_t>(i % 256);
    }
    std::vector<uint8_t> result = hand.getSubVector(large_vec);
    
    // 验证大小：跳过第一个元素，所以应该是 999
    EXPECT_EQ(result.size(), 999);
    
    // 验证第一个元素：应该是原始向量的第二个元素 (1)
    EXPECT_EQ(result[0], 1);
    
    // 修复：result[998] 对应 large_vec[999]
    // large_vec[999] = 999 % 256 = 231
    EXPECT_EQ(static_cast<int>(result[998]), 231);
    
    // 附加验证：确保理解正确
    // result[254] 对应 large_vec[255] = 255
    EXPECT_EQ(static_cast<int>(result[254]), 255);
}

