const SERVICE_UUID     = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const GATE_CHAR_UUID   = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
const STATUS_CHAR_UUID = 'cba1d466-344c-4be3-ab3f-189f80dd7518';

let gateCharacteristic   = null;
let statusCharacteristic = null;
let _device              = null;   // Keep reference for disconnect
let onStatusUpdate       = null;
let onDisconnectCb       = null;   // Called when connection drops

/**
 * Open the BLE device picker and connect to ParkingGate.
 * onStatus(gateOpen: bool, spaces: number) called on every ESP32 notify.
 * onDisconnect() called when the connection drops for any reason.
 */
export async function bleConnect(onStatus, onDisconnect) {
  if (!isBleSupported()) throw new Error('Web Bluetooth not supported in this browser.');
  onStatusUpdate = onStatus;
  onDisconnectCb = onDisconnect;

  _device = await navigator.bluetooth.requestDevice({
    filters: [{ name: 'ParkingGate' }],
    optionalServices: [SERVICE_UUID],
  });

  _device.addEventListener('gattserverdisconnected', () => {
    gateCharacteristic   = null;
    statusCharacteristic = null;
    _device              = null;
    console.warn('[BLE] Disconnected');
    if (onDisconnectCb) onDisconnectCb();
  });

  const server  = await _device.gatt.connect();
  const service = await server.getPrimaryService(SERVICE_UUID);

  gateCharacteristic   = await service.getCharacteristic(GATE_CHAR_UUID);
  statusCharacteristic = await service.getCharacteristic(STATUS_CHAR_UUID);

  // Subscribe to status notifications — fires after every OPEN/CLOSE command
  await statusCharacteristic.startNotifications();
  statusCharacteristic.addEventListener('characteristicvaluechanged', (e) => {
    const raw              = new TextDecoder().decode(e.target.value); // "OPEN,8" or "CLOSED,5"
    const [status, spaces] = raw.split(',');
    if (onStatusUpdate) onStatusUpdate(status === 'OPEN', parseInt(spaces));
  });

  return _device.name;
}

/** Send a command string to the gate characteristic */
async function sendCommand(cmd) {
  if (!gateCharacteristic) throw new Error('BLE not connected');
  await gateCharacteristic.writeValue(new TextEncoder().encode(cmd));
}

export const bleOpenGate  = () => sendCommand('OPEN');
export const bleCloseGate = () => sendCommand('CLOSE');

/**
 * Disconnect from the ESP32.
 * Sends a DISCONNECT command first so the ESP32 can clean up,
 * then drops the GATT connection from the browser side.
 */
export async function bleDisconnect() {
  try {
    // Tell ESP32 first so it can restart advertising cleanly
    await sendCommand('DISCONNECT');
    // Small delay to let ESP32 process before browser drops connection
    await new Promise(r => setTimeout(r, 300));
  } catch (_) {
    // If send fails the connection is already gone — just disconnect
  }
  if (_device?.gatt?.connected) {
    _device.gatt.disconnect();
  }
}

export const isBleSupported = () => 'bluetooth' in navigator;
export const bleIsConnected = () => gateCharacteristic !== null;
