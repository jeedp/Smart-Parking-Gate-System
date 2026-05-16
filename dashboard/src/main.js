import { subscribeParkingState, subscribeLog }           from './firebase.js';
import { initChart, recordDataPoint }                    from './chart.js';
import { updateStats, updateLog, setWifiBadge, setBleConnected } from './ui.js';
import {
  isBleSupported, bleConnect,
  bleOpenGate, bleCloseGate, bleRefreshStatus,
} from './ble.js';

// ── Boot ──────────────────────────────────────────────────────────────────────
initChart('chart');
setWifiBadge(false);

// ── Firebase real-time listeners ──────────────────────────────────────────────
subscribeParkingState((data) => {
  updateStats(data);
  recordDataPoint(MAX_CAPACITY - (data.spaces_available ?? MAX_CAPACITY));
  setWifiBadge(true);
});

subscribeLog((entries) => {
  updateLog(entries);
});

// ── BLE ───────────────────────────────────────────────────────────────────────
const MAX_CAPACITY = 12;

if (!isBleSupported()) {
  const btn = document.getElementById('btn-ble-connect');
  btn.textContent = 'BLE not supported';
  btn.disabled    = true;
}

document.getElementById('btn-ble-connect').addEventListener('click', async () => {
  try {
    const deviceName = await bleConnect((gateOpen, spaces) => {
      // BLE notify received — update UI immediately without waiting for Firebase
      updateStats({
        gate_status:       gateOpen ? 'OPEN' : 'CLOSED',
        spaces_available:  spaces,
        total_entries:     parseInt(document.getElementById('val-entries').textContent) || 0,
        total_exits:       parseInt(document.getElementById('val-exits').textContent)   || 0,
      });
    });
    setBleConnected(true, deviceName);
  } catch (err) {
    console.error('[BLE] Connect failed:', err);
    alert('BLE connection failed: ' + err.message);
  }
});

document.getElementById('btn-open').addEventListener('click', async () => {
  try { await bleOpenGate(); }
  catch (e) { alert('BLE error: ' + e.message); }
});

document.getElementById('btn-close').addEventListener('click', async () => {
  try { await bleCloseGate(); }
  catch (e) { alert('BLE error: ' + e.message); }
});

document.getElementById('btn-refresh').addEventListener('click', async () => {
  try { await bleRefreshStatus(); }
  catch (e) { alert('BLE error: ' + e.message); }
});
