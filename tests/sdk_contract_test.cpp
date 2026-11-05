#include "LegacyModels.h"
static_assert(L6 == 0 && L7 == 1 && L10 == 2 && L20 == 3 && L21 == 4 && L25 == 5);
static_assert(O6 == 6 && G20 == 7 && O20 == 8 && L30 == 9 && O30 == 10);
static_assert(L20_10 == G20 && L20_11 == G20 && L20_Lite == L10);
static_assert(LEFT == 0x28 && RIGHT == 0x27);
int main() {
    const auto* alias = linkerhand::legacy::find_model("L20_11");
    return !alias || alias->model != G20 || linkerhand::to_legacy(linkerhand::HandModel::G20) != G20;
}
