#!/bin/bash

# LinkerHand C++ SDK 构建和安装脚本
# 支持 Linux 和 macOS

set -e

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# 打印带颜色的消息
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查命令是否存在
check_command() {
    if ! command -v $1 &> /dev/null; then
        print_error "$1 未安装，请先安装 $1"
        exit 1
    fi
}

# 删除 $INSTALL_PREFIX 下所有已安装产物（含历史版本的目录式布局）。
# install 前的清理与 -u/--uninstall 共用此函数，确保"装什么"与"卸什么"始终一致：
#   - lib/linkerhand-cpp-sdk/           当前布局：lib/ + third_party/（旧版还有 x86_64/ 等架构子目录）
#   - lib/cmake/LinkerHand              当前 cmake 配置包；LinkerHandCPPSDK 为旧包名
#   - lib/{lib,}linkerhand_cpp_sdk.*    标准库目录下的软链（旧版或为实体文件）
#   - include/linkerhand-cpp-sdk        公共头
# 参数 $1 为 sudo 前缀（无权限时为 "sudo"，root 下为空）。
purge_install_artifacts() {
    local sudo_cmd="$1"
    $sudo_cmd rm -rf $INSTALL_PREFIX/lib/liblinkerhand_cpp_sdk.* \
                     $INSTALL_PREFIX/lib/linkerhand_cpp_sdk.* \
                     $INSTALL_PREFIX/lib/linkerhand-cpp-sdk \
                     $INSTALL_PREFIX/lib/cmake/linkerhand-cpp-sdk \
                     $INSTALL_PREFIX/lib/cmake/LinkerHand \
                     $INSTALL_PREFIX/lib/cmake/LinkerHandCPPSDK \
                     $INSTALL_PREFIX/include/linkerhand-cpp-sdk
}

# 显示帮助信息
show_help() {
    cat << EOF
用法: $0 [选项]

选项:
    -h, --help          显示此帮助信息
    -b, --build         仅构建项目
    -i, --install       构建并安装到系统
    -u, --uninstall     从系统卸载（删除所有已安装产物）
    -c, --clean         清理构建目录
    -t, --test          构建并运行测试
    --prefix PATH       指定安装前缀 (默认: /usr/local)
    --skip-tests        跳过测试程序编译

示例:
    $0                  构建项目
    $0 -i               构建并安装到系统
    $0 -u               从系统卸载
    $0 -c               清理构建目录
    $0 --prefix /opt    构建并安装到 /opt
    $0 -u --prefix /opt 从 /opt 卸载

EOF
}

# 默认参数
BUILD_ONLY=false
INSTALL=false
UNINSTALL=false
CLEAN=false
RUN_TESTS=false
SKIP_TESTS=false
INSTALL_PREFIX="/usr/local"

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -b|--build)
            BUILD_ONLY=true
            shift
            ;;
        -i|--install)
            INSTALL=true
            shift
            ;;
        -u|--uninstall)
            UNINSTALL=true
            shift
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -t|--test)
            RUN_TESTS=true
            shift
            ;;
        --prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --skip-tests)
            SKIP_TESTS=true
            shift
            ;;
        *)
            print_error "未知选项: $1"
            show_help
            exit 1
            ;;
    esac
done

# 检测系统信息
OS=$(uname -s)
ARCH=$(uname -m)
print_info "检测到操作系统: $OS"
print_info "检测到系统架构: $ARCH"

# 验证系统支持
if [[ "$OS" != "Linux" && "$OS" != "Darwin" ]]; then
    print_error "不支持的操作系统: $OS"
    print_error "此脚本仅支持 Linux 和 macOS"
    exit 1
fi

# 清理构建目录
if [ "$CLEAN" = true ]; then
    print_info "清理构建目录..."
    if [ -d "build" ]; then
        rm -rf build
        print_info "构建目录已清理"
    else
        print_warn "构建目录不存在"
    fi
    exit 0
fi

# 卸载：删除 $INSTALL_PREFIX 下的全部已安装产物，无需先构建
if [ "$UNINSTALL" = true ]; then
    print_info "从 $INSTALL_PREFIX 卸载 LinkerHand C++ SDK..."

    if [ "$EUID" -ne 0 ]; then
        print_warn "需要 sudo 权限来卸载系统目录"
        SUDO="sudo"
    else
        SUDO=""
    fi

    purge_install_artifacts "$SUDO"

    if [ "$OS" = "Linux" ]; then
        print_info "更新动态链接库缓存..."
        $SUDO ldconfig
    fi

    # 自检：确认没有残留
    LEFTOVER=$(find "$INSTALL_PREFIX/lib" "$INSTALL_PREFIX/include" \
                    -iname '*linkerhand*' 2>/dev/null | sort)
    if [ -n "$LEFTOVER" ]; then
        print_warn "仍检测到以下残留（可能来自其他安装方式，请手动确认）："
        echo "$LEFTOVER" | sed 's/^/  /'
    else
        print_info "卸载完成，未检测到残留。"
    fi
    exit 0
fi

# 检查必要的工具
print_info "检查必要的构建工具..."
check_command cmake
check_command make

# 创建构建目录
print_info "创建构建目录..."
mkdir -p build
cd build

# 配置 CMake
print_info "配置 CMake..."
CMAKE_ARGS=("-DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX")
if [ "$SKIP_TESTS" = true ]; then
    CMAKE_ARGS+=("-DBUILD_TESTS=OFF")
fi
cmake .. "${CMAKE_ARGS[@]}"

# 构建
print_info "开始构建项目..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# 检查构建结果
if [ $? -eq 0 ]; then
    print_info "构建成功！"
else
    print_error "构建失败！"
    exit 1
fi

# 运行测试
if [ "$RUN_TESTS" = true ]; then
    print_info "运行测试..."
    if [ -f "bin/main" ]; then
        ./bin/main
    else
        print_warn "未找到测试程序"
    fi
fi

# 如果只构建，则退出
if [ "$BUILD_ONLY" = true ]; then
    print_info "构建完成！可执行文件位于: $(pwd)/bin/"
    exit 0
fi

# 安装
if [ "$INSTALL" = true ]; then
    print_info "开始安装到 $INSTALL_PREFIX..."

    # 检查是否有 sudo 权限
    if [ "$EUID" -ne 0 ]; then
        print_warn "需要 sudo 权限来安装到系统"
        SUDO="sudo"
    else
        SUDO=""
    fi

    # 清理旧的安装。必须连历史版本的目录式布局一起删，否则会出现新旧 .so 叠层：
    #   - 旧版把 .so 装在 lib/linkerhand-cpp-sdk/<arch>/（如 x86_64/），符号是无命名空间的旧 ABI
    #   - 旧版 cmake 包名曾是 LinkerHandCPPSDK
    # 残留会让示例的 find_library 兜底分支（搜索顺序 .../linkerhand-cpp-sdk/<arch> 在 /usr/local/lib 之前）
    # 链到旧 .so → undefined reference。整目录删掉，由本次 install 重新铺。
    print_info "清理旧的安装..."
    purge_install_artifacts "$SUDO"

    # 通过 CMake 的 install 规则安装。对外公共头白名单、库文件、third_party 运行时
    # 依赖、cmake config target 等都由 linkerhand/CMakeLists.txt 统一控制，
    # 不要在这里再手动 cp 头文件——会绕过白名单把全部内部头释放出去。
    print_info "执行 cmake --install (受白名单约束)..."
    $SUDO cmake --install . --prefix "$INSTALL_PREFIX"

    # 在标准库目录建符号链接，方便 -llinkerhand_cpp_sdk 直接找到
    if [ "$OS" = "Linux" ]; then
        SDK_LIB_DIR="$INSTALL_PREFIX/lib/linkerhand-cpp-sdk/lib"
        if [ -d "$SDK_LIB_DIR" ]; then
            for lib_file in $SDK_LIB_DIR/*.so*; do
                [ -f "$lib_file" ] || continue
                lib_name=$(basename $lib_file)
                $SUDO ln -sf "$SDK_LIB_DIR/$lib_name" "$INSTALL_PREFIX/lib/$lib_name"
            done
        fi
        print_info "更新动态链接库缓存..."
        $SUDO ldconfig
    elif [ "$OS" = "Darwin" ]; then
        SDK_LIB_DIR="$INSTALL_PREFIX/lib/linkerhand-cpp-sdk/lib"
        if [ -d "$SDK_LIB_DIR" ]; then
            for lib_file in $SDK_LIB_DIR/*.dylib*; do
                [ -f "$lib_file" ] || continue
                lib_name=$(basename $lib_file)
                $SUDO ln -sf "$SDK_LIB_DIR/$lib_name" "$INSTALL_PREFIX/lib/$lib_name"
            done
        fi
    fi

    # 列出最终对外暴露的公共头，便于打包前肉眼确认白名单未被破坏
    # 头按 api/core/communication 子目录发布，故递归列出（去掉 -maxdepth 1）
    print_info "对外公共头清单："
    find "$INSTALL_PREFIX/include/linkerhand-cpp-sdk" -name "*.h" | sort | sed "s|$INSTALL_PREFIX/include/linkerhand-cpp-sdk/|  |"

    print_info "安装完成！"
    echo ""
    echo "使用示例（推荐 find_package）："
    echo "  find_package(linkerhand-cpp-sdk CONFIG REQUIRED)"
    echo "  target_link_libraries(<tgt> PRIVATE LinkerHand::linkerhand_cpp_sdk)"
    echo "  参考工程: linkerhand/examples/standalone/"
    echo ""
    INC="$INSTALL_PREFIX/include/linkerhand-cpp-sdk"
    echo "或手动指定（头按子目录发布）："
    echo "  g++ -std=c++17 -o test test.cpp -llinkerhand_cpp_sdk \\"
    echo "      -I$INC -I$INC/api -I$INC/core -I$INC/communication"
    if [ "$OS" = "Linux" ]; then
        echo "  运行: LD_LIBRARY_PATH=$INSTALL_PREFIX/lib ./test"
    else
        echo "  运行: DYLD_LIBRARY_PATH=$INSTALL_PREFIX/lib ./test"
    fi
    exit 0
fi

# 默认只构建
print_info "构建完成！可执行文件位于: $(pwd)/bin/"
print_info "使用 '$0 -i' 来安装到系统"
print_info "使用 '$0 -h' 查看更多选项"