const SERVICE_UUID     = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const GATE_CHAR_UUID   = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
const STATUS_CHAR_UUID = 'cba1d466-344c-4be3-ab3f-189f80dd7518';

let gateCharacteristic   = null;
let statusCharacteristic = null;
let onStatusUpdate        = null;

export function isBleSupported() {
  return 'bluetooth' in navigator;
}

/**
 * Open the BLE device picker and connect to ParkingGate.
 * onStatus(gateOpen: bool, spaces: number) is called on every BLE notification.
 */
export async function bleConnect(onStatus) {
  if (!isBleSupported()) throw new Error('Web Bluetooth not supported in this browser.');
  onStatusUpdate = onStatus;

  const device = await navigator.bluetooth.requestDevice({
    filters: [{ name: 'ParkingGate' }],
    optionalServices: [SERVICE_UUID],
  });

  device.addEventListener('gattserverdisconnected', () => {
    gateCharacteristic   = null;
    statusCharacteristic = null;
    console.warn('[BLE] Disconnected');
  });

  const server  = await device.gatt.connect();
  const service = await server.getPrimaryService(SERVICE_UUID);

  gateCharacteristic   = await service.getCharacteristic(GATE_CHAR_UUID);
  statusCharacteristic = await service.getCharacteristic(STATUS_CHAR_UUID);

  // Subscribe to status notifications from ESP32
  await statusCharacteristic.startNotifications();
  statusCharacteristic.addEventListener('characteristicvaluechanged', (e) => {
    const raw    = new TextDecoder().decode(e.target.value);   // "OPEN,8" or "CLOSED,5"
    const [status, spaces] = raw.split(',');
    if (onStatusUpdate) onStatusUpdate(status === 'OPEN', parseInt(spaces));
  });

  return device.name;
}

/** Send a command string to the gate characteristic */
async function sendCommand(cmd) {
  if (!gateCharacteristic) throw new Error('BLE not connected');
  await gateCharacteristic.writeValue(new TextEncoder().encode(cmd));
}

export const bleOpenGate    = () => sendCommand('OPEN');
export const bleCloseGate   = () => sendCommand('CLOSE');
export const bleRefreshStatus = () => sendCommand('REFRESH');
export const bleIsConnected = () => gateCharacteristic !== null;
