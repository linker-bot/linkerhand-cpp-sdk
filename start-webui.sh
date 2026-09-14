#!/usr/bin/env bash
# 启动 Web 示教器：先确保 C++ 桥 web_bridge 已编译，未编译则自动构建，再拉起 webui。
# 透传所有参数给 run.py（如 --model O6 --side left --port 8080）。
set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BRIDGE="$REPO/build/bin/web_bridge"

# web_bridge 由 examples 一站式构建产出（BUILD_EXAMPLES=ON）。缺失则先编译。
if [ ! -x "$BRIDGE" ]; then
    echo "[start-webui] 未找到已编译的 web_bridge ($BRIDGE)，开始构建 SDK..."
    "$REPO/build.sh" -b
fi

if [ ! -x "$BRIDGE" ]; then
    echo "[start-webui] 构建后仍未找到 web_bridge，请检查构建输出。" >&2
    exit 1
fi

exec python3 "$REPO/webui/run.py" "$@"
