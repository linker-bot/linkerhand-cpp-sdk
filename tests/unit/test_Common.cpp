#include <gtest/gtest.h>
#include "Common.h"

// 测试 LINKER_HAND 枚举值
TEST(CommonTest, LinkerHandEnumValues) {
    // 验证枚举值是否正确定义
    EXPECT_EQ(static_cast<int>(O6), 0);
    EXPECT_EQ(static_cast<int>(L6), 1);
    EXPECT_EQ(static_cast<int>(L7), 2);
    EXPECT_EQ(static_cast<int>(L10), 3);
    EXPECT_EQ(static_cast<int>(L20), 4);
    EXPECT_EQ(static_cast<int>(L21), 5);
    EXPECT_EQ(static_cast<int>(L25), 6);
}

// 测试 HAND_TYPE 枚举值
TEST(CommonTest, HandTypeEnumValues) {
    EXPECT_EQ(static_cast<uint32_t>(LEFT), 0x28);
    EXPECT_EQ(static_cast<uint32_t>(RIGHT), 0x27);
    
    // 验证左右手类型不同
    EXPECT_NE(LEFT, RIGHT);
}

// 测试 COMM_TYPE 枚举值
TEST(CommonTest, CommTypeEnumValues) {
    EXPECT_EQ(static_cast<int>(COMM_CAN_0), 0);
    EXPECT_EQ(static_cast<int>(COMM_CAN_1), 1);
    EXPECT_EQ(static_cast<int>(COMM_MODBUS), 2);
    EXPECT_EQ(static_cast<int>(COMM_ETHERCAT), 3);
}

// 测试 Common 命名空间中的版本号
TEST(CommonTest, CurrentHandVersion) {
    EXPECT_GT(linkerhand::Common::current_hand_version, 0.0f);
    EXPECT_LE(linkerhand::Common::current_hand_version, 10.0f); // 合理的版本号范围
}

// 测试枚举值的类型转换
TEST(CommonTest, EnumTypeConversions) {
    LINKER_HAND hand = L10;
    HAND_TYPE type = RIGHT;
    COMM_TYPE comm = COMM_CAN_0;
    
    EXPECT_EQ(hand, L10);
    EXPECT_EQ(type, RIGHT);
    EXPECT_EQ(comm, COMM_CAN_0);
}

// 参数化测试：测试所有 LINKER_HAND 类型
class LinkerHandTypeTest : public ::testing::TestWithParam<LINKER_HAND> {};

TEST_P(LinkerHandTypeTest, ValidHandTypes) {
    LINKER_HAND hand = GetParam();
    // 验证是有效的枚举值
    EXPECT_GE(static_cast<int>(hand), 0);
    EXPECT_LE(static_cast<int>(hand), 6);
}

INSTANTIATE_TEST_SUITE_P(
    AllHandTypes,
    LinkerHandTypeTest,
    ::testing::Values(O6, L6, L7, L10, L20, L21, L25)
);

// 参数化测试：测试所有 COMM_TYPE 类型
class CommTypeTest : public ::testing::TestWithParam<COMM_TYPE> {};

TEST_P(CommTypeTest, ValidCommTypes) {
    COMM_TYPE comm = GetParam();
    // 验证是有效的枚举值
    EXPECT_GE(static_cast<int>(comm), 0);
    EXPECT_LE(static_cast<int>(comm), 3);
}

INSTANTIATE_TEST_SUITE_P(
    AllCommTypes,
    CommTypeTest,
    ::testing::Values(COMM_CAN_0, COMM_CAN_1, COMM_MODBUS, COMM_ETHERCAT)
);

