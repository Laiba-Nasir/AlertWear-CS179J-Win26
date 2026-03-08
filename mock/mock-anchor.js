const mqtt = require('mqtt');

// Connect to MQTT broker
const client = mqtt.connect('mqtt://localhost:1883');

// Simulated worker data (matches your config)
const workers = [
    { tagId: 1, fullName: 'John Smith', empId: 'EMP-001' },
    { tagId: 2, fullName: 'Maria Garcia', empId: 'EMP-002' },
    { tagId: 3, fullName: 'Bob Wilson', empId: 'EMP-003' },
    { tagId: 4, fullName: 'Alex Chen', empId: 'EMP-004' }
];

// Simulated anchor positions (must match anchors.json IDs and coordinates)
const anchors = [
    { anchorId: '0', x: 0, y: 0 },
    { anchorId: '1', x: 7.5, y: 0 },
    { anchorId: '2', x: 7.5, y: 7.5 },
    { anchorId: '3', x: 0, y: 7.5 }
];

// Simulate tag positions (will move randomly)
const tagPositions = [
    { x: 3, y: 4 },
    { x: 7, y: 3 },
    { x: 5, y: 7 },
    { x: 8, y: 8 }
];

// Calculate distance between two points
function getDistance(x1, y1, x2, y2) {
    return Math.sqrt(Math.pow(x2 - x1, 2) + Math.pow(y2 - y1, 2));
}

// Generate random gas reading (normal: 5-20, warning: 50-70, alert: 80-100)
function getGasReading(tagId) {
    // Tag 4 (Alex Chen) will occasionally have high gas readings
    if (tagId === 4 && Math.random() < 0.3) {
        return Math.floor(Math.random() * 20) + 80; // 80-100 ppm (ALERT)
    }
    return Math.floor(Math.random() * 15) + 5; // 5-20 ppm (normal)
}

// Move tags slightly (simulate walking)
function moveTags() {
    tagPositions.forEach((pos) => {
        pos.x += (Math.random() - 0.5) * 0.5; // Move ±0.25m
        pos.y += (Math.random() - 0.5) * 0.5;
        // Keep within bounds
        pos.x = Math.max(0.5, Math.min(9.5, pos.x));
        pos.y = Math.max(0.5, Math.min(9.5, pos.y));
    });
}

client.on('connect', () => {
    console.log('✅ Mock anchor connected to MQTT broker');
    console.log('📡 Publishing simulated UWB data every 1 second...\n');

    // Publish data every second
    setInterval(() => {
        moveTags();

        workers.forEach((worker, index) => {
            const tagPos = tagPositions[index];
            const gas = getGasReading(worker.tagId);

            // Each anchor "sees" this tag and reports distance
            anchors.forEach((anchor) => {
                const distance = getDistance(tagPos.x, tagPos.y, anchor.x, anchor.y);

                const payload = {
                    tagId: worker.tagId,
                    fullName: worker.fullName,
                    empId: worker.empId,
                    distance: parseFloat(distance.toFixed(2)),
                    gas: gas,
                    anchorId: anchor.anchorId,
                    anchorX: anchor.x,
                    anchorY: anchor.y,
                    timestamp: new Date().toISOString()
                };

                const topic = `uwb/anchor/${anchor.anchorId}/data`;
                client.publish(topic, JSON.stringify(payload));
            });
        });

        console.log(`📨 Published data for ${workers.length} tags from ${anchors.length} anchors`);
    }, 1000);
});

client.on('error', (err) => {
    console.error('MQTT error:', err);
});