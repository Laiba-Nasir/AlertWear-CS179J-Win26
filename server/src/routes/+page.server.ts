import { getWorkerPositions, getAnchors, getAlertHistory } from '$lib/server/store';

export function load() {
    return {
        workers: getWorkerPositions(),
        anchors: getAnchors(),
        alerts: getAlertHistory()
    };
}