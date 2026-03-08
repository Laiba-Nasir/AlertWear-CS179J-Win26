#!/usr/bin/env bash
# Flash tag firmware (no WiFi/MQTT — UWB piggyback mode only).
# Usage: ./scripts/flash-tag.sh
set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
FIRMWARE="$ROOT/firmware/tag"

echo "Flashing tag firmware..."
cd "$FIRMWARE"
~/.platformio/penv/bin/pio run --target upload

echo "Done. Monitor with: pio device monitor -d $FIRMWARE"
