#!/usr/bin/env python3
"""Web slider UI for the LinkerHand O6, with live position readback + torque control.

Launches the compiled `o6_web_bridge` (build/bin) as a subprocess and forwards
slider poses / torque to it over stdin, while reading back the hand's measured
joint positions. Pure Python stdlib -- no pip installs.

Usage:
    python3 web/o6_web.py [--side left|right] [--host 0.0.0.0] [--port 8080]

Then open http://<this-machine>:8080/ in a browser.
"""
import argparse
import json
import os
import subprocess
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BRIDGE = os.path.join(REPO, "build", "bin", "o6_web_bridge")

bridge = None
bridge_lock = threading.Lock()

state_lock = threading.Lock()
latest_actual = None            # last measured [v0..v5] from the hand, or None


def start_bridge(side):
    global bridge
    if not os.path.exists(BRIDGE):
        raise SystemExit(f"bridge binary not found: {BRIDGE}\nCompile it first.")
    bridge = subprocess.Popen(
        [BRIDGE, side],
        cwd=os.path.dirname(BRIDGE),          # so $ORIGIN .so resolution works
        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, bufsize=1,
    )
    for line in bridge.stdout:                # wait for READY
        line = line.rstrip()
        print(f"[bridge] {line}", flush=True)
        if line == "READY":
            break
    else:
        raise SystemExit("bridge exited before READY -- is can0 up and the hand powered?")

    def drain():
        global latest_actual
        for ln in bridge.stdout:
            ln = ln.rstrip()
            if ln.startswith("POS "):
                try:
                    vals = [int(x) for x in ln.split()[1:7]]
                    if len(vals) == 6:
                        with state_lock:
                            latest_actual = vals
                except ValueError:
                    pass
            elif ln:                          # surface anything unexpected
                print(f"[bridge] {ln}", flush=True)
    threading.Thread(target=drain, daemon=True).start()


def _write(line):
    with bridge_lock:
        if bridge.poll() is not None:
            raise RuntimeError("bridge process has exited")
        bridge.stdin.write(line)
        bridge.stdin.flush()


def send_pose(vals):
    _write("P " + " ".join(str(int(v)) for v in vals) + "\n")


def send_torque(vals):
    _write("T " + " ".join(str(int(v)) for v in vals) + "\n")


class Handler(BaseHTTPRequestHandler):
    def log_message(self, *a):
        pass

    def _send(self, code, body, ctype="text/html; charset=utf-8"):
        data = body.encode() if isinstance(body, str) else body
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        if self.path == "/" or self.path.startswith("/?"):
            self._send(200, PAGE)
        elif self.path == "/state":
            with state_lock:
                a = latest_actual
            self._send(200, json.dumps({"actual": a}), "application/json")
        else:
            self._send(404, "not found", "text/plain")

    def do_POST(self):
        try:
            n = int(self.headers.get("Content-Length", 0))
            payload = json.loads(self.rfile.read(n) or b"{}")
            vals = payload["vals"]
            assert isinstance(vals, list) and len(vals) == 6
            vals = [max(0, min(255, int(v))) for v in vals]
            if self.path == "/pose":
                send_pose(vals)
            elif self.path == "/torque":
                send_torque(vals)
            else:
                return self._send(404, "not found", "text/plain")
            self._send(200, json.dumps({"ok": True, "vals": vals}), "application/json")
        except Exception as e:
            self._send(400, json.dumps({"ok": False, "error": str(e)}), "application/json")


PAGE = r"""<!doctype html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>LinkerHand O6 Control</title>
<style>
 :root{color-scheme:dark}
 body{font-family:system-ui,sans-serif;margin:0;background:#12141a;color:#e8eaed}
 .wrap{max-width:760px;margin:0 auto;padding:20px}
 h1{font-size:20px;margin:0 0 4px}
 .sub{color:#8b93a1;font-size:13px;margin:0 0 16px}
 .joint{padding:10px 12px;margin:8px 0;background:#1b1e26;border-radius:10px}
 .joint.trigger{background:#22303a;outline:1px solid #2f6f8f}
 .joint.stuck{outline:1px solid #c9762f;background:#2a2016}
 .jtop{display:grid;grid-template-columns:150px 1fr 96px;gap:12px;align-items:center}
 .joint label{font-size:14px}
 .joint .tag{display:block;font-size:11px;color:#8b93a1}
 input[type=range]{width:100%}
 .val{font-variant-numeric:tabular-nums;text-align:right;font-size:14px;color:#cdd3dc}
 .actual{font-variant-numeric:tabular-nums;font-size:12px;color:#8b93a1;text-align:right}
 .actual .d{color:#c9762f}
 .bar{height:5px;border-radius:3px;background:#2a2f3a;margin-top:7px;overflow:hidden}
 .bar>i{display:block;height:100%;background:#4a94b8;width:0%}
 .joint.stuck .bar>i{background:#c9762f}
 .row{display:flex;flex-wrap:wrap;gap:10px;margin:16px 0}
 button{font-size:14px;padding:10px 14px;border:0;border-radius:9px;cursor:pointer;background:#2a2f3a;color:#e8eaed}
 button:hover{background:#333a47}
 button.pri{background:#2f6f8f}button.pri:hover{background:#387fa3}
 .panel{background:#1b1e26;padding:12px 14px;border-radius:10px;margin:14px 0}
 .panel h3{margin:0 0 8px;font-size:14px}
 .panel .line{display:flex;gap:12px;align-items:center;flex-wrap:wrap;margin:6px 0}
 .panel input[type=number]{width:60px;background:#12141a;color:#e8eaed;border:1px solid #2a2f3a;border-radius:6px;padding:6px}
 .panel input[type=range]{width:220px}
 #status{font-size:13px;color:#8b93a1;min-height:18px;margin-top:8px}
 .saved .chip{display:inline-flex;gap:6px;align-items:center;background:#1b1e26;border-radius:20px;padding:4px 6px 4px 12px;margin:4px 6px 0 0;font-size:13px}
 .saved .chip button{padding:2px 8px;font-size:12px;border-radius:14px}
</style></head><body><div class="wrap">
<h1>LinkerHand O6 &mdash; live control</h1>
<p class="sub">Drag a slider to move that joint immediately. 255 = open/extended, 0 = fully bent.
The <b>index finger is the trigger finger</b>. The thin bar shows the joint's <b>actual</b> measured
position; a joint turns <span style="color:#c9762f">orange</span> when it can't reach its target (hit something).</p>

<div id="joints"></div>

<div class="panel">
  <h3>Trigger</h3>
  <div class="line">
    <label>pull depth <input type="number" id="depth" min="0" max="255" value="80"></label>
    <button class="pri" onclick="pull()">PULL TRIGGER</button>
    <button onclick="release()">RELEASE</button>
    <span class="sub">(moves only the index finger)</span>
  </div>
</div>

<div class="panel">
  <h3>Safety / torque</h3>
  <div class="line">
    <label>grip torque <input type="range" id="torque" min="0" max="255" value="200"></label>
    <span class="val" id="torqueVal" style="width:auto">200</span>
    <span class="sub">lower = gentler grip</span>
  </div>
  <div class="line">
    <label><input type="checkbox" id="autoease"> auto-ease stuck fingers</label>
    <label class="sub">gap threshold <input type="number" id="gapthr" min="5" max="200" value="35"></label>
    <span class="sub">drops torque on a finger that stalls against the drill, so it holds instead of straining</span>
  </div>
</div>

<div class="row">
  <button onclick="preset(OPEN)">Open hand</button>
  <button class="pri" onclick="preset(GRIP)">Drill grip (squeeze all but trigger)</button>
  <button onclick="savePose()">Save current pose</button>
</div>
<div id="saved" class="saved"></div>
<div id="status">ready</div>

<script>
const JOINTS=[
 {name:"Thumb curl",   tag:"index 0"},
 {name:"Thumb rotate", tag:"index 1"},
 {name:"Index (TRIGGER)",tag:"index 2", trigger:true},
 {name:"Middle",       tag:"index 3"},
 {name:"Ring",         tag:"index 4"},
 {name:"Little",       tag:"index 5"},
];
const OPEN=[255,104,255,255,255,255];
const GRIP=[255,255,255,0,0,0];       // squeeze middle/ring/little + thumb; index stays extended
const IDX=2;                          // trigger finger slider index
const EASE_TORQUE=60;                 // torque applied to a stalled finger when auto-ease is on
let vals=OPEN.slice();
let actual=[null,null,null,null,null,null];
let stuck=[0,0,0,0,0,0];              // consecutive polls a joint has been out of reach
let lastTorque=null;
const sliders=[],outs=[],acts=[],bars=[],cards=[];

const jd=document.getElementById('joints');
JOINTS.forEach((j,i)=>{
  const d=document.createElement('div');d.className='joint'+(j.trigger?' trigger':'');
  const top=document.createElement('div');top.className='jtop';
  top.innerHTML=`<label>${j.name}<span class="tag">${j.tag}</span></label>`;
  const s=document.createElement('input');s.type='range';s.min=0;s.max=255;s.value=vals[i];
  const o=document.createElement('div');o.className='val';o.textContent=vals[i];
  const a=document.createElement('div');a.className='actual';a.textContent='actual –';
  s.addEventListener('input',()=>{vals[i]=+s.value;o.textContent=s.value;sendThrottled();});
  const rcol=document.createElement('div');rcol.appendChild(o);rcol.appendChild(a);
  top.appendChild(s);top.appendChild(rcol);
  const bar=document.createElement('div');bar.className='bar';const fill=document.createElement('i');bar.appendChild(fill);
  d.appendChild(top);d.appendChild(bar);jd.appendChild(d);
  sliders.push(s);outs.push(o);acts.push(a);bars.push(fill);cards.push(d);
});

function refresh(){vals.forEach((v,i)=>{sliders[i].value=v;outs[i].textContent=v;});}
function status(t){document.getElementById('status').textContent=t;}

let timer=null,pending=false;
function sendThrottled(){ if(timer){pending=true;return;} sendPose(); timer=setTimeout(()=>{timer=null; if(pending){pending=false;sendThrottled();}},60); }
async function post(url,body){
  const r=await fetch(url,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});
  return r.json();
}
async function sendPose(){
  try{const j=await post('/pose',{vals});status(j.ok?('sent  ['+j.vals.join(', ')+']'):('error: '+j.error));}
  catch(e){status('network error: '+e);}
}
function preset(p){vals=p.slice();refresh();sendPose();}
function pull(){vals[IDX]=+document.getElementById('depth').value;refresh();sendPose();}
function release(){vals[IDX]=255;refresh();sendPose();}

// --- torque ---
const torqueEl=document.getElementById('torque'),torqueVal=document.getElementById('torqueVal');
function baseTorque(){return +torqueEl.value;}
async function sendTorque(vec){ if(JSON.stringify(vec)===JSON.stringify(lastTorque))return; lastTorque=vec.slice(); try{await post('/torque',{vals:vec});}catch(e){} }
torqueEl.addEventListener('input',()=>{torqueVal.textContent=torqueEl.value; if(!document.getElementById('autoease').checked) sendTorque(Array(6).fill(baseTorque()));});

// --- readback poll: update actual bars, detect stuck, apply auto-ease ---
async function poll(){
  try{
    const r=await fetch('/state');const j=await r.json();
    if(j.actual) actual=j.actual;
    const thr=+document.getElementById('gapthr').value;
    const ease=document.getElementById('autoease').checked;
    const tvec=[];
    for(let i=0;i<6;i++){
      const av=actual[i];
      if(av==null){acts[i].textContent='actual –';bars[i].style.width='0%';tvec.push(baseTorque());continue;}
      const gap=Math.abs(av-vals[i]);
      bars[i].style.width=(av/255*100).toFixed(0)+'%';
      const isStuck=gap>thr; stuck[i]=isStuck?stuck[i]+1:0;
      const persistent=stuck[i]>=3;
      cards[i].classList.toggle('stuck',persistent);
      acts[i].innerHTML='actual '+av+(gap>thr?' <span class="d">&Delta;'+gap+'</span>':'');
      tvec.push(ease&&persistent?EASE_TORQUE:baseTorque());
    }
    if(ease) sendTorque(tvec);
  }catch(e){}
  setTimeout(poll,150);
}

// --- save / recall poses ---
function loadSaved(){return JSON.parse(localStorage.getItem('o6poses')||'{}');}
function drawSaved(){
  const s=loadSaved(),box=document.getElementById('saved');box.innerHTML='';
  Object.keys(s).forEach(name=>{
    const c=document.createElement('span');c.className='chip';c.innerHTML=`<span>${name}</span>`;
    const a=document.createElement('button');a.textContent='apply';a.onclick=()=>preset(s[name]);
    const d=document.createElement('button');d.textContent='×';
    d.onclick=()=>{const o=loadSaved();delete o[name];localStorage.setItem('o6poses',JSON.stringify(o));drawSaved();};
    c.appendChild(a);c.appendChild(d);box.appendChild(c);
  });
}
function savePose(){const name=prompt('Name this pose:');if(!name)return;const s=loadSaved();s[name]=vals.slice();localStorage.setItem('o6poses',JSON.stringify(s));drawSaved();}

drawSaved();
sendPose();     // push initial open pose
poll();         // start readback loop
</script>
</div></body></html>
"""


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--side", choices=["left", "right"], default="left")
    ap.add_argument("--host", default="0.0.0.0")
    ap.add_argument("--port", type=int, default=8080)
    args = ap.parse_args()

    print(f"Starting O6 bridge (side={args.side})...", flush=True)
    start_bridge(args.side)
    srv = ThreadingHTTPServer((args.host, args.port), Handler)
    shown = "localhost" if args.host in ("0.0.0.0", "") else args.host
    print(f"\n  O6 control UI -> http://{shown}:{args.port}/\n", flush=True)
    if args.host == "0.0.0.0":
        print("  (bound on all interfaces -- reachable from other devices on your LAN)\n", flush=True)
    try:
        srv.serve_forever()
    except KeyboardInterrupt:
        print("\nshutting down", flush=True)
    finally:
        try:
            _write("Q\n")
        except Exception:
            pass


if __name__ == "__main__":
    main()
