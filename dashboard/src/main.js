import { subscribeParkingState, subscribeLog } from './firebase.js';
import { initChart, recordDataPoint }          from './chart.js';
import { updateStats, updateLog, setWifiBadge, setBleConnected } from './ui.js';
import {
  isBleSupported, bleConnect,
  bleOpenGate, bleCloseGate, bleRefreshStatus, bleIsConnected,
} from './ble.js';

// ── Boot ──────────────────────────────────────────────────────────────────────
initChart('chart');
setWifiBadge(false);

// ── Firebase listeners ────────────────────────────────────────────────────────
subscribeParkingState((data) => {
  updateStats(data);
  recordDataPoint(12 - (data.spaces_available ?? 12));
  setWifiBadge(true);
});

subscribeLog((entries) => {
  updateLog(entries);
});

// ── BLE setup ─────────────────────────────────────────────────────────────────
if (!isBleSupported()) {
  document.getElementById('btn-ble-connect').textContent = 'BLE not supported';
  document.getElementById('btn-ble-connect').disabled    = true;
}

document.getElementById('btn-ble-connect').addEventListener('click', async () => {
  try {
    const name = await bleConnect((gateOpen, spaces) => {
      // BLE status notification received — update UI instantly
      updateStats({
        gate_status:       gateOpen ? 'OPEN' : 'CLOSED',
        spaces_available:  spaces,
        total_entries:     parseInt(document.getElementById('val-entries').textContent) || 0,
        total_exits:       parseInt(document.getElementById('val-exits').textContent)   || 0,
      });
    });
    setBleConnected(true, name);
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
