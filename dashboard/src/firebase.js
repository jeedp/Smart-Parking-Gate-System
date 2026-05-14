import { initializeApp }             from 'firebase/app';
import { getDatabase, ref, onValue, push } from 'firebase/database';

// ── Replace these with your actual Firebase project config ────────────────────
const firebaseConfig = {
  apiKey: "AIzaSyANEm_QXOkhhC6JlH6YqkuUgUkcChr_qCQ",
  authDomain: "smart-parking-gate-system.firebaseapp.com",
  databaseURL: "https://smart-parking-gate-system-default-rtdb.asia-southeast1.firebasedatabase.app",
  projectId: "smart-parking-gate-system",
  storageBucket: "smart-parking-gate-system.firebasestorage.app",
  messagingSenderId: "878509472011",
  appId: "1:878509472011:web:988b1f1afbca1327eb0a0e",
  measurementId: "G-FLFGY2L51S"
};

const app = initializeApp(firebaseConfig);
const db  = getDatabase(app);

/**
 * Subscribe to /parking node.
 * callback(data) is called immediately and on every change.
 */
export function subscribeParkingState(callback) {
  const parkingRef = ref(db, '/parking');
  onValue(parkingRef, (snapshot) => {
    const data = snapshot.val();
    if (data) callback(data);
  });
}

/**
 * Subscribe to /log node.
 * callback(entries[]) called with the last 20 log entries, newest first.
 */
export function subscribeLog(callback) {
  const logRef = ref(db, '/log');
  onValue(logRef, (snapshot) => {
    const raw = snapshot.val();
    if (!raw) return;
    const entries = Object.values(raw)
      .sort((a, b) => b.time - a.time)
      .slice(0, 20);
    callback(entries);
  });
}
