/* ─── Tabs ───────────────────────────────────────────────────────── */
document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        const target = btn.dataset.tab;

        document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
        document.querySelectorAll('.tab-panel').forEach(p => p.classList.remove('active'));

        btn.classList.add('active');
        document.getElementById('tab-' + target).classList.add('active');
    });
});

/* ─── Clear alerts button ──────────────────────────────────────── */
let alertCount = 0;

document.getElementById('btnClearAlerts').addEventListener('click', () => {
    document.getElementById('alertFeed').innerHTML =
        '<div class="alert-item alert-stable">Feed limpiado. Esperando alertas...</div>';
    alertCount = 0;
    updateAlertBadge();
});

/* ─── Sensors — initial load + WebSocket broadcast ─────────────── */
let sensorCount = 0;

// Initial loading when opening the page (brings the current cache)
async function fetchSensorsOnce() {
    try {
        const res  = await fetch('/api/sensors');
        const data = await res.json();
        renderSensors(data);
    } catch (e) {
        console.warn('Error al obtener sensores iniciales:', e);
    }
}

const SENSOR_TYPES = ['temperature', 'energy', 'vibration'];

function renderSensors(sensors) {
    // Group by type
    const groups = { temperature: [], energy: [], vibration: [] };

    (sensors || []).forEach(s => {
        const type = (s.type || '').toLowerCase();
        if (groups[type]) groups[type].push(s);
    });

    // Render each group
    SENSOR_TYPES.forEach(type => {
        const tbody = document.getElementById('tbody-' + type);
        const count = groups[type].length;

        document.getElementById('count-' + type).textContent = count;

        if (count === 0) {
            tbody.innerHTML = '<tr><td colspan="3" class="empty-row">Sin sensores</td></tr>';
        } else {
            tbody.innerHTML = groups[type].map(s => `
                <tr>
                    <td>${escHtml(s.id)}</td>
                    <td class="value-cell">${escHtml(s.value)}</td>
                    <td class="unit-cell">${escHtml(s.unit)}</td>
                </tr>
            `).join('');
        }
    });

    sensorCount = (sensors || []).length;
    document.getElementById('sensorCount').textContent = sensorCount;
}

fetchSensorsOnce();

/* ─── WebSocket STOMP — real-time alerts ───────────────────── */
const stompClient = new StompJs.Client({
    webSocketFactory: () => new SockJS('/ws'),
    reconnectDelay: 5000,

    onConnect: () => {
        console.log('WebSocket conectado');

        // Sensors — server C broadcasts every 2 seconds
        stompClient.subscribe('/topic/sensors', msg => {
            const sensors = JSON.parse(msg.body);
            renderSensors(sensors);
            document.getElementById('lastUpdate').textContent =
                'Última actualización: ' + new Date().toLocaleTimeString('es-CO');
        });

        // Server C alerts
        stompClient.subscribe('/topic/alerts', msg => {
            addAlert(msg.body);
        });

        // Notification of lost connection with server C
        stompClient.subscribe('/topic/connection', msg => {
            if (msg.body === 'LOST') showConnectionLostToast();
        });
    },

    onDisconnect: () => console.warn('WebSocket desconectado'),
    onStompError:  frame => console.error('STOMP error:', frame)
});

stompClient.activate();

/* ─── Rendering alerts ─────────────────────────────────────── */

// Thresholds (same as the original AlertPanel.java)
const THRESHOLDS = {
    temperature: { warning: 30, critical: 50 },
    vibration:   { warning: 5,  critical: 10 },
    energy:      { warning: 500, critical: 800 }
};

function addAlert(rawLine) {
    // Format: "ALERT sensor_id,sensor_type,value,unit"
    const data = rawLine.startsWith('ALERTA ') ? rawLine.substring(7) : rawLine;
    const parts = data.split(',');

    const sensorId = parts[0]?.trim() ?? '?';
    const type     = parts[1]?.trim() ?? '?';
    const value    = parts[2]?.trim() ?? '?';
    const unit     = parts[3]?.trim() ?? '';

    const numVal   = parseFloat(value);
    const thresh   = THRESHOLDS[type];
    let level      = 'WARNING';

    if (thresh && !isNaN(numVal) && numVal > thresh.critical) level = 'CRITICAL';

    const timestamp = new Date().toLocaleString('es-CO', { hour12: false });
    const cssClass  = level === 'CRITICAL' ? 'alert-critical' : 'alert-warning';

    const item = document.createElement('div');
    item.className = `alert-item ${cssClass}`;
    item.textContent =
        `[${timestamp}] [${level}] Sensor ${sensorId} (${type}) | Valor: ${value} ${unit}`;

    const feed = document.getElementById('alertFeed');
    feed.appendChild(item);
    feed.scrollTop = feed.scrollHeight;

    alertCount++;
    updateAlertBadge();
}

function updateAlertBadge() {
    document.getElementById('alertCount').textContent = alertCount;
}

/* ─── Toast lost connection ─────────────────────────────────────── */
function showConnectionLostToast() {
    const dot   = document.querySelector('.dot');
    const badge = document.getElementById('connectionBadge');

    if (dot) {
        dot.classList.remove('dot-green');
        dot.classList.add('dot-red');
    }
    if (badge) badge.querySelector('span:last-child').textContent = 'Sin conexión';

    document.getElementById('toastConnectionLost').style.display = 'block';
}

/* ─── Utilities ─────────────────────────────────────────────────── */
function escHtml(str) {
    return String(str)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;');
}
