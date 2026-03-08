#!/usr/bin/env bash
# Flash anchor_mqtt firmware. Optionally update the MQTT broker IP first.
# Usage:
#   ./scripts/flash-anchor.sh              # prompts for IP
#   ./scripts/flash-anchor.sh 192.168.1.5  # sets IP and flashes
set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
FIRMWARE="$ROOT/firmware/anchor_mqtt"
MAIN="$FIRMWARE/src/main.cpp"

# Read current non-commented MQTT_SERVER line
CURRENT_IP=$(grep '^const char\* MQTT_SERVER' "$MAIN" | sed 's/.*"\(.*\)".*/\1/')
echo "Current MQTT_SERVER: $CURRENT_IP"

if [ -n "$1" ]; then
    NEW_IP="$1"
else
    read -rp "Enter MQTT broker IP (press Enter to keep '$CURRENT_IP'): " NEW_IP
fi

if [ -n "$NEW_IP" ] && [ "$NEW_IP" != "$CURRENT_IP" ]; then
    sed -i '' "s/^const char\* MQTT_SERVER = \".*\";/const char* MQTT_SERVER = \"$NEW_IP\";/" "$MAIN"
    echo "Updated MQTT_SERVER -> $NEW_IP"
fi

echo "Flashing anchor firmware..."
cd "$FIRMWARE"
~/.platformio/penv/bin/pio run --target upload

echo "Done. Monitor with: pio device monitor -d $FIRMWARE"
