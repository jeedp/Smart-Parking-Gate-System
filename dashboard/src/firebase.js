import { initializeApp } from 'firebase/app';
import { getDatabase, ref, onValue, query, limitToLast } from 'firebase/database';

// ── Replace these with your actual Firebase project config ────────────────────
const firebaseConfig = {
  apiKey:             import.meta.env.VITE_FIREBASE_API_KEY,
  authDomain:         import.meta.env.VITE_FIREBASE_AUTH_DOMAIN,
  databaseURL:        import.meta.env.VITE_FIREBASE_DATABASE_URL,
  projectId:          import.meta.env.VITE_FIREBASE_PROJECT_ID,
  storageBucket:      import.meta.env.VITE_FIREBASE_STORAGE_BUCKET,
  messagingSenderId:  import.meta.env.VITE_FIREBASE_MESSAGING_SENDER_ID,
  appId:              import.meta.env.VITE_FIREBASE_APP_ID,
  measurementId:      import.meta.env.VITE_FIREBASE_MEASUREMENT_ID,
};

const app = initializeApp(firebaseConfig);
const db  = getDatabase(app);

/**
 * Subscribe to /parking node.
 * callback(data) fired immediately and on every ESP32 push.
 */
export function subscribeParkingState(callback) {
  const parkingRef = ref(db, '/parking');
  onValue(parkingRef, (snapshot) => {
    const data = snapshot.val();
    if (data) callback(data);
  });
}
 
/**
 * Subscribe to the last 20 log entries only.
 * callback(entries[]) called with newest-first sorted array.
 * limitToLast prevents downloading the entire /log node on every change.
 */
export function subscribeLog(callback) {
  const logQuery = query(ref(db, '/log'), limitToLast(20));
  onValue(logQuery, (snapshot) => {
    const raw = snapshot.val();
    if (!raw) return;
    const entries = Object.values(raw)
      .sort((a, b) => (b.time ?? 0) - (a.time ?? 0));
    callback(entries);
  });
}