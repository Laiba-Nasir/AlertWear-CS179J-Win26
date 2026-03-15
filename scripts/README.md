# Scripts

## Usage

```bash
./scripts/mock.sh                       # full mock run (Docker + server + simulated anchors)
./scripts/start.sh                      # real hardware run (Docker + server)
./scripts/flash-anchor.sh               # flash anchor — prompts for MQTT broker IP
./scripts/flash-anchor.sh 172.20.10.7   # flash anchor — set IP inline
./scripts/flash-tag.sh                  # flash tag firmware
```

---

## mock.sh

Starts the full simulated stack for development and testing without any physical hardware.

**What it runs:**
1. Starts the Mosquitto MQTT broker in Docker (`mqtt/docker-compose.yml`)
2. Starts the SvelteKit backend dev server (`server/`)
3. Waits 3 seconds for the server to initialize, then starts `mock/mock-anchor.js`

**The mock anchor** simulates 4 anchors (IDs `0–3`) and 4 workers (tag IDs `1–4`) publishing UWB distance + gas readings to the broker every second on topics `uwb/anchor/<id>/data`. The server receives these, runs trilateration, and updates the dashboard in real time.

**Cleanup:** Pressing Ctrl+C stops the dev server, the mock process, and the Docker container in one step.

**Prerequisites:** Docker running, `node_modules` installed in `server/` (`npm install`).

---

## start.sh

Starts the real hardware stack. Use this when physical anchors and tags are deployed and already flashed with the correct firmware.

**What it runs:**
1. Starts the Mosquitto MQTT broker in Docker (`mqtt/docker-compose.yml`)
2. Starts the SvelteKit backend dev server (`server/`)

The anchors connect over WiFi and publish to the broker automatically. The server listens on `uwb/#` for their messages.

**Prerequisites:**
- Anchors flashed with the correct MQTT broker IP (see `flash-anchor.sh`)
- Anchors and the machine running this script must be on the same network
- Docker running, `node_modules` installed in `server/`

**Cleanup:** Ctrl+C stops the dev server and Docker container.

---

## flash-anchor.sh

Builds and flashes the anchor firmware (`firmware/anchor_mqtt`) to a connected ESP32-S3. Optionally updates the MQTT broker IP in the firmware before flashing.

**Usage:**
```bash
./scripts/flash-anchor.sh              # prompts for IP interactively
./scripts/flash-anchor.sh 192.168.1.5  # sets IP and flashes immediately
```

**What it does:**
1. Reads the current `MQTT_SERVER` value from `firmware/anchor_mqtt/src/main.cpp`
2. Prompts for a new IP (or accepts one as an argument). Press Enter to keep the existing IP.
3. If a new IP is provided, updates the `MQTT_SERVER` line in `main.cpp` using `sed`
4. Runs `pio run --target upload` in `firmware/anchor_mqtt/`

**WiFi credentials** (SSID and password) are hardcoded in `firmware/anchor_mqtt/src/main.cpp` lines 20–21 and must be changed manually if switching networks.

**Prerequisites:** [PlatformIO CLI](https://docs.platformio.org/en/latest/core/installation/index.html) (`pio`) installed and on your PATH. Anchor connected via USB.

---

## flash-tag.sh

Builds and flashes the tag firmware (`firmware/tag`) to a connected ESP32-S3.

**What it does:**
Runs `pio run --target upload` in `firmware/tag/`. The tag firmware operates in UWB piggyback mode — it reads the BME680 gas sensor and transmits data over UWB directly to the anchors. No WiFi or MQTT configuration is needed.

**Prerequisites:** PlatformIO CLI installed and on your PATH. Tag connected via USB on the port configured in `firmware/tag/platformio.ini` (default: `/dev/cu.usbmodem101`).
