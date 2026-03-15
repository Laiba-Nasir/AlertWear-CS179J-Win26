import mqtt from 'mqtt';
import { processReading, markAnchorOnline } from './store';

// Store for debug messages
export const messages: { timestamp: string; topic: string; payload: string }[] = [];

// MQTT Client
let client: mqtt.MqttClient | null = null;

export function connectMQTT() {
    if (client) return client;

    // Use environment variable or default to home WiFi IP
    const brokerIp = process.env.VITE_MQTT_BROKER_IP || 'localhost';
    const brokerPort = process.env.VITE_MQTT_PORT || '1883';
    const brokerUrl = `mqtt://${brokerIp}:${brokerPort}`;

    console.log(`🔌 Connecting to MQTT broker at ${brokerUrl}`);
    client = mqtt.connect(brokerUrl);

    client.on('connect', () => {
        console.log('✅ Connected to MQTT broker');
        client!.subscribe('uwb/#', (err) => {
            if (err) {
                console.error('Subscribe error:', err);
            } else {
                console.log('✅ Subscribed to all topics (uwb/#)');
            }
        });
    });

    client.on('message', (topic, payload) => {
        const payloadStr = payload.toString();
        
        // Store for debug console
        const msg = {
            timestamp: new Date().toISOString(),
            topic,
            payload: payloadStr
        };
        messages.unshift(msg);
        if (messages.length > 100) {
            messages.pop();
        }

        // Handle anchor status heartbeats (e.g. uwb/anchor/2/status → "online")
        const statusMatch = topic.match(/^uwb\/anchor\/(\d+)\/status$/);
        if (statusMatch) {
            markAnchorOnline(statusMatch[1]);
            return;
        }

        // Process UWB data from anchors (JSON with distance + gas data)
        if (topic.startsWith('uwb/')) {
            try {
                const data = JSON.parse(payloadStr);
                console.log(`📡 Range data: anchor=${data.anchorId} tag=${data.tagId} dist=${data.distance}m`);
                processReading(data);
            } catch (e) {
                // Skip non-JSON messages silently
            }
        }
    });

    client.on('error', (err) => {
        console.error('MQTT error:', err);
    });

    return client;
}