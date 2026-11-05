#pragma once

#include "Common.h"
#include <array>
#include <string_view>

namespace linkerhand::legacy {
struct ModelInfo {
    const char* name;
    LINKER_HAND model;
    int position_size;
    bool fd;
    bool modbus;
};
// Maintained model/API layouts. Historical enum values remain available for ABI compatibility.
inline constexpr std::array<ModelInfo, 5> models{{
    {"L7", L7, 7, false, true},
    {"L20_10", L20_10, 16, false, false},
    {"L21", L21, 25, false, false},
    {"L25", L25, 25, false, false},
    {"O20", O20, 16, true, false},
}};
inline const ModelInfo* find_model(std::string_view name) {
    if (name == "L20_11" || name == "G20") name = "L20_10";
    for (const auto& info : models) if (name == info.name) return &info;
    return nullptr;
}
} // namespace linkerhand::legacy
