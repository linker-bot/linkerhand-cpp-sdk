// CommunicationCallbacks.h
#pragma once
#include <cstdint>
#include <cstddef>
#include <functional>
#include <vector>
#include <string>
#include <variant>
#include "communication/CanFrame.h"
#include "core/Common.h"


// 统一的消息ID类型
// using MsgId = std::variant<uint32_t, uint16_t, std::string>;

// 统一的数据缓冲区
using DataBuffer = std::vector<uint8_t>;

// 通用的发送回调函数类型
// using CommTxCallback = std::function<int(const MsgId& id, const uint8_t* data, size_t len)>;
// 通用的接收回调函数类型
// using CommRxCallback = std::function<int(MsgId& id_out, uint8_t* data_out, size_t* len_out)>;

// TX回调函数类型：发送数据 (CAN ID, 数据指针, 数据长度) -> 返回状态码
using CanTxCallback = std::function<int32_t(uint32_t can_id,
	const uint8_t *data,
	uintptr_t data_len)>;
// RX回调函数类型：接收数据 (CAN ID, 数据指针, 数据长度) -> 返回状态码
using CanRxCallback = std::function<int32_t(uint32_t* can_id_out,
    uint8_t* data_out,
    uint8_t* data_len_out)>;

// using CanTxCallback = std::function<int32_t(uint32_t can_id, const uint8_t* data, uintptr_t data_len)>;
// using CanRxCallback = std::function<int32_t(uint32_t* can_id_out, uint8_t* data_out, uint8_t* data_len_out)>;

using ModbusTxCallback = std::function<int32_t(uint8_t slave_id, uint16_t reg_addr, 
                                               const uint8_t* data, uintptr_t data_len)>;
using ModbusRxCallback = std::function<int32_t(uint8_t slave_id, uint16_t* reg_addr_out,
                                               uint8_t* data_out, uint8_t* data_len_out)>;

using EthercatTxCallback = std::function<int32_t(uint16_t slave_addr, uint16_t index,
                                                 uint8_t subindex, const uint8_t* data, uintptr_t data_len)>;
using EthercatRxCallback = std::function<int32_t(uint16_t slave_addr, uint16_t index,
                                                 uint8_t subindex, uint8_t* data_out, uint8_t* data_len_out)>;

// 协议类型枚举
enum class ProtocolType {
    CAN,
    MODBUS,
    ETHERCAT
};
