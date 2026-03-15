#!/usr/bin/env bash
# Start the full mock stack: MQTT broker (Docker) + backend server + mock anchor
set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

cleanup() {
    echo ""
    echo "Stopping mock stack..."
    kill "$SERVER_PID" "$MOCK_PID" 2>/dev/null || true
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

# 3. Start mock anchor
echo "Waiting for server to start..."
sleep 3
echo "Starting mock anchor..."
cd "$ROOT"
node mock/mock-anchor.js &
MOCK_PID=$!

echo ""
echo "Mock stack running. Press Ctrl+C to stop all."
wait
