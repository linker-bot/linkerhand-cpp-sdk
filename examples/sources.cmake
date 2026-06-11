# 客户级示例源清单（相对 examples/ 根，不含 .cpp 扩展名）。
# 嵌入构建（examples/CMakeLists.txt）与独立 CONFIG 构建（examples/standalone/）共用，
# 避免两处维护两份列表。新增示例只需在此登记一次。

# 平台无关示例（所有平台都构建）
set(LINKERHAND_EXAMPLES
    test
    test_0
    test_l10
    test_l10_can_0
    test_l10_can_1
    test_l10_modbus
    test_l7_can_0
    test_l7_can_1
    test_l7_modbus_0
    test_l7_modbus_1
    test_l20_can_0
    test_l21_can_0
    test_g20_can_0
    test_g20_can_1
    test_o6_can_0
    test_o6_can_1
    test_o6_can_3
    test_o6_modbus_0
    test_o6_modbus_1
    test_o6_modbus_2
    L10/action_group_show
    range_to_arc/range_to_arc
)

# 需要 CanFD 支持的示例：仅在非 aarch64 的 Linux 上构建（与顶层 ENABLE_CANFD 逻辑一致）
set(LINKERHAND_EXAMPLES_CANFD
    test_o20_canfd_0
    test_o20_canfd_1
    linker_hand_example
)
