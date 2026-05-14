import { initializeApp }             from 'firebase/app';
import { getDatabase, ref, onValue, push } from 'firebase/database';

// ── Replace these with your actual Firebase project config ────────────────────
const firebaseConfig = {
  apiKey:            "YOUR_API_KEY",
  authDomain:        "your-project.firebaseapp.com",
  databaseURL:       "https://your-project-default-rtdb.firebaseio.com",
  projectId:         "your-project",
  storageBucket:     "your-project.appspot.com",
  messagingSenderId: "YOUR_SENDER_ID",
  appId:             "YOUR_APP_ID",
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
