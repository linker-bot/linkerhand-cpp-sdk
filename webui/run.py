#!/usr/bin/env python3
"""LinkerHand 全型号 Web 示教器 —— 纯 Python 标准库，零第三方依赖。

启动编译好的 C++ 桥 `web_bridge`（build/bin/）作为子进程，通过 stdin/stdout 文本协议
下发姿势/速度/力矩、读回实测位置与触觉压感；浏览器端用 HTTP 轮询交互。任何装了 Python3
的平台解压即用，无需 pip、Boost、websocketpp。

用法:
    python3 webui/run.py --model O6 --side left [--channel can0]
                               [--host 0.0.0.0] [--port 8080] [--bridge PATH]

然后浏览器打开 http://<本机>:8080/ 。运行前需先拉起对应 CAN 接口并给手上电。
"""
import argparse
import json
import mimetypes
import os
import threading
import subprocess
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_BRIDGE = os.path.join(REPO, "build", "bin", "web_bridge")
STATIC_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "static")

# 型号 -> 关节名（复刻 hand_teach_pendant/src/HandController.h:joint_names_）。
# 数量 = 该型号控制自由度，前端据此渲染滑块与标签。
JOINT_NAMES = {
    "L6":  ["拇指根部", "拇指侧摆", "食指根部", "中指根部", "无名指根部", "小指根部"],
    "O6":  ["拇指根部", "拇指侧摆", "食指根部", "中指根部", "无名指根部", "小指根部"],
    "L7":  ["大拇指弯曲", "大拇指横摆", "食指弯曲", "中指弯曲", "无名指弯曲", "小拇指弯曲", "拇指旋转"],
    "L10": ["拇指根部", "拇指侧摆", "食指根部", "中指根部", "无名指根部", "小指根部",
            "食指侧摆", "无名指侧摆", "小指侧摆", "拇指旋转"],
    "L20": ["拇指根部", "食指根部", "中指根部", "无名指根部", "小指根部", "拇指侧摆",
            "食指侧摆", "中指侧摆", "无名指侧摆", "小指侧摆", "拇指横摆", "预留1",
            "预留2", "预留3", "预留4", "拇指尖部", "食指末端", "中指末端", "无名指末端", "小指末端"],
    "G20": ["拇指根部", "食指根部", "中指根部", "无名指根部", "小指根部",
            "拇指侧摆", "食指侧摆", "中指侧摆", "无名指侧摆", "小指侧摆", "拇指横摆",
            "拇指指尖", "食指指尖", "中指指尖", "无名指指尖", "小指指尖"],
    "L21": ["大拇指根部", "食指根部", "中指根部", "无名指根部", "小拇指根部", "大拇指侧摆",
            "食指侧摆", "中指侧摆", "无名指侧摆", "小拇指侧摆", "大拇指横滚", "预留1",
            "预留2", "预留3", "预留4", "大拇指中部", "预留5", "预留6", "预留7", "预留8",
            "大拇指指尖", "食指指尖", "中指指尖", "无名指指尖", "小拇指指尖"],
    "L25": ["大拇指根部", "食指根部", "中指根部", "无名指根部", "小拇指根部", "大拇指侧摆",
            "食指侧摆", "中指侧摆", "无名指侧摆", "小拇指侧摆", "大拇指横滚", "预留1",
            "预留2", "预留3", "预留4", "大拇指中部", "食指中部", "中指中部", "无名指中部",
            "小拇指中部", "大拇指指尖", "食指指尖", "中指指尖", "无名指指尖", "小拇指指尖"],
    "O20": ["拇指指根", "拇指指尖", "拇指侧摆", "拇指旋转",
            "食指侧摆", "食指指根", "食指指尖",
            "中指侧摆", "中指指根", "中指指尖",
            "无名指侧摆", "无名指指根", "无名指指尖",
            "小指侧摆", "小指指根", "小指指尖"],
}

# 与 JOINT_NAMES 同型号、同顺序的英文关节名，供前端英文界面使用。
JOINT_NAMES_EN = {
    "L6":  ["Thumb base", "Thumb abd.", "Index base", "Middle base", "Ring base", "Pinky base"],
    "O6":  ["Thumb base", "Thumb abd.", "Index base", "Middle base", "Ring base", "Pinky base"],
    "L7":  ["Thumb flex", "Thumb roll", "Index flex", "Middle flex", "Ring flex", "Pinky flex", "Thumb rot."],
    "L10": ["Thumb base", "Thumb abd.", "Index base", "Middle base", "Ring base", "Pinky base",
            "Index abd.", "Ring abd.", "Pinky abd.", "Thumb rot."],
    "L20": ["Thumb base", "Index base", "Middle base", "Ring base", "Pinky base", "Thumb abd.",
            "Index abd.", "Middle abd.", "Ring abd.", "Pinky abd.", "Thumb roll", "Reserved 1",
            "Reserved 2", "Reserved 3", "Reserved 4", "Thumb tip", "Index tip", "Middle tip", "Ring tip", "Pinky tip"],
    "G20": ["Thumb base", "Index base", "Middle base", "Ring base", "Pinky base",
            "Thumb abd.", "Index abd.", "Middle abd.", "Ring abd.", "Pinky abd.", "Thumb roll",
            "Thumb tip", "Index tip", "Middle tip", "Ring tip", "Pinky tip"],
    "L21": ["Thumb base", "Index base", "Middle base", "Ring base", "Pinky base", "Thumb abd.",
            "Index abd.", "Middle abd.", "Ring abd.", "Pinky abd.", "Thumb roll", "Reserved 1",
            "Reserved 2", "Reserved 3", "Reserved 4", "Thumb mid", "Reserved 5", "Reserved 6", "Reserved 7", "Reserved 8",
            "Thumb tip", "Index tip", "Middle tip", "Ring tip", "Pinky tip"],
    "L25": ["Thumb base", "Index base", "Middle base", "Ring base", "Pinky base", "Thumb abd.",
            "Index abd.", "Middle abd.", "Ring abd.", "Pinky abd.", "Thumb roll", "Reserved 1",
            "Reserved 2", "Reserved 3", "Reserved 4", "Thumb mid", "Index mid", "Middle mid", "Ring mid",
            "Pinky mid", "Thumb tip", "Index tip", "Middle tip", "Ring tip", "Pinky tip"],
    "O20": ["Thumb base", "Thumb tip", "Thumb abd.", "Thumb rot.",
            "Index abd.", "Index base", "Index tip",
            "Middle abd.", "Middle base", "Middle tip",
            "Ring abd.", "Ring base", "Ring tip",
            "Pinky abd.", "Pinky base", "Pinky tip"],
}

bridge = None
bridge_lock = threading.Lock()
state_lock = threading.Lock()

BRIDGE_PATH = DEFAULT_BRIDGE   # main() 里按 --bridge 覆盖，供 /connect 复用
current = None                 # 当前连接参数 {"model","side","comm","channel"}；未连接为 None

# 型号 -> 可选通信方式（前端下拉联动 + /connect 校验依据）。
# CAN 全型号支持；CAN-FD 仅 O20；Modbus 仅 O6/L7/L10（HandFactory 其余会抛异常）。
# EtherCAT 当前 SDK 未接通（工厂直接抛异常），不列入。
COMM_SUPPORT = {
    "L6":  ["can"],
    "O6":  ["can", "modbus"],
    "L7":  ["can", "modbus"],
    "L10": ["can", "modbus"],
    "L20": ["can"],
    "L21": ["can"],
    "L25": ["can"],
    "G20": ["can"],
    "O20": ["canfd"],
}

meta = {}                 # {"model","dof","version"}
latest = {                # 最新回读缓存
    "position": None,
    "speed": None,
    "torque": None,
    "force": None,        # {"fingers":[[[..]]]}
    "palm": None,         # {"palm":[[..]]}
    "temperature": None,  # 逐关节温度(°C)
    "fault": None,        # 逐关节故障码
}


def start_bridge(bridge_path, model, side, comm, channel, timeout=8.0):
    global bridge, current
    if not os.path.exists(bridge_path):
        raise SystemExit(f"web_bridge 未找到: {bridge_path}\n请先构建 SDK 示例 (BUILD_EXAMPLES=ON)。")
    argv = [bridge_path, model, side, comm]
    if channel:
        argv.append(channel)
    proc = subprocess.Popen(
        argv,
        cwd=os.path.dirname(bridge_path),      # 让 $ORIGIN 定位到同目录 .so
        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, bufsize=1,
    )
    bridge = proc
    current = {"model": model, "side": side, "comm": comm, "channel": channel}

    ready = threading.Event()

    def reader():
        for line in proc.stdout:            # 绑定到本次进程，重连后旧线程随其 EOF 自退
            line = line.rstrip()
            if not line:
                continue
            if line == "READY":
                ready.set()
            elif line.startswith("META "):
                try:
                    with state_lock:
                        meta.update(json.loads(line[5:]))
                except ValueError:
                    pass
            elif line.startswith("POS "):
                _store("position", _ints(line[4:]))
            elif line.startswith("SPD "):
                _store("speed", _ints(line[4:]))
            elif line.startswith("TRQ "):
                _store("torque", _ints(line[4:]))
            elif line.startswith("FORCE "):
                _store_json("force", line[6:])
            elif line.startswith("PALM "):
                _store_json("palm", line[5:])
            elif line.startswith("TEMP "):
                _store("temperature", _ints(line[5:]))
            elif line.startswith("FAULT "):
                _store("fault", _ints(line[6:]))
            else:
                print(f"[bridge] {line}", flush=True)   # 冒出 BRIDGE_ERROR/WARN 等
        ready.set()   # EOF 也解除等待

    threading.Thread(target=reader, daemon=True).start()

    if not ready.wait(timeout):
        print("[warn] 等待 bridge READY 超时——检查 CAN 接口是否已拉起、手是否上电。", flush=True)
    if proc.poll() is not None:
        raise SystemExit("bridge 在就绪前退出——检查接口 / 供电 / 型号与通信是否匹配。")


def stop_bridge():
    """停掉当前 bridge 子进程并清空回读缓存。旧 reader 线程随 stdout EOF 自退。"""
    global bridge, current
    proc = bridge
    bridge = None
    current = None
    if proc is not None and proc.poll() is None:
        try:
            proc.stdin.write("Q\n")
            proc.stdin.flush()
        except Exception:
            pass
        try:
            proc.wait(timeout=2.0)
        except Exception:
            proc.terminate()
            try:
                proc.wait(timeout=2.0)
            except Exception:
                proc.kill()
    with state_lock:
        meta.clear()
        for k in latest:
            latest[k] = None



def _ints(s):
    out = []
    for tok in s.split():
        try:
            out.append(int(tok))
        except ValueError:
            pass
    return out


def _store(key, val):
    with state_lock:
        latest[key] = val


def _store_json(key, s):
    try:
        val = json.loads(s)
    except ValueError:
        return
    with state_lock:
        latest[key] = val


def _write(line):
    with bridge_lock:
        if bridge is None or bridge.poll() is not None:
            raise RuntimeError("bridge 进程已退出")
        bridge.stdin.write(line)
        bridge.stdin.flush()


def _clamp_vec(vals):
    return [max(0, min(255, int(v))) for v in vals]


def send_pose(vals):
    _write("P " + " ".join(str(v) for v in _clamp_vec(vals)) + "\n")


def send_speed(vals):
    _write("S " + " ".join(str(v) for v in _clamp_vec(vals)) + "\n")


def send_torque(vals):
    _write("T " + " ".join(str(v) for v in _clamp_vec(vals)) + "\n")


def _dof():
    return int(meta.get("dof", 0)) or 10


def _vec_from_payload(payload):
    """接受 {"vals":[...]}（整向量）或 {"val":N}（单标量广播成 DOF 长）。"""
    if "vals" in payload:
        return list(payload["vals"])
    if "val" in payload:
        return [int(payload["val"])] * _dof()
    raise ValueError("需要 'vals' 或 'val'")


class Handler(BaseHTTPRequestHandler):
    def log_message(self, *a):
        pass

    def handle_one_request(self):
        # 前端每隔几百 ms 轮询 /state，页面刷新/切换时旧连接会中途断开，
        # 服务端 write 抛 BrokenPipe/ConnectionReset。属正常现象，静默关闭即可，避免刷 traceback。
        try:
            super().handle_one_request()
        except (BrokenPipeError, ConnectionResetError):
            self.close_connection = True

    def _send(self, code, body, ctype="text/html; charset=utf-8"):
        data = body.encode("utf-8") if isinstance(body, str) else body
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def _serve_static(self, path):
        # "/" -> index.html；其余按 static/ 下相对路径取文件（防目录穿越）。
        rel = path.split("?", 1)[0]
        if rel in ("/", ""):
            rel = "/index.html"
        full = os.path.normpath(os.path.join(STATIC_DIR, rel.lstrip("/")))
        if not full.startswith(STATIC_DIR) or not os.path.isfile(full):
            return self._send(404, "not found", "text/plain")
        ctype = mimetypes.guess_type(full)[0] or "application/octet-stream"
        if ctype.startswith("text/") or ctype in ("application/javascript", "application/json"):
            ctype += "; charset=utf-8"
        with open(full, "rb") as f:
            self._send(200, f.read(), ctype)

    def do_GET(self):
        if self.path == "/meta":
            connected = bridge is not None and bridge.poll() is None
            with state_lock:
                model = meta.get("model", "?")
                m = {
                    "connected": connected,
                    "model": model,
                    "dof": _dof(),
                    "version": meta.get("version", ""),
                    "rates": meta.get("rates", {}),
                    "names": JOINT_NAMES.get(model, [f"关节{i}" for i in range(_dof())]),
                    "names_en": JOINT_NAMES_EN.get(model, [f"Joint {i}" for i in range(_dof())]),
                }
            self._send(200, json.dumps(m), "application/json")
        elif self.path == "/options":
            o = {
                "models": list(JOINT_NAMES.keys()),
                "comm_support": COMM_SUPPORT,
                "sides": ["left", "right"],
                "current": current,
            }
            self._send(200, json.dumps(o), "application/json")
        elif self.path == "/state":
            with state_lock:
                snap = dict(latest)
            self._send(200, json.dumps(snap), "application/json")
        else:
            self._serve_static(self.path)

    def do_POST(self):
        try:
            n = int(self.headers.get("Content-Length", 0))
            payload = json.loads(self.rfile.read(n) or b"{}")
            if self.path == "/connect":
                return self._handle_connect(payload)
            if self.path == "/disconnect":
                with bridge_lock:
                    stop_bridge()
                return self._send(200, json.dumps({"ok": True}), "application/json")
            if self.path == "/rate":
                # 回读频率(Hz)，转成 bridge 的 "R <chan> <hz>" 命令；chan 白名单校验，hz 限幅 1~100
                chan = str(payload.get("chan", "")).lower()
                if chan not in ("pos", "st", "force", "temp", "fault"):
                    return self._send(400, json.dumps({"ok": False, "error": f"未知通道: {chan}"}), "application/json")
                hz = max(1, min(100, int(payload.get("hz", 30))))
                _write(f"R {chan} {hz}\n")
                return self._send(200, json.dumps({"ok": True, "chan": chan, "hz": hz}), "application/json")
            if self.path == "/pose":
                vals = _clamp_vec(_vec_from_payload(payload))
                send_pose(vals)
            elif self.path == "/speed":
                vals = _clamp_vec(_vec_from_payload(payload))
                send_speed(vals)
            elif self.path == "/torque":
                vals = _clamp_vec(_vec_from_payload(payload))
                send_torque(vals)
            else:
                return self._send(404, "not found", "text/plain")
            self._send(200, json.dumps({"ok": True, "vals": vals}), "application/json")
        except Exception as e:
            self._send(400, json.dumps({"ok": False, "error": str(e)}), "application/json")

    def _handle_connect(self, payload):
        model = str(payload.get("model", "")).upper()
        side = str(payload.get("side", "left")).lower()
        comm = str(payload.get("comm", "")).lower()
        channel = str(payload.get("channel", "")).strip()
        if model not in JOINT_NAMES:
            return self._send(200, json.dumps({"ok": False, "error": f"未知型号: {model}"}), "application/json")
        if side not in ("left", "right"):
            side = "left"
        if comm == "ethercat":
            return self._send(200, json.dumps({"ok": False, "error": "EtherCAT 暂不支持"}), "application/json")
        if comm not in COMM_SUPPORT.get(model, []):
            return self._send(200, json.dumps({"ok": False, "error": f"{model} 不支持通信方式 {comm or '(空)'}"}), "application/json")
        with bridge_lock:
            stop_bridge()
            try:
                start_bridge(BRIDGE_PATH, model, side, comm, channel)
            except (Exception, SystemExit) as e:
                stop_bridge()
                return self._send(200, json.dumps({"ok": False, "error": str(e)}), "application/json")
        return self._send(200, json.dumps({"ok": True, "model": model, "dof": _dof()}), "application/json")



def main():
    global BRIDGE_PATH
    ap = argparse.ArgumentParser(description="LinkerHand 全型号无依赖 Web 示教器")
    ap.add_argument("--model", choices=list(JOINT_NAMES.keys()),
                    help="启动即连的型号；不填则待前端设置面板在线连接")
    ap.add_argument("--side", choices=["left", "right"], default="left")
    ap.add_argument("--comm", choices=["can", "canfd", "modbus"], default="",
                    help="通信方式；缺省按型号自动（O20→canfd，其余→can）")
    ap.add_argument("--channel", default="", help="CAN 接口名（如 can0）/ Modbus 串口（如 /dev/ttyUSB0）；O20 可用 'socketcan:can0'")
    ap.add_argument("--host", default="0.0.0.0")
    ap.add_argument("--port", type=int, default=8080)
    ap.add_argument("--bridge", default=DEFAULT_BRIDGE, help="web_bridge 可执行文件路径")
    args = ap.parse_args()
    BRIDGE_PATH = args.bridge

    if args.model:
        comm = args.comm or ("canfd" if args.model == "O20" else "can")
        print(f"启动 bridge (model={args.model} side={args.side} comm={comm} channel={args.channel or 'auto'})...", flush=True)
        start_bridge(args.bridge, args.model, args.side, comm, args.channel)
    else:
        print("未指定 --model，等待前端设置面板在线连接。", flush=True)

    srv = ThreadingHTTPServer((args.host, args.port), Handler)
    shown = "localhost" if args.host in ("0.0.0.0", "") else args.host
    print(f"\n  控制界面 -> http://{shown}:{args.port}/\n", flush=True)
    if args.host == "0.0.0.0":
        print("  (已绑定所有网卡——同局域网其他设备可访问)\n", flush=True)
    try:
        srv.serve_forever()
    except KeyboardInterrupt:
        print("\n正在关闭", flush=True)
    finally:
        try:
            _write("Q\n")
        except Exception:
            pass


if __name__ == "__main__":
    main()
