# Maintained hardware examples; these are build targets, not offline tests.
set(LINKERHAND_EXAMPLES
    test_l7_can_0
    test_l7_can_1
    test_l7_modbus_0
    test_l7_modbus_1
    test_l21_can_0
    test_l20_10_can_0
    test_l20_10_can_1
    test_conversion
)
# Packaged vendor CAN FD examples need matching libraries; excluded on Linux aarch64.
set(LINKERHAND_EXAMPLES_CANFD
    test_o20_canfd_0
    test_o20_canfd_1
    test_o20_canfd_double
    hand_tui
)
# SocketCAN FD does not require the vendor SDK.
set(LINKERHAND_EXAMPLES_LINUX
    test_o20_canfd_socket_0
)
