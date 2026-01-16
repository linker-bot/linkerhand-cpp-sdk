# 测试说明

本目录包含 LinkerHand-CPP-SDK 的单元测试。本文档介绍如何构建、运行测试，以及如何添加新测试。

## 目录

- [测试结构](#测试结构)
- [构建测试](#构建测试)
- [运行测试](#运行测试)
- [测试覆盖范围](#测试覆盖范围)
- [测试用例详解](#测试用例详解)
- [添加新测试](#添加新测试)
- [测试最佳实践](#测试最佳实践)
- [故障排查](#故障排查)
- [更多信息](#更多信息)

---

## 测试结构

```
tests/
├── unit/                    # 单元测试
│   ├── test_Common.cpp      # Common.h 的测试
│   ├── test_RangeToArc.cpp  # RangeToArc.h 的测试
│   ├── test_IHand_Utils.cpp # IHand 工具函数的测试
│   └── test_CanFrame.cpp    # CanFrame 的测试
├── CMakeLists.txt           # 测试构建配置
└── README.md                # 本文件
```

---

## 构建测试

### 方法一：使用 CMake 构建（推荐）

```bash
# 在项目根目录
mkdir build && cd build
cmake .. -DBUILD_TESTING=ON
make
```

**说明**:
- `-DBUILD_TESTING=ON` 启用测试构建（默认已启用）
- CMake 会自动下载 Google Test 框架（如果未安装）
- 确保网络连接正常，以便下载依赖
- **注意**: 项目要求 CMake 3.15 或更高版本

### 方法二：禁用测试

如果不想构建测试：

```bash
cmake .. -DBUILD_TESTING=OFF
```

或者不设置 `BUILD_TESTING` 选项（默认为 ON）。

### 方法三：仅构建测试（不构建主项目）

```bash
cd build
cmake .. -DBUILD_TESTING=ON
make test_Common test_RangeToArc test_IHand_Utils test_CanFrame
```

---

## 运行测试

### 运行所有测试

```bash
cd build
ctest
```

**输出示例**:
```
Test project /path/to/build
    Start 1: test_Common
1/4 Test #1: test_Common ...................   Passed    0.01 sec
    Start 2: test_RangeToArc
2/4 Test #2: test_RangeToArc ...............   Passed    0.02 sec
    Start 3: test_IHand_Utils
3/4 Test #3: test_IHand_Utils ..............   Passed    0.01 sec
    Start 4: test_CanFrame
4/4 Test #4: test_CanFrame ..................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 4
```

### 运行所有测试（显示详细输出）

```bash
cd build
ctest --output-on-failure
```

**说明**: 仅当测试失败时显示详细输出，成功时不显示。

### 运行所有测试（详细模式）

```bash
cd build
ctest -V
# 或
ctest --verbose
```

**说明**: 显示所有测试的详细输出，包括成功的测试。

### 运行特定测试

```bash
cd build
# 运行 test_Common
ctest -R test_Common

# 运行多个测试（使用正则表达式）
ctest -R "test_Common|test_RangeToArc"
```

### 并行运行测试

```bash
cd build
# 使用 4 个并行任务
ctest -j4

# 使用所有可用 CPU 核心
ctest -j$(nproc)
```

**说明**: 并行运行可以加快测试速度，特别是测试较多时。

### 直接运行测试可执行文件

```bash
cd build
./test_Common
./test_RangeToArc
./test_IHand_Utils
./test_CanFrame
```

**说明**: 直接运行可执行文件可以看到更详细的输出，包括 Google Test 的详细日志。

### 使用便捷目标

```bash
cd build
make run_tests
```

**说明**: 如果 CMakeLists.txt 中定义了 `run_tests` 目标，可以使用此命令。

---

## 测试覆盖范围

### test_Common.cpp

**测试内容**: Common.h 中的枚举和类型定义

**测试用例**:
- ✅ `LINKER_HAND` 枚举值测试
  - 验证所有型号枚举值（O6, L6, L7, L10, L20, L21, L25）
- ✅ `HAND_TYPE` 枚举值测试
  - 验证左手和右手枚举值
- ✅ `COMM_TYPE` 枚举值测试
  - 验证所有通信类型枚举值
- ✅ `Common` 命名空间版本号测试
  - 验证版本号定义
- ✅ 参数化测试
  - 使用 Google Test 参数化测试测试多个值

**示例测试**:
```cpp
TEST(CommonTest, LinkerHandEnum) {
    EXPECT_EQ(LINKER_HAND::O6, 0);
    EXPECT_EQ(LINKER_HAND::L6, 1);
    // ...
}
```

---

### test_RangeToArc.cpp

**测试内容**: RangeToArc.h 中的转换函数

**测试用例**:
- ✅ `is_within_range()` 函数测试
  - 测试值是否在有效范围内
  - 边界值测试
- ✅ `scale_value()` 函数测试
  - 测试值缩放功能
  - 边界情况测试
- ✅ `should_skip_joint()` 函数测试
  - 测试关节跳过逻辑
- ✅ `initialize_params()` 函数测试
  - 测试参数初始化
- ✅ `range_to_arc()` 函数测试
  - 测试从 0-255 范围转换为弧度
  - 不同型号的转换测试
- ✅ `arc_to_range()` 函数测试
  - 测试从弧度转换为 0-255 范围
- ✅ 往返转换测试
  - 测试 `range_to_arc()` 和 `arc_to_range()` 的往返一致性
- ✅ 边界值测试
  - 测试最小值（0）、最大值（255）、中间值（128）
- ✅ 不同型号测试
  - 测试 L10、L20、L21、L25 等不同型号的转换

**示例测试**:
```cpp
TEST(RangeToArcTest, RangeToArcConversion) {
    double arc = range_to_arc(128, LINKER_HAND::L10, 0);
    EXPECT_NEAR(arc, expected_value, 0.001);
}
```

---

### test_IHand_Utils.cpp

**测试内容**: IHand 工具函数

**测试用例**:
- ✅ `getSubVector()` 单参数版本测试
  - 测试从向量中提取子向量
  - 边界情况测试（空向量、越界等）
- ✅ `getSubVector()` 双参数版本测试
  - 测试指定起始和结束位置的子向量提取
- ✅ `getCurrentTime()` 函数测试
  - 测试时间获取功能
  - 时间格式验证
- ✅ 边界情况测试
  - 空向量、单元素向量、越界访问等

**示例测试**:
```cpp
TEST(IHandUtilsTest, GetSubVector) {
    std::vector<uint8_t> vec = {1, 2, 3, 4, 5};
    auto sub = getSubVector(vec, 1, 3);
    EXPECT_EQ(sub.size(), 2);
    EXPECT_EQ(sub[0], 2);
    EXPECT_EQ(sub[1], 3);
}
```

---

### test_CanFrame.cpp

**测试内容**: CanFrame.h 中的 CAN 帧结构

**测试用例**:
- ✅ CANFrame 结构体测试
  - 结构体大小和布局验证
- ✅ 数据设置和读取测试
  - 测试数据的设置和读取功能
- ✅ 边界值测试
  - 测试最小值和最大值
- ✅ ID 范围测试
  - 测试有效的 CAN ID 范围
- ✅ 数据复制测试
  - 测试数据复制和移动语义

**示例测试**:
```cpp
TEST(CanFrameTest, DataSetting) {
    CanFrame frame;
    frame.setData({0x01, 0x02, 0x03, 0x04});
    auto data = frame.getData();
    EXPECT_EQ(data.size(), 4);
    EXPECT_EQ(data[0], 0x01);
}
```

---

## 测试用例详解

### 测试命名规范

测试用例使用 Google Test 的命名规范：
- 测试套件名：描述被测试的模块（如 `CommonTest`、`RangeToArcTest`）
- 测试用例名：描述具体的测试内容（如 `LinkerHandEnum`、`RangeToArcConversion`）

### 测试类型

1. **单元测试**: 测试单个函数或类的功能
2. **参数化测试**: 使用 `TEST_P` 测试多个输入值
3. **边界测试**: 测试边界值和极端情况
4. **集成测试**: 测试多个组件的交互（当前未包含）

### 测试覆盖率

当前测试覆盖：
- ✅ 核心工具函数（Common、RangeToArc、IHand Utils）
- ✅ 数据结构（CanFrame）
- ⚠️ API 函数（部分覆盖，需要硬件支持）
- ❌ 通信层（需要硬件支持）
- ❌ 端到端测试（需要硬件支持）

---

## 添加新测试

### 步骤 1: 创建测试文件

在 `tests/unit/` 目录下创建新的测试文件，例如 `test_NewFeature.cpp`:

```cpp
#include <gtest/gtest.h>
#include "NewFeature.h"

TEST(NewFeatureTest, BasicFunctionality) {
    // 测试代码
    EXPECT_EQ(expected, actual);
}
```

### 命名规范说明

在编写测试时，请注意以下命名规范：

1. **使用新的命名空间**（推荐）:
```cpp
#include "LinkerHandL10.h"
#include "HandFactory.h"

TEST(HandTest, L10HandCreation) {
    using namespace linkerhand;
    auto hand = factory::HandFactory::createHand(
        LINKER_HAND::L10,
        HAND_TYPE::RIGHT,
        COMM_CAN_0
    );
    // 测试代码
}
```

2. **向后兼容的旧命名**（可用但不推荐）:
```cpp
#include "LinkerHandL10.h"

TEST(HandTest, L10HandCreation) {
    // 旧命名仍然可用，但建议使用新命名
    LinkerHandL10::LinkerHand hand(0x27, "can0", 1000000);
    // 测试代码
}
```

3. **接口测试**:
```cpp
#include "IHand.h"

TEST(IHandTest, InterfaceTest) {
    // 使用正确的命名空间
    class TestHand : public linkerhand::hand::IHand {
        // 实现接口
    };
    // 测试代码
}
```

更多关于命名规范的详细信息，请参考 [命名规范改进文档](../docs/NAMING_IMPROVEMENTS.md)。

### 步骤 2: 更新 CMakeLists.txt

在 `tests/CMakeLists.txt` 的 `TEST_SOURCES` 列表中添加新文件：

```cmake
set(TEST_SOURCES
    unit/test_Common.cpp
    unit/test_RangeToArc.cpp
    unit/test_IHand_Utils.cpp
    unit/test_CanFrame.cpp
    unit/test_NewFeature.cpp  # 添加新文件
)
```

### 步骤 3: 配置链接（如需要）

如果测试需要链接 SDK 库或其他库，在 CMakeLists.txt 中添加：

```cmake
target_link_libraries(test_NewFeature 
    ${LINKER_HAND_LIB}  # 如果需要链接 SDK 库
    pthread
)
```

### 步骤 4: 重新构建和运行

```bash
cd build
cmake ..
make
ctest -R test_NewFeature
```

---

## 测试最佳实践

### 1. 测试独立性

每个测试应该独立，不依赖其他测试：

```cpp
// ✅ 好的做法
TEST(MyTest, Test1) {
    // 独立的测试代码
}

// ❌ 不好的做法
TEST(MyTest, Test2) {
    // 依赖 Test1 的结果
}
```

### 2. 描述性命名

使用描述性的测试名称：

```cpp
// ✅ 好的做法
TEST(RangeToArcTest, ConvertsZeroToMinimumArc)

// ❌ 不好的做法
TEST(RangeToArcTest, Test1)
```

### 3. 测试多种情况

测试正常情况、边界情况和错误情况：

```cpp
TEST(MyTest, NormalCase) {
    // 正常情况
}

TEST(MyTest, BoundaryCase) {
    // 边界值（0, 255, 最大值等）
}

TEST(MyTest, ErrorCase) {
    // 错误情况（无效输入等）
}
```

### 4. 使用参数化测试

对于需要测试多个输入值的情况，使用参数化测试：

```cpp
class RangeToArcParamTest : public ::testing::TestWithParam<int> {};

TEST_P(RangeToArcParamTest, ConvertsCorrectly) {
    int input = GetParam();
    // 测试代码
}

INSTANTIATE_TEST_SUITE_P(
    RangeValues,
    RangeToArcParamTest,
    ::testing::Values(0, 128, 255)
);
```

### 5. 保持测试简洁

测试代码应该简洁、可读：

```cpp
// ✅ 好的做法
TEST(MyTest, SimpleTest) {
    int result = function(5);
    EXPECT_EQ(result, 10);
}

// ❌ 不好的做法
TEST(MyTest, ComplexTest) {
    // 100 行复杂的测试代码
}
```

### 6. 使用适当的断言

选择合适的断言：

```cpp
EXPECT_EQ(a, b);      // 相等
EXPECT_NE(a, b);      // 不相等
EXPECT_NEAR(a, b, 0.001);  // 浮点数近似相等
EXPECT_TRUE(condition);    // 为真
EXPECT_FALSE(condition);   // 为假
EXPECT_THROW(statement, ExceptionType);  // 抛出异常
```

---

## 故障排查

### 问题 1: 找不到 Google Test

**错误信息**:
```
Could not find gtest
```

**解决方案**:
1. CMake 会自动下载 Google Test，确保网络连接正常
2. 如果网络有问题，可以手动安装 Google Test：
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libgtest-dev
   ```
3. 检查 CMake 版本（需要 3.15+）

---

### 问题 2: 链接错误

**错误信息**:
```
undefined reference to `...`
```

**解决方案**:
1. **检查 SDK 库是否正确找到**:
   ```bash
   ls -la lib/x86_64/liblinkerhand_cpp_sdk.so
   ```

2. **检查测试是否需要链接 SDK 库**:
   - 如果测试需要链接 SDK 库，在 `tests/CMakeLists.txt` 中添加：
     ```cmake
     target_link_libraries(test_name ${LINKER_HAND_LIB} pthread)
     ```

3. **检查库路径**:
   ```cmake
   find_library(LINKER_HAND_LIB
       NAMES linkerhand_cpp_sdk
       PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../lib/x86_64
   )
   ```

---

### 问题 3: 测试失败

**错误信息**:
```
Test failed: ...
```

**解决方案**:
1. **查看详细输出**:
   ```bash
   ctest --output-on-failure
   ```

2. **直接运行测试可执行文件**:
   ```bash
   ./test_Common
   ```
   这样可以查看更详细的错误信息。

3. **检查测试代码**:
   - 验证测试逻辑是否正确
   - 检查期望值是否正确

4. **检查 SDK 库版本**:
   - 确保使用的 SDK 库版本与测试代码匹配
   - 检查是否有 API 变更

5. **检查环境**:
   - 确保测试环境正确配置
   - 检查依赖项是否完整

---

### 问题 4: 测试运行缓慢

**可能原因**:
- 测试数量多
- 测试包含耗时操作
- 系统负载高

**解决方案**:
1. **并行运行测试**:
   ```bash
   ctest -j4
   ```

2. **优化测试代码**:
   - 减少不必要的操作
   - 使用 mock 对象替代真实对象

3. **选择性运行测试**:
   ```bash
   ctest -R test_Common  # 只运行特定测试
   ```

---

### 问题 5: CMake 配置错误

**错误信息**:
```
CMake Error: ...
```

**解决方案**:
1. **清理构建目录**:
   ```bash
   rm -rf build
   mkdir build && cd build
   ```

2. **检查 CMake 版本**:
   ```bash
   cmake --version  # 需要 3.15+
   ```

3. **检查 CMakeLists.txt 语法**:
   - 确保语法正确
   - 检查变量名和路径

---

## 更多信息

### 相关文档

- **详细测试文档**: [测试文档](../docs/TESTING.md)（如果存在）
- **API 参考**: [API 参考文档](../docs/API-Reference.md)
- **故障排查**: [故障排查指南](../docs/TROUBLESHOOTING.md)
- **常见问题**: [常见问题解答](../docs/FAQ.md)
- **命名规范**: [命名规范改进文档](../docs/NAMING_IMPROVEMENTS.md)

### Google Test 文档

- [Google Test 官方文档](https://google.github.io/googletest/)
- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [Google Test Advanced](https://google.github.io/googletest/advanced.html)

### 测试工具

- **ctest**: CMake 测试运行器
- **gtest**: Google Test 框架
- **valgrind**: 内存检查工具（可选）
- **gcov/lcov**: 代码覆盖率工具（可选）

---

## 贡献测试

欢迎贡献新的测试用例！在添加新测试时，请：

1. 遵循测试最佳实践
2. 确保测试独立且可重复
3. 添加适当的注释和文档
4. 运行所有测试确保没有破坏现有功能
5. 提交 Pull Request

如有问题，请参考 [故障排查指南](../docs/TROUBLESHOOTING.md) 或联系技术支持。