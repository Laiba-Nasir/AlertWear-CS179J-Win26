const mqtt = require('mqtt');

// Connect to MQTT broker
client = mqtt.connect('mqtt://localhost:1883');

// Anchors (match anchors.json)
const anchors = [
    { anchorId: 0, x: 0, y: 0 },
    { anchorId: 1, x: 7.5, y: 0 },
    { anchorId: 2, x: 7.5, y: 7.5 },
    { anchorId: 3, x: 0, y: 7.5 }
];

// Workers (match workers.json)
const workers = [
    { tagId: 0, fullName: 'Test Worker', empId: 'EMP-000' },
    { tagId: 1, fullName: 'John Smith', empId: 'EMP-001' },
    { tagId: 2, fullName: 'Maria Garcia', empId: 'EMP-002' },
    { tagId: 3, fullName: 'Bob Wilson', empId: 'EMP-003' },
    { tagId: 4, fullName: 'Alex Chen', empId: 'EMP-004' }
];

// Simulated tag positions (will drift randomly)
const tagPositions = {
    0: { x: 2, y: 3 },
    1: { x: 5, y: 2 },
    2: { x: 6, y: 5 },
    3: { x: 1, y: 6 },
    4: { x: 4, y: 4 }
};

// Calculate distance between two points with slight noise
function getDistance(x1, y1, x2, y2) {
    const real = Math.sqrt(Math.pow(x2 - x1, 2) + Math.pow(y2 - y1, 2));
    const noise = (Math.random() - 0.5) * 0.2; // ±0.1m noise
    return Math.max(0.1, real + noise);
}

// Move tags slightly (simulate walking)
function moveTags() {
    for (const tagId in tagPositions) {
        const pos = tagPositions[tagId];
        pos.x += (Math.random() - 0.5) * 0.3;
        pos.y += (Math.random() - 0.5) * 0.3;
        pos.x = Math.max(0.5, Math.min(7.0, pos.x));
        pos.y = Math.max(0.5, Math.min(7.0, pos.y));
    }
}

// Generate gas sensor readings
function getGasReading(tagId) {
    // Tag 4 (Alex Chen) occasionally has dangerous gas levels
    if (tagId === 4 && Math.random() < 0.3) {
        return {
            temperature: 28 + Math.random() * 5,
            humidity: 60 + Math.random() * 20,
            pressure: 1010 + Math.random() * 10,
            gas: 80 + Math.random() * 40  // high gas resistance = alert
        };
    }
    return {
        temperature: 22 + Math.random() * 4,
        humidity: 40 + Math.random() * 15,
        pressure: 1012 + Math.random() * 5,
        gas: 5 + Math.random() * 15  // normal range
    };
}

client.on('connect', () => {
    console.log('✅ Mock VERTEX connected to MQTT broker');
    console.log('📡 Publishing simulated data every 600ms...\n');

    // Publish anchor status
    anchors.forEach(a => {
        client.publish(`uwb/anchor/${a.anchorId}/status`, 'online', { retain: true });
    });

    // Publish distance data every 600ms (matches real system rate)
    setInterval(() => {
        moveTags();

        workers.forEach(worker => {
            const pos = tagPositions[worker.tagId];

            anchors.forEach(anchor => {
                const distance = getDistance(pos.x, pos.y, anchor.x, anchor.y);

                const payload = {
                    tagId: worker.tagId,
                    distance: parseFloat(distance.toFixed(2)),
                    rssi: 0,
                    anchorId: anchor.anchorId,
                    anchorX: anchor.x,
                    anchorY: anchor.y,
                    timestamp: Date.now()
                };

                client.publish(`uwb/anchor/${anchor.anchorId}`, JSON.stringify(payload));
            });
        });
    }, 600);

    // Publish gas data every 5 seconds (matches BME680 read interval)
    setInterval(() => {
        workers.forEach(worker => {
            const gasData = getGasReading(worker.tagId);

            const payload = {
                tagId: worker.tagId,
                temperature: parseFloat(gasData.temperature.toFixed(1)),
                humidity: parseFloat(gasData.humidity.toFixed(1)),
                pressure: parseFloat(gasData.pressure.toFixed(1)),
                gas: parseFloat(gasData.gas.toFixed(1))
            };

            client.publish(`uwb/tag/${worker.tagId}/gas`, JSON.stringify(payload));
        });

        console.log(`🌡️  Published gas data for ${workers.length} workers`);
    }, 5000);
});

client.on('error', (err) => {
    console.error('MQTT error:', err);
});