import Chart from 'chart.js/auto';

let _chart = null;

// Stores hourly counts keyed by hour string "HH"
const hourlyData = {};

export function initChart(canvasId) {
  const ctx    = document.getElementById(canvasId).getContext('2d');
  const labels = Array.from({ length: 24 }, (_, i) => `${String(i).padStart(2,'0')}:00`);
  const data   = Array(24).fill(0);

  _chart = new Chart(ctx, {
    type: 'line',
    data: {
      labels,
      datasets: [{
        label:           'Vehicles',
        data,
        borderColor:     '#5badff',
        backgroundColor: 'rgba(91,173,255,.08)',
        fill:            true,
        tension:         0.4,
        pointRadius:     0,
        borderWidth:     2,
      }],
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      animation: { duration: 400 },
      plugins: { legend: { display: false } },
      scales: {
        x: {
          grid:  { color: 'rgba(255,255,255,.05)' },
          ticks: { color: '#6b6a66', font: { size: 10 }, maxTicksLimit: 8, autoSkip: true },
          border: { display: false },
        },
        y: {
          min: 0,
          grid:  { color: 'rgba(255,255,255,.05)' },
          ticks: { color: '#6b6a66', font: { size: 10 }, stepSize: 2 },
          border: { display: false },
        },
      },
    },
  });
}

/** Call this on each Firebase update to record the current vehicle count */
export function recordDataPoint(vehiclesParked) {
  const hour = new Date().getHours();
  hourlyData[hour] = vehiclesParked;   // Latest reading for this hour

  if (!_chart) return;
  _chart.data.datasets[0].data = Array.from({ length: 24 }, (_, i) => hourlyData[i] ?? 0);
  _chart.update('none');
}
