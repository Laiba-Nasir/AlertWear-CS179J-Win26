#!/usr/bin/env bash
# Start the real hardware stack: MQTT broker (Docker) + backend server
# Firmware on anchors must already be flashed with the correct MQTT broker IP.
set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

cleanup() {
    echo ""
    echo "Stopping server..."
    kill "$SERVER_PID" 2>/dev/null || true
    docker compose -f "$ROOT/mqtt/docker-compose.yml" stop
    exit 0
}
trap cleanup INT TERM

# 1. Start MQTT broker
echo "Starting MQTT broker..."
docker compose -f "$ROOT/mqtt/docker-compose.yml" up -d

# 2. Start backend server
echo "Starting backend server..."
cd "$ROOT/server"
npm run dev &
SERVER_PID=$!

echo ""
echo "Server running. Waiting for real hardware to connect..."
echo "Press Ctrl+C to stop."
wait
