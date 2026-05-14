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

  // Timestamp
  const sub = document.getElementById('sub-gate');
  if (data.updated_at) {
    const d = new Date(data.updated_at);
    sub.textContent = `Updated ${d.toLocaleTimeString()}`;
  }
}

/** Update the Firebase connection badge */
export function setWifiBadge(connected, ip = '') {
  const badge = document.getElementById('wifi-badge');
  badge.textContent = connected ? '● Live' : '○ Offline';
  badge.className   = 'pill ' + (connected ? 'live' : 'offline');
  if (ip) document.getElementById('ip-badge').textContent = ip;
}

/** Render the access log list */
export function updateLog(entries) {
  const list = document.getElementById('log-list');
  if (!entries.length) {
    list.innerHTML = '<li class="log-empty">No events yet.</li>';
    return;
  }
  list.innerHTML = entries.map(e => {
    const icon  = e.event === 'GRANTED' ? '✅' : e.event === 'DENIED' ? '❌' : '⚙️';
    const label = e.event === 'GRANTED'
      ? `Card ${formatUID(e.card)} — access granted`
      : e.event === 'DENIED'
      ? `Card ${formatUID(e.card)} — access denied`
      : `Manual override — ${e.event.toLowerCase()}`;
    const time  = e.time ? new Date(e.time).toLocaleTimeString() : '';
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
  dot.className = 'ble-dot' + (connected ? ' connected' : '');
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
