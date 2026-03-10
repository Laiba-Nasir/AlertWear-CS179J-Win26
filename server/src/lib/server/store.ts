import { readFileSync } from 'fs';
import { resolve } from 'path';

function loadWorkers(): Worker[] {
    try {
        const data = readFileSync(resolve('src/lib/config/workers.json'), 'utf-8');
        return JSON.parse(data);
    } catch {
        return [];
    }
}

// Types
export interface Worker {
    tagId: number;
    fullName: string;
    empId: string;
}

export interface Anchor {
    anchorId: string;
    name: string;
    x: number;
    y: number;
    online: boolean;
    lastSeen: string | null;
}

export interface WorkerPosition {
    tagId: number;
    fullName: string;
    empId: string;
    x: number;
    y: number;
    gasResistance: number;
    iaq: number;
    iaqLabel: string;
    temperature: number | null;
    humidity: number | null;
    pressure: number | null;
    panic: boolean;
    status: 'ok' | 'warning' | 'alert' | 'panic';
    lastSeen: string;
}

export interface RangeReading {
    anchorId: string;
    distance: number;
    timestamp: string;
}

// Store
export let workers: Worker[] = loadWorkers();

setInterval(() => {
    workers = loadWorkers();
    for (const tagId of workerPositions.keys()) {
        if (!workers.find(w => w.tagId === tagId)) {
            workerPositions.delete(tagId);
        }
    }
}, 5000);

import anchorsConfig from '$lib/config/anchors.json';

export const anchors: Anchor[] = anchorsConfig.map(a => ({
    ...a,
    online: false,
    lastSeen: null
}));

export interface AlertRecord {
    tagId: number;
    fullName: string;
    empId: string;
    type: 'gas' | 'panic';
    gasResistance: number;
    iaq: number;
    timestamp: string;
}

export const alertHistory: AlertRecord[] = [];

const tagReadings: Map<number, Map<string, RangeReading>> = new Map();
export const workerPositions: Map<number, WorkerPosition> = new Map();

// Staleness timeout: mark anchors offline and remove workers after 15 seconds of no data
const STALE_TIMEOUT_MS = 15_000;

setInterval(() => {
    const now = Date.now();

    // Mark anchors offline if no data received recently
    for (const anchor of anchors) {
        if (anchor.online && anchor.lastSeen) {
            const age = now - new Date(anchor.lastSeen).getTime();
            if (age > STALE_TIMEOUT_MS) {
                anchor.online = false;
            }
        }
    }

    // Remove stale worker positions
    for (const [tagId, pos] of workerPositions) {
        const age = now - new Date(pos.lastSeen).getTime();
        if (age > STALE_TIMEOUT_MS) {
            workerPositions.delete(tagId);
            tagReadings.delete(tagId);
        }
    }
}, 3000);

// IAQ Calculation
function gasResistanceToIAQ(gasResistance: number): { iaq: number; label: string } {
    let iaq: number;

    if (gasResistance >= 300) {
        iaq = Math.max(0, 25 - ((gasResistance - 300) / 200) * 25);
    } else if (gasResistance >= 150) {
        iaq = 25 + ((300 - gasResistance) / 150) * 50;
    } else if (gasResistance >= 75) {
        iaq = 75 + ((150 - gasResistance) / 75) * 75;
    } else if (gasResistance >= 30) {
        iaq = 150 + ((75 - gasResistance) / 45) * 100;
    } else if (gasResistance >= 10) {
        iaq = 250 + ((30 - gasResistance) / 20) * 100;
    } else {
        iaq = 350 + ((10 - gasResistance) / 10) * 150;
    }

    iaq = Math.round(Math.max(0, Math.min(500, iaq)));

    let label: string;
    if (iaq <= 50) label = 'Excellent';
    else if (iaq <= 100) label = 'Good';
    else if (iaq <= 150) label = 'Moderate';
    else if (iaq <= 200) label = 'Poor';
    else if (iaq <= 300) label = 'Unhealthy';
    else label = 'Hazardous';

    return { iaq, label };
}

// Panic overrides gas status
function getStatus(iaq: number, panic: boolean): 'ok' | 'warning' | 'alert' | 'panic' {
    if (panic) return 'panic';
    if (iaq >= 200) return 'alert';
    if (iaq >= 150) return 'warning';
    return 'ok';
}

// Trilateration
function trilaterate(readings: Map<string, RangeReading>): { x: number; y: number } | null {
    if (readings.size < 3) return null;

    const points: { x: number; y: number; d: number }[] = [];
    
    readings.forEach((reading, anchorId) => {
        const anchor = anchors.find(a => a.anchorId === String(anchorId));
        if (anchor) {
            points.push({ x: anchor.x, y: anchor.y, d: reading.distance });
        }
    });

    if (points.length < 3) return null;

    const [p1, p2, p3] = points;

    const A = 2 * (p2.x - p1.x);
    const B = 2 * (p2.y - p1.y);
    const C = p1.d * p1.d - p2.d * p2.d - p1.x * p1.x + p2.x * p2.x - p1.y * p1.y + p2.y * p2.y;
    const D = 2 * (p3.x - p2.x);
    const E = 2 * (p3.y - p2.y);
    const F = p2.d * p2.d - p3.d * p3.d - p2.x * p2.x + p3.x * p3.x - p2.y * p2.y + p3.y * p3.y;

    const denominator = A * E - B * D;
    if (Math.abs(denominator) < 0.0001) return null;

    const x = (C * E - F * B) / denominator;
    const y = (A * F - D * C) / denominator;

    return {
        x: Math.max(0, Math.min(10, x)),
        y: Math.max(0, Math.min(20, y))
    };
}

// Process incoming MQTT data from anchors
export function processReading(data: {
    tagId: number;
    distance: number;
    anchorId: string;
    timestamp: string;
    temperature?: number;
    humidity?: number;
    pressure?: number;
    gas?: number;
    panic?: boolean;
}) {
    const { tagId, distance, anchorId } = data;
    const now = new Date().toISOString();

    const registeredWorker = workers.find(w => w.tagId === tagId);
    if (!registeredWorker) return;

    const anchor = anchors.find(a => a.anchorId === String(anchorId));
    if (anchor) {
        anchor.online = true;
        anchor.lastSeen = now;
    }

    if (!tagReadings.has(tagId)) {
        tagReadings.set(tagId, new Map());
    }
    tagReadings.get(tagId)!.set(anchorId, { anchorId, distance, timestamp: now });

    const readings = tagReadings.get(tagId)!;
    const position = trilaterate(readings);

    const existing = workerPositions.get(tagId);

    let gasResistance = existing?.gasResistance ?? 0;
    let iaq = existing?.iaq ?? 0;
    let iaqLabel = existing?.iaqLabel ?? 'Unknown';
    let temperature = existing?.temperature ?? null;
    let humidity = existing?.humidity ?? null;
    let pressure = existing?.pressure ?? null;
    let panic = existing?.panic ?? false;

    if (data.gas !== undefined) {
        gasResistance = data.gas;
        const iaqResult = gasResistanceToIAQ(gasResistance);
        iaq = iaqResult.iaq;
        iaqLabel = iaqResult.label;
    }
    if (data.temperature !== undefined) temperature = data.temperature;
    if (data.humidity !== undefined) humidity = data.humidity;
    if (data.pressure !== undefined) pressure = data.pressure;
    if (data.panic !== undefined) panic = data.panic;

    const status = getStatus(iaq, panic);

    // Log alert for gas hazard
    if (status === 'alert' && data.gas !== undefined) {
        alertHistory.unshift({
            tagId,
            fullName: registeredWorker.fullName,
            empId: registeredWorker.empId,
            type: 'gas',
            gasResistance,
            iaq,
            timestamp: new Date().toISOString()
        });
        if (alertHistory.length > 200) alertHistory.pop();
    }

    // Log alert for panic activation
    if (data.panic === true && !existing?.panic) {
        alertHistory.unshift({
            tagId,
            fullName: registeredWorker.fullName,
            empId: registeredWorker.empId,
            type: 'panic',
            gasResistance,
            iaq,
            timestamp: new Date().toISOString()
        });
        if (alertHistory.length > 200) alertHistory.pop();
    }

    if (position) {
        workerPositions.set(tagId, {
            tagId,
            fullName: registeredWorker.fullName,
            empId: registeredWorker.empId,
            x: parseFloat(position.x.toFixed(2)),
            y: parseFloat(position.y.toFixed(2)),
            gasResistance,
            iaq,
            iaqLabel,
            temperature,
            humidity,
            pressure,
            panic,
            status,
            lastSeen: now
        });
    } else if (existing) {
        workerPositions.set(tagId, {
            ...existing,
            gasResistance,
            iaq,
            iaqLabel,
            temperature,
            humidity,
            pressure,
            panic,
            status,
            lastSeen: now
        });
    } else {
        // Show worker immediately even before trilateration succeeds
        // Use nearest anchor position as approximate location
        const nearestAnchor = anchors.find(a => a.anchorId === String(anchorId));
        workerPositions.set(tagId, {
            tagId,
            fullName: registeredWorker.fullName,
            empId: registeredWorker.empId,
            x: nearestAnchor ? nearestAnchor.x : 0,
            y: nearestAnchor ? nearestAnchor.y : 0,
            gasResistance,
            iaq,
            iaqLabel,
            temperature,
            humidity,
            pressure,
            panic,
            status,
            lastSeen: now
        });
    }
}

export function getWorkerPositions(): WorkerPosition[] {
    return Array.from(workerPositions.values());
}

export function markAnchorOnline(anchorId: string) {
    const anchor = anchors.find(a => a.anchorId === String(anchorId));
    if (anchor) {
        anchor.online = true;
        anchor.lastSeen = new Date().toISOString();
    }
}

export function getAnchors(): Anchor[] {
    return anchors;
}

export function getAlertHistory(): AlertRecord[] {
    return alertHistory;
}