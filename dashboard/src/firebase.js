import { initializeApp }                                   from 'firebase/app';
import { getDatabase, ref, onValue, query, limitToLast }   from 'firebase/database';

// ── Config is read from .env (VITE_ prefix required for Vite to expose vars) ──
// Copy .env.example → .env and fill in your Firebase project values.
// Get them from: Firebase Console → Project Settings → Your apps → SDK setup
const firebaseConfig = {
  apiKey:            import.meta.env.VITE_FIREBASE_API_KEY,
  authDomain:        import.meta.env.VITE_FIREBASE_AUTH_DOMAIN,
  databaseURL:       import.meta.env.VITE_FIREBASE_DATABASE_URL,
  projectId:         import.meta.env.VITE_FIREBASE_PROJECT_ID,
  storageBucket:     import.meta.env.VITE_FIREBASE_STORAGE_BUCKET,
  messagingSenderId: import.meta.env.VITE_FIREBASE_MESSAGING_SENDER_ID,
  appId:             import.meta.env.VITE_FIREBASE_APP_ID,
};

// Guard: warn clearly if .env is missing instead of a cryptic Firebase error
const missing = Object.entries(firebaseConfig)
  .filter(([, v]) => !v || v === 'undefined')
  .map(([k]) => k);

if (missing.length > 0) {
  console.error(
    '[Firebase] Missing env vars:', missing,
    '\nCopy .env.example → .env and fill in your Firebase project values.'
  );
}

const app = initializeApp(firebaseConfig);
const db  = getDatabase(app);

/**
 * Subscribe to /parking node.
 * callback(data) fired immediately and on every ESP32 push.
 */
export function subscribeParkingState(callback) {
  const parkingRef = ref(db, '/parking');
  onValue(
    parkingRef,
    (snapshot) => {
      const data = snapshot.val();
      if (data) callback(data);
    },
    (error) => {
      console.error('[Firebase] subscribeParkingState error:', error.message);
    }
  );
}

/**
 * Subscribe to the last 20 log entries only.
 * limitToLast prevents downloading the full /log on every change.
 */
export function subscribeLog(callback) {
  const logQuery = query(ref(db, '/log'), limitToLast(20));
  onValue(
    logQuery,
    (snapshot) => {
      const raw = snapshot.val();
      if (!raw) return;
      const entries = Object.values(raw)
        .sort((a, b) => (b.time ?? 0) - (a.time ?? 0));
      callback(entries);
    },
    (error) => {
      console.error('[Firebase] subscribeLog error:', error.message);
    }
  );
}
