#!/usr/bin/env bash
# LinkerHand C++ SDK 一条命令安装脚本 (Linux)
#
# 用法：
#   curl -fsSL https://raw.githubusercontent.com/linker-bot/linkerhand-cpp-sdk/main/scripts/install.sh | bash
#
# 带参数：
#   curl -fsSL .../install.sh | bash -s -- --version v2.1.8 --prefix $HOME/.local
#
# 选项：
#   --version <TAG>   安装指定 Release tag（默认 latest）
#   --prefix <PATH>   安装前缀（默认 /usr/local）
#   --no-sudo         不使用 sudo（在 root 容器或已具备写权限时使用）
#   --keep-tmp        安装完成后保留临时目录，便于排查
#   -h, --help        帮助

set -euo pipefail

REPO="linker-bot/linkerhand-cpp-sdk"
VERSION=""            # 为空表示 latest
PREFIX="/usr/local"
USE_SUDO="auto"       # auto|yes|no
KEEP_TMP="false"

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; NC='\033[0m'
info(){  echo -e "${GREEN}[INFO]${NC} $*"; }
warn(){  echo -e "${YELLOW}[WARN]${NC} $*"; }
die(){   echo -e "${RED}[ERROR]${NC} $*" >&2; exit 1; }

usage(){ sed -n '2,15p' "$0" | sed 's/^# \{0,1\}//'; exit 0; }

# 解析参数
while [[ $# -gt 0 ]]; do
    case $1 in
        --version) VERSION="$2"; shift 2 ;;
        --prefix)  PREFIX="$2";  shift 2 ;;
        --no-sudo) USE_SUDO="no"; shift ;;
        --keep-tmp) KEEP_TMP="true"; shift ;;
        -h|--help) usage ;;
        *) die "未知参数: $1" ;;
    esac
done

# 依赖检查
for cmd in curl tar sha256sum uname; do
    command -v "$cmd" >/dev/null 2>&1 || die "缺少依赖命令: $cmd"
done

# 架构映射
UNAME_M=$(uname -m)
case "$UNAME_M" in
    x86_64|amd64) ARCH="linux-x86_64" ;;
    aarch64|arm64) ARCH="linux-aarch64" ;;
    *) die "不支持的架构: $UNAME_M（当前仅支持 x86_64 / aarch64）" ;;
esac
info "目标架构: $ARCH"

# sudo 策略
if [[ "$USE_SUDO" == "auto" ]]; then
    if [[ $EUID -eq 0 ]] || [[ -w "$PREFIX" ]]; then
        SUDO=""
    else
        command -v sudo >/dev/null 2>&1 || die "需要写入 $PREFIX 但当前非 root 且未安装 sudo；请指定 --prefix 到可写目录或使用 --no-sudo"
        SUDO="sudo"
    fi
elif [[ "$USE_SUDO" == "no" ]]; then
    SUDO=""
fi

# 解析版本 → tag
if [[ -z "$VERSION" ]]; then
    info "查询最新 Release..."
    TAG=$(curl -fsSL "https://api.github.com/repos/${REPO}/releases/latest" \
          | grep -oE '"tag_name":\s*"[^"]+"' | head -1 | sed -E 's/.*"([^"]+)".*/\1/')
    [[ -n "$TAG" ]] || die "无法解析最新 Release tag"
else
    # 允许用户传 v2.1.8 或 2.1.8
    TAG="$VERSION"
    [[ "$TAG" == v* ]] || TAG="v$TAG"
fi
VER="${TAG#v}"
info "目标版本: $TAG"

PKG="linkerhand-cpp-sdk-${VER}-${ARCH}"
TARBALL="${PKG}.tar.gz"
BASE_URL="https://github.com/${REPO}/releases/download/${TAG}"

# 临时目录
TMP=$(mktemp -d -t linkerhand-sdk-install.XXXXXX)
cleanup(){
    if [[ "$KEEP_TMP" == "true" ]]; then
        warn "保留临时目录：$TMP"
    else
        rm -rf "$TMP"
    fi
}
trap cleanup EXIT

cd "$TMP"

# 下载 tarball 与 SHA256SUMS
info "下载: $BASE_URL/$TARBALL"
curl -fL --retry 3 -o "$TARBALL" "$BASE_URL/$TARBALL"

info "下载: $BASE_URL/SHA256SUMS"
curl -fL --retry 3 -o SHA256SUMS "$BASE_URL/SHA256SUMS"

# 校验
info "校验 SHA256..."
if ! grep -E "  ${TARBALL}\$" SHA256SUMS | sha256sum -c - >/dev/null 2>&1; then
    die "SHA256 校验失败，请重试或提交 issue"
fi
info "校验通过"

# 解压 + 交给 build.sh 完成安装（复用符号链接 + ldconfig）
info "解压..."
tar -xzf "$TARBALL"

# tarball 内的顶层目录名与包名一致
STAGE_DIR="$TMP/$PKG"
[[ -d "$STAGE_DIR" ]] || die "解压后未找到目录：$STAGE_DIR"

info "安装到 $PREFIX ..."
# 这里的 tarball 已是"已 install 好的目录树"，直接 rsync/cp 到 PREFIX 即可，
# 然后调用 ldconfig 与建立 /usr/local/lib 下的库软链，逻辑与 build.sh 保持一致。
if command -v rsync >/dev/null 2>&1; then
    $SUDO rsync -a "$STAGE_DIR/" "$PREFIX/"
else
    $SUDO cp -a "$STAGE_DIR/." "$PREFIX/"
fi

# 在 $PREFIX/lib 下建库软链，让 -llinkerhand_cpp_sdk 直接可用
SDK_LIB_DIR="$PREFIX/lib/linkerhand-cpp-sdk/lib"
if [[ -d "$SDK_LIB_DIR" ]]; then
    for lib in "$SDK_LIB_DIR"/*.so*; do
        [[ -f "$lib" ]] || continue
        $SUDO ln -sf "$lib" "$PREFIX/lib/$(basename "$lib")"
    done
    if command -v ldconfig >/dev/null 2>&1 && [[ "$PREFIX" == "/usr" || "$PREFIX" == "/usr/local" ]]; then
        $SUDO ldconfig
    fi
fi

info "安装完成 ✓"
echo
echo "版本    : $TAG"
echo "前缀    : $PREFIX"
echo "头文件  : $PREFIX/include/linkerhand-cpp-sdk/"
echo "CMake   : find_package(linkerhand-cpp-sdk CONFIG REQUIRED)"
if [[ "$PREFIX" != "/usr" && "$PREFIX" != "/usr/local" ]]; then
    echo
    warn "非标准前缀，需在下游 CMake 配置时加："
    echo "  cmake -DCMAKE_PREFIX_PATH=$PREFIX ..."
    echo "或运行时："
    echo "  export LD_LIBRARY_PATH=$PREFIX/lib:\$LD_LIBRARY_PATH"
fi
