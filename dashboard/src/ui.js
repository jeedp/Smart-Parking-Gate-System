const MAX_CAPACITY = 12;

/** Update the four stat cards from a ParkingState snapshot */
export function updateStats(data) {
  setText('val-spaces',  data.spaces_available ?? '—');
  setText('val-entries', data.total_entries    ?? '—');
  setText('val-exits',   data.total_exits      ?? '—');

  const gateEl = document.getElementById('val-gate');
  const isOpen = data.gate_status === 'OPEN';
  gateEl.textContent = data.gate_status ?? '—';
  gateEl.className   = 'stat-value gate-val ' + (isOpen ? 'open' : 'closed');

  const used = MAX_CAPACITY - (data.spaces_available ?? MAX_CAPACITY);
  const pct  = Math.round(used / MAX_CAPACITY * 100);
  document.getElementById('occ-fill').style.width = pct + '%';
  document.getElementById('occ-tag').textContent  = `${pct}% occupied · ${used}/${MAX_CAPACITY} spaces`;

  const sub = document.getElementById('sub-gate');
  if (data.updated_at) {
    sub.textContent = `Updated ${new Date(data.updated_at).toLocaleTimeString()}`;
  }
}

/** Update the Firebase connection badge */
export function setWifiBadge(connected, ip = '') {
  const badge = document.getElementById('wifi-badge');
  badge.textContent = connected ? '● Live' : '○ Offline';
  badge.className   = 'pill ' + (connected ? 'live' : 'offline');
  if (ip) document.getElementById('ip-badge').textContent = ip;
}

// ── Event display map ─────────────────────────────────────────────────────────
// Maps every last_event value from firmware to { icon, label() }
const EVENT_MAP = {
  'ENTRY GRANTED': { icon: '✅', label: (e) => `Card ${formatUID(e.card)} — entry granted` },
  'GRANTED':       { icon: '✅', label: (e) => `Card ${formatUID(e.card)} — entry granted` },
  'ENTRY DENIED':  { icon: '❌', label: (e) => `Card ${formatUID(e.card)} — access denied` },
  'DENIED':        { icon: '❌', label: (e) => `Card ${formatUID(e.card)} — access denied` },
  'ENTERED':       { icon: '🚗', label: ()  => `Vehicle entered — gate closed` },
  'TIMEOUT CLOSE': { icon: '⏱️', label: ()  => `Gate closed after timeout` },
  'EXIT DETECT':   { icon: '🔍', label: ()  => `Vehicle detected at exit gate` },
  'EXIT':          { icon: '🚗', label: ()  => `Vehicle exited — gate closed` },
  'EXITED':        { icon: '🚗', label: ()  => `Vehicle exited — gate closed` },
  'EXIT TIMEOUT':  { icon: '⏱️', label: ()  => `Exit gate closed after timeout` },
  'MANUAL OPEN':   { icon: '🔘', label: ()  => `Manual override — gate opened` },
  'MANUAL CLOSE':  { icon: '🔘', label: ()  => `Manual override — gate closed` },
  'BLE OPEN':      { icon: '📱', label: ()  => `BLE command — gate opened` },
  'BLE CLOSE':     { icon: '📱', label: ()  => `BLE command — gate closed` },
  'INIT':          { icon: 'ℹ️', label: ()  => `System initialized` },
};

/** Render the access log list */
export function updateLog(entries) {
  const list = document.getElementById('log-list');
  if (!entries || !entries.length) {
    list.innerHTML = '<li class="log-empty">No events yet.</li>';
    return;
  }
  list.innerHTML = entries.map(e => {
    const ev      = (e.event ?? '').toUpperCase();
    const mapping = EVENT_MAP[ev] ?? { icon: '⚙️', label: () => e.event ?? 'Unknown event' };
    const icon    = mapping.icon;
    const label   = mapping.label(e);
    const time    = e.time ? new Date(e.time).toLocaleTimeString() : '';
    return `<li class="log-item">
      <span class="log-icon">${icon}</span>
      <span class="log-text">${label}</span>
      <span class="log-time">${time}</span>
    </li>`;
  }).join('');
}

/** Update BLE connection UI */
export function setBleConnected(connected, deviceName = '') {
  const dot  = document.getElementById('ble-dot');
  const name = document.getElementById('ble-name');
  const info = document.getElementById('ble-info');
  dot.className    = 'ble-dot' + (connected ? ' connected' : '');
  name.textContent = connected ? deviceName : 'Not connected';
  info.textContent = connected ? 'BLE active' : 'Web Bluetooth';

  const disabled = !connected;
  document.getElementById('btn-open').disabled    = disabled;
  document.getElementById('btn-close').disabled   = disabled;
  document.getElementById('btn-refresh').disabled = disabled;
}

// ── Helpers ───────────────────────────────────────────────────────────────────
function setText(id, value) {
  const el = document.getElementById(id);
  if (el) el.textContent = value;
}

function formatUID(uid = '') {
  return uid.match(/.{2}/g)?.join(':') ?? uid;
}
