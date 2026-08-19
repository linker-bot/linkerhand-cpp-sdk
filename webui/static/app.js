// LinkerHand Web 示教器前端 —— 原生 JS，无框架/CDN。
// 与后端契约：GET /meta、GET /state；POST /pose|/speed|/torque，body {vals:[..]} 或 {val:N}。
let META = {dof: 0, names: [], model: "?"};
let verReady = false, verTick = 0;   // 设备信息是否已渲染 + 补拉节拍
let vals = [];
const sliders = [], outs = [], monCells = [];

// ---- 国际化（中/英）----
const I18N = {
  zh: {
    doc_title: 'LinkerHand CPP Web',
    conn_title: '连接状态', lang_title: '切换语言', theme_title: '切换主题', set_title: '设置',
    tab_conn: '连接',
    tab_about: '关于', about_name: '灵心巧手',
    about_sdk: 'SDK 版本', about_site: '官网', about_desc: 'LinkerHand 系列 C++ SDK 的 Web 控制界面。',
    f_model: '型号', f_comm: '通信方式', f_side: '左右手', f_chan: '设备',
    side_left: '左手', side_right: '右手', connect: '连接', connecting: '连接中…',
    connected_ok: '连接成功', not_connected: '未连接——点击右上角 ⚙ 选择型号与通信方式后连接',
    disconnect: '断开连接', disconnecting: '断开中…', disconnected: '已断开',
    ver_info: '设备信息', joints_panel: '监控 & 控制', sp_tq_panel: '速度 & 扭矩',
    speed: '速度', torque: '扭矩', force_panel: '压感数据', rate: '频率',
    rate_pos: '位置', rate_st: '速度/扭矩', rate_force: '触觉', rate_temp: '温度', rate_fault: '故障码',
    no_force: '暂无触觉数据（该型号无传感器或未接入）。', ready: '就绪',
    h_joint: '关节', h_ctrl: '控制', h_target: '目标', h_pos: '实际',
    h_speed: '速度', h_torque: '扭矩', h_temp: '温度°C', h_fault: '故障码',
    joint: '关节', fingers: ['拇指', '食指', '中指', '无名指', '小指'], finger_fb: '指',
    palm: '掌心', peak: '峰值', apply: '应用',
    sent: '已下发', err: '错误', net_err: '网络错误', no_version: '暂无版本信息',
    ver: {'Device Serial': '序列号', 'Software Version': '软件版本',
          'Hardware Version': '硬件版本', 'Mechanical Version': '机械版本',
          'freedom': '自由度', 'Robot version': '机器人版本', 'Version Number': '版本号',
          'Hand direction': '手方向', 'Freedom': '自由度', 'RobotVersion': '机器人版本',
          'VersionNumber': '版本号', 'HandDirection': '手方向',
          'SoftwareVersion': '软件版本', 'HardwareVersion': '硬件版本'},
  },
  en: {
    doc_title: 'LinkerHand CPP Web',
    conn_title: 'Connection', lang_title: 'Switch language', theme_title: 'Toggle theme', set_title: 'Settings',
    tab_conn: 'Connection',
    tab_about: 'About', about_name: 'LinkerHand',
    about_sdk: 'SDK Version', about_site: 'Website', about_desc: 'Web control UI for the LinkerHand C++ SDK.',
    f_model: 'Model', f_comm: 'Comm', f_side: 'Hand', f_chan: 'Device',
    side_left: 'Left', side_right: 'Right', connect: 'Connect', connecting: 'Connecting…',
    connected_ok: 'Connected', not_connected: 'Not connected — click ⚙ to pick model & comm, then connect',
    disconnect: 'Disconnect', disconnecting: 'Disconnecting…', disconnected: 'Disconnected',
    ver_info: 'Device Info', joints_panel: 'Monitor & Control', sp_tq_panel: 'Speed & Torque',
    speed: 'Speed', torque: 'Torque', force_panel: 'Pressure Data', rate: 'Frequency',
    rate_pos: 'Position', rate_st: 'Speed/Torque', rate_force: 'Tactile', rate_temp: 'Temp', rate_fault: 'Fault',
    no_force: 'No tactile data (this model has no sensor or is not connected).', ready: 'Ready',
    h_joint: 'Joint', h_ctrl: 'Control', h_target: 'Target', h_pos: 'Actual',
    h_speed: 'Speed', h_torque: 'Torque', h_temp: 'Temp°C', h_fault: 'Fault',
    joint: 'Joint', fingers: ['Thumb', 'Index', 'Middle', 'Ring', 'Pinky'], finger_fb: 'F',
    palm: 'Palm', peak: 'Peak', apply: 'Apply',
    sent: 'Sent', err: 'Error', net_err: 'Network error', no_version: 'No version info',
    ver: {'Device Serial': 'Serial No.', 'Software Version': 'Firmware',
          'Hardware Version': 'Hardware', 'Mechanical Version': 'Mechanical',
          'freedom': 'DoF', 'Robot version': 'Robot Ver.', 'Version Number': 'Version',
          'Hand direction': 'Hand', 'Freedom': 'DoF', 'RobotVersion': 'Robot Ver.',
          'VersionNumber': 'Version', 'HandDirection': 'Hand',
          'SoftwareVersion': 'Firmware', 'HardwareVersion': 'Hardware'},
  },
};
let LANG = localStorage.getItem('lh_lang') || 'zh';
function t(k){ const d = I18N[LANG] || I18N.zh; return d[k] != null ? d[k] : (I18N.zh[k] != null ? I18N.zh[k] : k); }
function jointName(i){
  const arr = (LANG === 'en' && META.names_en) ? META.names_en : META.names;
  return (arr && arr[i]) || (t('joint') + i);
}

function status(t){ document.getElementById('status').textContent = t; }
async function post(url, body){
  const r = await fetch(url, {method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify(body)});
  return r.json();
}

// ---- 主题 ----
const themeBtn = document.getElementById('themeBtn');
function applyTheme(t){
  document.documentElement.dataset.theme = t;
  themeBtn.textContent = (t === 'light') ? '☀' : '🌙';
}
applyTheme(localStorage.getItem('lh_theme') || 'dark');
themeBtn.addEventListener('click', () => {
  const t = (document.documentElement.dataset.theme === 'light') ? 'dark' : 'light';
  localStorage.setItem('lh_theme', t);
  applyTheme(t);
});

// ---- 语言 ----
const langBtn = document.getElementById('langBtn');
function applyStatic(){
  document.documentElement.lang = LANG;
  document.title = t('doc_title');
  langBtn.textContent = (LANG === 'zh') ? 'EN' : 'CN';
  const logoEl = document.querySelector('.brand .logo');
  if(logoEl) logoEl.src = (LANG === 'en') ? 'en_logo.png' : 'logo.png';   // 中英各一套 logo
  document.querySelectorAll('[data-i18n]').forEach(el => { el.textContent = t(el.dataset.i18n); });
  document.querySelectorAll('[data-i18n-title]').forEach(el => { el.title = t(el.dataset.i18nTitle); });
}
function applyLang(lang){
  LANG = lang; localStorage.setItem('lh_lang', lang);
  applyStatic();
  if(META.connected && META.dof){
    buildJoints(); drawSaved();
    renderVersion(document.getElementById('infoVer'), META.version);
  }
  // 复位触觉/波形状态，让下一轮 poll 用新语言重画手指名与图例
  forceCanvases = []; palmCanvas = null; waveBuilt = false;
  status(t('ready'));
}
applyStatic();
langBtn.addEventListener('click', () => applyLang(LANG === 'zh' ? 'en' : 'zh'));

// ---- 连接设置弹窗 ----
const setModal = document.getElementById('setModal');
const mModel = document.getElementById('mModel'), mComm = document.getElementById('mComm');
const mSide = document.getElementById('mSide'), mChan = document.getElementById('mChan');
const connMsg = document.getElementById('connMsg');
const disconnectBtn = document.getElementById('disconnectBtn');
let OPTS = null;
const COMM_LABEL = {can: 'CAN', canfd: 'CAN-FD', modbus: 'Modbus', ethercat: 'EtherCAT'};
const CHAN_PH = {can: 'can0', canfd: 'socketcan:can0', modbus: '/dev/ttyUSB0'};

async function openSettings(){
  setModal.hidden = false;
  setActiveTab('conn');                      // 每次打开复位到“连接”页
  connMsg.textContent = ''; connMsg.className = 'conn-msg';
  if(!OPTS){
    try{ OPTS = await (await fetch('/options')).json(); }
    catch(e){ connMsg.textContent = t('net_err'); return; }
    mModel.innerHTML = '';
    OPTS.models.forEach(m => { const o = document.createElement('option'); o.value = o.textContent = m; mModel.appendChild(o); });
  }
  const cur = OPTS.current;
  disconnectBtn.hidden = !cur;               // 仅已连接时可断开
  if(cur && cur.model) mModel.value = cur.model;
  onModelChange();
  if(cur){ if(cur.comm) mComm.value = cur.comm; mSide.value = cur.side || 'left'; mChan.value = cur.channel || ''; onCommChange(); }
}
function closeSettings(){ setModal.hidden = true; }
// 设置弹窗内 TAB 切换：高亮所选 .tab、显隐对应 .tabpane；连接/断开按钮仅“连接”页显示。
function setActiveTab(name){
  setModal.querySelectorAll('.tab').forEach(b => b.classList.toggle('active', b.dataset.pane === name));
  setModal.querySelectorAll('.tabpane').forEach(p => { p.hidden = (p.dataset.pane !== name); });
  document.querySelector('.modal-foot').style.visibility = (name === 'conn') ? '' : 'hidden';
}
// 型号变更 -> 依 comm_support 重建通信下拉；EtherCAT 始终附加（选中连接时后端提示暂不支持）。
function onModelChange(){
  const comms = (OPTS.comm_support && OPTS.comm_support[mModel.value]) || ['can'];
  mComm.innerHTML = '';
  comms.concat(['ethercat']).forEach(c => {
    const o = document.createElement('option'); o.value = c; o.textContent = COMM_LABEL[c] || c; mComm.appendChild(o);
  });
  onCommChange();
}
function onCommChange(){ mChan.placeholder = CHAN_PH[mComm.value] || ''; }
async function doConnect(){
  connMsg.className = 'conn-msg'; connMsg.textContent = t('connecting');
  const body = {model: mModel.value, side: mSide.value, comm: mComm.value, channel: mChan.value.trim()};
  try{
    const j = await post('/connect', body);
    if(j.ok){
      connMsg.className = 'conn-msg ok'; connMsg.textContent = t('connected_ok');
      OPTS = null;              // 下次打开重新拉取 current
      closeSettings();
      await init();             // 按新型号/DOF 重建界面
    }else{
      connMsg.className = 'conn-msg'; connMsg.textContent = t('err') + ': ' + j.error;
    }
  }catch(e){ connMsg.textContent = t('net_err') + ': ' + e; }
}
async function doDisconnect(){
  connMsg.className = 'conn-msg'; connMsg.textContent = t('disconnecting');
  try{
    const j = await post('/disconnect', {});
    if(j.ok){
      OPTS = null;              // 下次打开重新拉取 current
      closeSettings();
      await init();             // 回到未连接态
    }else{
      connMsg.textContent = t('err') + ': ' + j.error;
    }
  }catch(e){ connMsg.textContent = t('net_err') + ': ' + e; }
}
document.getElementById('setBtn').addEventListener('click', openSettings);
document.getElementById('setClose').addEventListener('click', closeSettings);
setModal.querySelectorAll('.tab').forEach(b => b.addEventListener('click', () => setActiveTab(b.dataset.pane)));
setModal.addEventListener('click', e => { if(e.target === setModal) closeSettings(); });
mModel.addEventListener('change', onModelChange);
mComm.addEventListener('change', onCommChange);
document.getElementById('connectBtn').addEventListener('click', doConnect);
disconnectBtn.addEventListener('click', doDisconnect);

// ---- 关节监控与控制（合并为一张表：控制列持久，监控列仅更新单元格）----
function buildJoints(){
  const jd = document.getElementById('joints'); jd.innerHTML = '';
  sliders.length = 0; outs.length = 0; monCells.length = 0;
  const table = document.createElement('table'); table.className = 'mon jt';
  table.innerHTML = '<thead><tr><th>' + t('h_joint') + '</th><th>' + t('h_ctrl') + '</th><th class="num">' + t('h_target') + '</th>'
    + '<th class="num">' + t('h_pos') + '</th><th class="num">' + t('h_speed') + '</th><th class="num">' + t('h_torque') + '</th>'
    + '<th class="num">' + t('h_temp') + '</th><th class="num">' + t('h_fault') + '</th></tr></thead>';
  const tb = document.createElement('tbody');
  for(let i = 0; i < META.dof; i++){
    const tr = document.createElement('tr');
    const nameTd = document.createElement('td'); nameTd.textContent = jointName(i);
    const ctlTd = document.createElement('td'); ctlTd.className = 'ctl';
    const s = document.createElement('input'); s.type = 'range'; s.min = 0; s.max = 255; s.value = vals[i];
    ctlTd.appendChild(s);
    const valTd = document.createElement('td'); valTd.className = 'num'; valTd.textContent = vals[i];
    s.addEventListener('input', () => { vals[i] = +s.value; valTd.textContent = s.value; sendThrottled(); });
    const posTd = document.createElement('td'); posTd.className = 'num'; posTd.textContent = '--';
    const spdTd = document.createElement('td'); spdTd.className = 'num'; spdTd.textContent = '--';
    const trqTd = document.createElement('td'); trqTd.className = 'num'; trqTd.textContent = '--';
    const tempTd = document.createElement('td'); tempTd.className = 'num'; tempTd.textContent = '--';
    const faultTd = document.createElement('td'); faultTd.className = 'num'; faultTd.textContent = '--';
    tr.appendChild(nameTd); tr.appendChild(ctlTd); tr.appendChild(valTd);
    tr.appendChild(posTd); tr.appendChild(spdTd); tr.appendChild(trqTd); tr.appendChild(tempTd); tr.appendChild(faultTd);
    tb.appendChild(tr);
    sliders.push(s); outs.push(valTd);
    monCells.push({pos: posTd, spd: spdTd, trq: trqTd, temp: tempTd, fault: faultTd});
  }
  table.appendChild(tb); jd.appendChild(table);
}
function refresh(){ for(let i = 0; i < META.dof; i++){ sliders[i].value = vals[i]; outs[i].textContent = vals[i]; } }

let timer = null, pending = false;
function sendThrottled(){
  if(timer){ pending = true; return; }
  sendPose();
  timer = setTimeout(() => { timer = null; if(pending){ pending = false; sendThrottled(); } }, 60);
}
async function sendPose(){
  try{ const j = await post('/pose', {vals}); status(j.ok ? (t('sent') + ' [' + j.vals.join(', ') + ']') : (t('err') + ': ' + j.error)); }
  catch(e){ status(t('net_err') + ': ' + e); }
}

// ---- 速度 / 力矩 ----
const speedEl = document.getElementById('speed'), speedVal = document.getElementById('speedVal');
const torqueEl = document.getElementById('torque'), torqueVal = document.getElementById('torqueVal');
speedEl.addEventListener('input', () => { speedVal.textContent = speedEl.value; post('/speed', {val:+speedEl.value}); });
torqueEl.addEventListener('input', () => { torqueVal.textContent = torqueEl.value; post('/torque', {val:+torqueEl.value}); });

// ---- 回读频率（各通道 Hz）：驱动 web_bridge 分频轮询；force 通道决定 B5→B1 空档 ----
const rateEls = Array.from(document.querySelectorAll('input.rate'));
const rateTimers = {};
rateEls.forEach(el => {
  const chan = el.dataset.chan, out = el.parentElement.querySelector('.rval b');
  el.addEventListener('input', () => {
    out.textContent = el.value;
    clearTimeout(rateTimers[chan]);          // 拖动去抖，避免刷爆 /rate
    rateTimers[chan] = setTimeout(() => post('/rate', {chan, hz:+el.value}), 120);
  });
});

function themeIsDark(){ return document.documentElement.dataset.theme !== 'light'; }

// ---- 触觉热力图 ----
// 浅色：白(无压力)渐深到红。暗黑：深灰蓝(隐入面板)渐亮到红，不再是刺眼白块。
function heatColor(v){
  const t = Math.max(0, Math.min(1, v / 255));
  if(themeIsDark()){
    // 0 -> ≈--panel2 深底; 255 -> 亮红。红分量用 sqrt(t) 略陡,让低值段就泛红而非发灰。
    const s = Math.sqrt(t);
    const r = Math.round(28 + (255 - 28) * s);
    const g = Math.round(32 + (70 - 32) * s);
    const b = Math.round(44 + (70 - 44) * s);
    return `rgb(${r},${g},${b})`;
  }
  const g = Math.round(255 * (1 - t));
  return `rgb(255,${g},${g})`;
}
// 按背景亮度选字色，保证数值可读
function textColor(v){
  if(themeIsDark()) return '#e8eaed';   // 深底/红底均用浅字
  const t = Math.max(0, Math.min(1, v / 255));
  const g = 255 * (1 - t);
  return (0.299 * 255 + 0.701 * g) > 140 ? '#000' : '#fff';
}
function drawMatrix(canvas, mat, cellOverride){
  const rows = mat.length, cols = rows ? mat[0].length : 0;
  if(!rows || !cols) return;
  const cell = cellOverride || Math.max(20, Math.floor(140 / Math.max(rows, cols)));
  // 画布多留 1px，使最右/最下网格线（位于 cols*cell+0.5）完整落在画布内不被裁掉
  canvas.width = cols * cell + 1; canvas.height = rows * cell + 1;
  const ctx = canvas.getContext('2d');
  ctx.font = Math.floor(cell * 0.42) + 'px system-ui';
  ctx.textAlign = 'center'; ctx.textBaseline = 'middle';
  // 先逐格填色 + 写数值
  for(let r = 0; r < rows; r++) for(let c = 0; c < cols; c++){
    const v = mat[r][c] || 0;   // 没按压 -> 0
    const x = c * cell, y = r * cell;
    ctx.fillStyle = heatColor(v);
    ctx.fillRect(x, y, cell, cell);
    ctx.fillStyle = textColor(v);
    ctx.fillText(v, x + cell / 2, y + cell / 2);
  }
  // 再统一描一遍网格：每条边界线只画一次，内外线宽都恰为 1px，
  // 避免逐格 strokeRect 时相邻格边框并排叠成 2px、而外缘只有 1px
  ctx.strokeStyle = themeIsDark() ? '#2b3040' : '#bbb'; ctx.lineWidth = 1;
  ctx.beginPath();
  for(let c = 0; c <= cols; c++){ const x = c * cell + 0.5; ctx.moveTo(x, 0); ctx.lineTo(x, rows * cell + 1); }
  for(let r = 0; r <= rows; r++){ const y = r * cell + 0.5; ctx.moveTo(0, y); ctx.lineTo(cols * cell + 1, y); }
  ctx.stroke();
}
let forceCanvases = [];
let fingerCell = 0;   // 手指矩阵当前格子边长,供掌心复用以保持一致
// 依据容器可用宽度算出格子边长，让矩阵撑大填满并居中；窄屏取下限后由 flex-wrap 换行。
function fitCell(wrapW, count, cols, gap, min, max){
  if(!wrapW || !count || !cols) return min;
  const per = (wrapW - (count - 1) * gap) / count;
  return Math.max(min, Math.min(max, Math.floor(per / cols)));
}
function renderForce(force){
  const wrap = document.getElementById('forceWrap');
  const fingers = force && force.fingers;
  if(!fingers || !fingers.length){
    if(forceCanvases.length){ wrap.innerHTML = '<span class="muted">' + t('no_force') + '</span>'; forceCanvases = []; }
    return;
  }
  // 参考 C++ example：每指一块，名称标题在上、矩阵在下，按 拇→食→中→无名→小 横向排列。
  if(forceCanvases.length !== fingers.length){
    wrap.innerHTML = ''; forceCanvases = [];
    fingers.forEach((_, i) => {
      const block = document.createElement('div'); block.className = 'fblock';
      const lb = document.createElement('div'); lb.className = 'flabel'; lb.textContent = t('fingers')[i] || (t('finger_fb') + i);
      const cv = document.createElement('canvas');
      block.appendChild(lb); block.appendChild(cv); wrap.appendChild(block); forceCanvases.push(cv);
    });
  }
  const maxCols = fingers.reduce((m, f) => Math.max(m, f[0] ? f[0].length : 0), 1);
  const cell = fitCell(wrap.clientWidth, fingers.length, maxCols, 22, 20, 46);
  fingerCell = cell;
  fingers.forEach((m, i) => drawMatrix(forceCanvases[i], m, cell));
}
let palmCanvas = null;
function renderPalm(palm){
  const wrap = document.getElementById('palmWrap');
  const mat = palm && palm.palm;
  if(!mat || !mat.length){ if(palmCanvas){ wrap.innerHTML = ''; palmCanvas = null; } return; }
  if(!palmCanvas){
    wrap.innerHTML = '<div class="fblock"><div class="flabel">' + t('palm') + '</div><canvas></canvas></div>';
    palmCanvas = wrap.querySelector('canvas');
  }
  drawMatrix(palmCanvas, mat, fingerCell || undefined);   // 与手指矩阵格子尺寸保持一致
}

// ---- 压感波形：每指所有单元求和，随时间滚动；另叠加掌心合力（有掌心数据时）----
const WAVE_COLORS = ['#e6550d', '#3182bd', '#31a354', '#9467bd', '#e7298a'];
const PALM_COLOR = '#00c2c2';   // 掌心合力线，区别于 5 指暖色系
const WAVE_LEN = 180;   // ≈6s @30Hz
let forceHist = [];
let palmHist = [];      // 与 forceHist 等长；无掌心数据的帧为 null
let waveBuilt = false;
let waveHasPalm = false;   // 图例当前是否含掌心项
const matSum = m => m.reduce((a, row) => a + row.reduce((b, v) => b + (v || 0), 0), 0);
function buildWaveLegend(hasPalm){
  const lg = document.getElementById('waveLegend'); lg.innerHTML = '';
  t('fingers').forEach((n, i) => {
    const s = document.createElement('span');
    s.innerHTML = `<i style="background:${WAVE_COLORS[i]}"></i>${n}`;
    lg.appendChild(s);
  });
  if(hasPalm){
    const s = document.createElement('span');
    s.innerHTML = `<i style="background:${PALM_COLOR}"></i>${t('palm')}`;
    lg.appendChild(s);
  }
}
function renderWave(force, palm){
  const wrap = document.getElementById('waveWrap');
  const fingers = force && force.fingers;
  if(!fingers || !fingers.length){
    if(waveBuilt){ wrap.style.display = 'none'; forceHist = []; palmHist = []; waveBuilt = false; }
    return;
  }
  const palmMat = palm && palm.palm;
  const hasPalm = !!(palmMat && palmMat.length);
  if(!waveBuilt || hasPalm !== waveHasPalm){
    wrap.style.display = ''; buildWaveLegend(hasPalm); waveBuilt = true; waveHasPalm = hasPalm;
  }
  forceHist.push(fingers.map(matSum));
  palmHist.push(hasPalm ? matSum(palmMat) : null);
  if(forceHist.length > WAVE_LEN){ forceHist.shift(); palmHist.shift(); }
  drawWave();
}
function drawWave(){
  const cv = document.getElementById('wave');
  const w = cv.clientWidth || 600, h = 140;
  cv.width = w; cv.height = h;
  const ctx = cv.getContext('2d');
  ctx.clearRect(0, 0, w, h);
  const pad = 6;
  let maxV = 1;
  for(const row of forceHist) for(const v of row) if(v > maxV) maxV = v;
  for(const v of palmHist) if(v != null && v > maxV) maxV = v;
  // 网格底线
  ctx.strokeStyle = 'rgba(128,128,128,.25)'; ctx.lineWidth = 1;
  ctx.beginPath(); ctx.moveTo(0, h - pad); ctx.lineTo(w, h - pad); ctx.stroke();
  const n = forceHist.length;
  if(n >= 2){
    for(let f = 0; f < 5; f++){
      ctx.strokeStyle = WAVE_COLORS[f]; ctx.lineWidth = 1.5; ctx.beginPath();
      for(let i = 0; i < n; i++){
        const x = w * i / (n - 1);
        const y = (h - pad) - (forceHist[i][f] / maxV) * (h - 2 * pad);
        i ? ctx.lineTo(x, y) : ctx.moveTo(x, y);
      }
      ctx.stroke();
    }
    // 掌心合力线：仅画有数据的段，null 处断开
    if(palmHist.some(v => v != null)){
      ctx.strokeStyle = PALM_COLOR; ctx.lineWidth = 1.5; ctx.beginPath();
      let started = false;
      for(let i = 0; i < n; i++){
        const v = palmHist[i];
        if(v == null){ started = false; continue; }
        const x = w * i / (n - 1);
        const y = (h - pad) - (v / maxV) * (h - 2 * pad);
        started ? ctx.lineTo(x, y) : ctx.moveTo(x, y);
        started = true;
      }
      ctx.stroke();
    }
  }
  // 峰值量级
  ctx.fillStyle = 'rgba(128,128,128,.8)'; ctx.font = '11px system-ui';
  ctx.textAlign = 'right'; ctx.textBaseline = 'top';
  ctx.fillText(t('peak') + ' ' + maxV, w - 4, 3);
}

// ---- 关节监控：更新合并表里的监控单元格（不重建，避免打断控制滑块）----
function tempClass(v){ return v > 60 ? 'err' : (v > 50 ? 'warn' : 'ok'); }
function renderMonitor(pos, spd, trq, temp, fault){
  const get = (a, i) => (Array.isArray(a) && a[i] != null) ? a[i] : null;
  const setNum = (td, v) => { td.className = 'num'; td.textContent = v == null ? '--' : v; };
  for(let i = 0; i < monCells.length; i++){
    const c = monCells[i];
    const p = get(pos, i);
    // 实际值与目标偏差超过容差时标橙,提示尚未到位(容差避开反馈抖动导致的常亮)
    c.pos.textContent = p == null ? '--' : p;
    c.pos.className = (p != null && Math.abs(p - vals[i]) > 5) ? 'num warn' : 'num';
    setNum(c.spd, get(spd, i)); setNum(c.trq, get(trq, i));
    const tp = get(temp, i), f = get(fault, i);
    c.temp.textContent = tp == null ? '--' : tp;
    c.temp.className = tp == null ? 'num' : ('num ' + tempClass(tp));
    c.fault.textContent = f == null ? '--' : f;
    c.fault.className = f == null ? 'num' : ('num ' + (f ? 'err' : 'ok'));
  }
}

// ---- 轮询：关节反馈 + 触觉 ----
async function poll(){
  try{
    const j = await (await fetch('/state')).json();
    document.getElementById('conn').classList.add('on');
    renderForce(j.force);
    renderPalm(j.palm);
    renderWave(j.force, j.palm);
    renderMonitor(j.position, j.speed, j.torque, j.temperature, j.fault);
    // 设备信息可能刚上电时晚到：未就绪则每 ~1s 重拉 /meta（web_bridge 拿到后会补发 META）。
    if(META.connected && !verReady && (verTick++ % 30 === 0)){
      const m = await (await fetch('/meta')).json();
      if(m.version) META.version = m.version;
      verReady = renderVersion(document.getElementById('infoVer'), META.version);
    }
  }catch(e){ document.getElementById('conn').classList.remove('on'); }
  setTimeout(poll, 33);   // ~30Hz，跟上后端压感刷新率
}

// ---- 预设（localStorage）----
function key(){ return 'lh_' + META.model; }
function loadSaved(){ return JSON.parse(localStorage.getItem(key()) || '{}'); }
function drawSaved(){
  const s = loadSaved(), box = document.getElementById('saved'); box.innerHTML = '';
  Object.keys(s).forEach(name => {
    const c = document.createElement('span'); c.className = 'chip'; c.innerHTML = `<span>${name}</span>`;
    const a = document.createElement('button'); a.textContent = t('apply'); a.onclick = () => { vals = s[name].slice(); refresh(); sendPose(); };
    const d = document.createElement('button'); d.textContent = '×';
    d.onclick = () => { const o = loadSaved(); delete o[name]; localStorage.setItem(key(), JSON.stringify(o)); drawSaved(); };
    c.appendChild(a); c.appendChild(d); box.appendChild(c);
  });
}

// 版本串各字段以 ';' 分隔（web_bridge 保留字段边界），各型号字段不同（L7 有 freedom/
// Hand direction 等，值可含空格如 "Left hand"）。按 ';' 切段、每段首个 ':' 拆 key/value，
// 全部展示；缺失/Unknown 跳过，全无则占位。旧格式（无 ';'）退化为整串单段，尽量兜底。
function renderVersion(el, s){
  el.innerHTML = '';
  s = (s || '').trim();
  const rows = [];
  s.split(';').forEach(seg => {
    seg = seg.trim();
    const i = seg.indexOf(':');
    if(i < 0) return;
    const k = seg.slice(0, i).trim();
    const v = seg.slice(i + 1).trim();
    if(k && v && !/^unknown$/i.test(v)) rows.push([k, v]);
  });
  if(!rows.length){ el.innerHTML = '<span class="muted">' + t('no_version') + '</span>'; return false; }
  // 序列号(Device Serial)固定排到最前，其余字段保持原顺序。
  const si = rows.findIndex(r => r[0] === 'Device Serial');
  if(si > 0) rows.unshift(rows.splice(si, 1)[0]);
  rows.forEach(([k, v]) => {
    const d = document.createElement('div'); d.className = 'vrow';
    const ks = document.createElement('span'); ks.textContent = t('ver')[k] || k;
    const vb = document.createElement('b'); vb.textContent = v;
    d.appendChild(ks); d.appendChild(vb); el.appendChild(d);
  });
  return true;
}

let polling = false;
function showDisconnected(){
  document.getElementById('infoVer').innerHTML = '<span class="muted">' + t('not_connected') + '</span>';
  document.getElementById('joints').innerHTML = '<span class="muted">' + t('not_connected') + '</span>';
  document.getElementById('saved').innerHTML = '';
  sliders.length = 0; outs.length = 0; monCells.length = 0;
  verReady = false;
  status(t('not_connected'));
}
async function init(){
  META = await (await fetch('/meta')).json();
  if(META.rates) rateEls.forEach(el => {
    const v = META.rates[el.dataset.chan];
    if(v){ el.value = v; el.parentElement.querySelector('.rval b').textContent = v; }
  });
  if(!polling){ polling = true; poll(); }   // poll 循环单例，重连不重复启动
  if(!META.connected){
    showDisconnected();
    openSettings();                          // 未连接时自动弹出设置面板
    return;
  }
  verReady = renderVersion(document.getElementById('infoVer'), META.version);
  vals = new Array(META.dof).fill(255);
  buildJoints(); drawSaved();
  sendPose();   // 推初始张开姿势
}
init();
