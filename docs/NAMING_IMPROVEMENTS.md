# LinkerHand C++ SDK 命名规范改进指南

## 概述

本文档描述了 LinkerHand C++ SDK 的命名规范改进，这些改进旨在提高代码一致性、可维护性和可读性。

## 改进内容

### 1. 统一命名空间结构

所有手型号实现类已迁移到统一的 `linkerhand::hand` 命名空间：

**改进前：**
```cpp
namespace LinkerHandL10 {
    class LinkerHand : public linkerhand::hand::IHand { ... };
}
```

**改进后：**
```cpp
namespace linkerhand {
namespace hand {
    class L10Hand : public IHand { ... };
} // namespace hand
} // namespace linkerhand
```

### 2. 类名改进

不同型号的手现在使用不同的类名，便于区分：

| 型号 | 旧类名 | 新类名 | 命名空间 |
|------|--------|--------|---------|
| L6/O6 | `LinkerHandL6::LinkerHand` | `linkerhand::hand::L6Hand` | `linkerhand::hand` |
| L7 | `LinkerHandL7::LinkerHand` | `linkerhand::hand::L7Hand` | `linkerhand::hand` |
| L10 | `LinkerHandL10::LinkerHand` | `linkerhand::hand::L10Hand` | `linkerhand::hand` |
| L20 | `LinkerHandL20::LinkerHand` | `linkerhand::hand::L20Hand` | `linkerhand::hand` |
| L25/L21 | `LinkerHandL25::LinkerHand` | `linkerhand::hand::L25Hand` | `linkerhand::hand` |
| Modbus L10 | `ModbusLinkerHandL10::LinkerHand` | `linkerhand::hand::ModbusL10Hand` | `linkerhand::hand` |

### 3. 帧属性枚举重命名

各型号的帧属性枚举已重命名，使用更清晰的命名：

| 型号 | 旧枚举名 | 新枚举名 |
|------|---------|---------|
| L6/O6 | `LinkerHandL6::FRAME_PROPERTY` | `linkerhand::hand::L6FrameProperty` |
| L7 | `LinkerHandL7::FRAME_PROPERTY` | `linkerhand::hand::L7FrameProperty` |
| L10 | `LinkerHandL10::FRAME_PROPERTY` | `linkerhand::hand::L10FrameProperty` |
| L20 | `LinkerHandL20::FRAME_PROPERTY` | `linkerhand::hand::L20FrameProperty` |
| L25/L21 | `LinkerHandL25::FRAME_PROPERTY` | `linkerhand::hand::L25FrameProperty` |

### 4. 参数命名修正

修正了错误的参数命名：

**改进前（错误）**：
```cpp
LinkerHandApi(const LINKER_HAND &handJoint, ...);
LINKER_HAND handJoint_;  // 这是手型号，不是关节
```

**改进后（正确）**：
```cpp
LinkerHandApi(const LINKER_HAND &handModel, ...);
LINKER_HAND handModel_;  // 手型号（L6, L7, L10, L20, L21, L25 等）
```

**说明**：
- `handJoint` 命名不准确，因为这是手型号（Model），不是关节（Joint）
- 已统一修正为 `handModel`，更准确地表示手型号

## 向后兼容性

为了保持向后兼容，所有旧的命名空间和类名都通过 `using` 别名保留：

```cpp
// 向后兼容：在旧命名空间中提供别名
namespace LinkerHandL10 {
    using FRAME_PROPERTY = linkerhand::hand::L10FrameProperty;
    using LinkerHand = linkerhand::hand::L10Hand;
} // namespace LinkerHandL10
```

这意味着现有代码可以继续使用旧的命名，无需立即修改。

## 使用示例

### 使用新命名（推荐）

```cpp
#include "LinkerHandL10.h"
#include "HandFactory.h"

// 使用新的命名空间和类名
using namespace linkerhand;

auto hand = factory::HandFactory::createHand(
    LINKER_HAND::L10,
    HAND_TYPE::RIGHT,
    COMM_CAN_0
);
// hand 的类型是 std::unique_ptr<hand::IHand>
// 实际类型是 hand::L10Hand
```

### 使用旧命名（向后兼容）

```cpp
#include "LinkerHandL10.h"
#include "HandFactory.h"

// 仍然可以使用旧的命名空间
LinkerHandL10::LinkerHand hand(0x27, "can0", 1000000);
```

## 迁移建议

### 阶段一：保持兼容（当前）

- ✅ 所有新命名已实现
- ✅ 向后兼容别名已添加
- ✅ 现有代码无需修改即可继续工作

### 阶段二：逐步迁移（推荐）

1. **新代码使用新命名**：所有新编写的代码应使用新的命名空间和类名
2. **更新文档和示例**：逐步更新文档和示例代码使用新命名
3. **代码审查**：在代码审查中鼓励使用新命名

### 阶段三：完全迁移（未来）

在未来版本中，可以考虑：
- 移除向后兼容别名（需要主版本号升级）
- 更新所有内部代码使用新命名
- 更新所有文档和示例

## 优势

1. **一致性**：所有型号使用统一的命名空间结构
2. **可读性**：类名直接反映型号（`L10Hand` vs `LinkerHand`）
3. **可维护性**：统一的命名空间便于管理和查找
4. **扩展性**：新型号可以轻松添加到同一命名空间

## 注意事项

1. **工厂类**：`HandFactory` 已更新为使用新类名，但返回类型仍然是 `IHand` 接口
2. **头文件**：头文件名暂时保持不变，未来可以考虑重命名
3. **枚举**：帧属性枚举已重命名，但枚举值保持不变

## 相关文件

- `include/LinkerHandL6.h` - L6/O6 型号实现
- `include/LinkerHandL7.h` - L7 型号实现
- `include/LinkerHandL10.h` - L10 型号实现
- `include/LinkerHandL20.h` - L20 型号实现
- `include/LinkerHandL25.h` - L25/L21 型号实现
- `include/ModbusLinkerHandL10.h` - Modbus L10 型号实现
- `include/HandFactory.h` - 工厂类

## 版本信息

- **改进版本**：1.1.7+
- **兼容性**：完全向后兼容

