#!/usr/bin/env bash
#
# CAN 自动配置安装脚本（适用于 PEAK PCAN-USB / candleLight(gs_usb) 等 SocketCAN 适配器）
#
# 作用：安装一个 systemd 模板服务 + udev 规则，使任意 canX 接口
#   - 插入/开机时自动 up（按设备能力自动选 经典CAN 或 CANFD、设 txqueuelen、bus-off 自动恢复）；
#   - 关机/重启时自动干净 down，避免适配器接收通路卡死。
#
# 用法：  sudo ./can-autocfg.sh                                   # 经典 1000000；FD 设备自动 dbitrate 5000000
#         sudo BITRATE=500000 ./can-autocfg.sh                    # 改经典/仲裁段波特率
#         sudo BITRATE=1000000 DBITRATE=2000000 ./can-autocfg.sh  # 改 FD 数据段波特率
#
set -euo pipefail

BITRATE="${BITRATE:-1000000}"
DBITRATE="${DBITRATE:-5000000}"
TXQUEUELEN="${TXQUEUELEN:-1024}"
RESTART_MS="${RESTART_MS:-100}"

if [ "$(id -u)" -ne 0 ]; then
  echo "请用 sudo 运行： sudo ./can-autocfg.sh" >&2
  exit 1
fi

IP_BIN="$(command -v ip || echo /usr/sbin/ip)"
HELPER=/usr/local/sbin/can-setup-up.sh
SERVICE=/etc/systemd/system/can-setup@.service
RULE=/etc/udev/rules.d/90-can-setup.rules

echo "[1/5] 写入能力判别启动脚本 $HELPER ..."
cat > "$HELPER" <<EOF
#!/usr/bin/env bash
# 由 can-autocfg.sh 生成：按设备能力自动以 经典CAN 或 CANFD 启动接口。
set -eu

IFACE="\${1:?usage: can-setup-up.sh <iface>}"
IP="$IP_BIN"
BITRATE="\${BITRATE:-$BITRATE}"
DBITRATE="\${DBITRATE:-$DBITRATE}"
RESTART_MS="\${RESTART_MS:-$RESTART_MS}"

# 干净起点
"\$IP" link set "\$IFACE" down 2>/dev/null || true

# FD 能力判别：仅 FD 控制器广播数据段时序常量 data_bittiming_const
# （DOWN 时即存在，与 max_mtu / 是否已 fd on 无关，厂商/驱动无关）
FDARGS=""
if "\$IP" -d -j link show "\$IFACE" | grep -q data_bittiming_const; then
  FDARGS="dbitrate \$DBITRATE fd on"
fi

# restart-ms 容错：部分 FD 控制器(如 gs_usb)不支持 Bus-Off 自动恢复，失败则降级重试
"\$IP" link set "\$IFACE" type can bitrate "\$BITRATE" \$FDARGS restart-ms "\$RESTART_MS" 2>/dev/null \\
  || "\$IP" link set "\$IFACE" type can bitrate "\$BITRATE" \$FDARGS

"\$IP" link set "\$IFACE" up
EOF
chmod +x "$HELPER"

echo "[2/5] 写入模板服务 $SERVICE (bitrate=$BITRATE dbitrate=$DBITRATE) ..."
cat > "$SERVICE" <<EOF
[Unit]
Description=CAN %i setup (up on appear, clean down on shutdown)
BindsTo=sys-subsystem-net-devices-%i.device
After=sys-subsystem-net-devices-%i.device

[Service]
Type=oneshot
RemainAfterExit=yes
Environment=BITRATE=$BITRATE DBITRATE=$DBITRATE RESTART_MS=$RESTART_MS
ExecStart=$HELPER %i
ExecStartPost=$IP_BIN link set %i txqueuelen $TXQUEUELEN
ExecStop=$IP_BIN link set %i down
EOF

echo "[3/5] 写入 udev 规则 $RULE ..."
cat > "$RULE" <<'EOF'
# 任何 CAN 网络接口出现时，自动启动对应的 can-setup@<iface> 实例
SUBSYSTEM=="net", ACTION=="add", KERNEL=="can*", TAG+="systemd", ENV{SYSTEMD_WANTS}+="can-setup@$name.service"
EOF

echo "[4/5] 重新加载 systemd 与 udev ..."
systemctl daemon-reload
udevadm control --reload

echo "[5/5] 对已存在的 CAN 接口触发一次（无需重启即可生效）..."
for IFACE in $(ls /sys/class/net/ | grep -E '^can[0-9]+$' || true); do
  echo "  -> 触发 $IFACE"
  udevadm trigger --action=add --subsystem-match=net --sysname-match="$IFACE" || true
done
sleep 2

echo
echo "===== 安装完成，当前 CAN 接口状态 ====="
for IFACE in $(ls /sys/class/net/ | grep -E '^can[0-9]+$' || true); do
  systemctl is-active "can-setup@${IFACE}.service" >/dev/null 2>&1 \
    && echo "  $IFACE : 服务 active" || echo "  $IFACE : 服务未激活(检查日志)"
  "$IP_BIN" -details link show "$IFACE" | sed -n '1p;3p' | sed 's/^/    /'
done
echo
echo "提示：以后无需手动 ip link set canX up，开机/插入即自动配置（经典/FD 自动识别）。"
