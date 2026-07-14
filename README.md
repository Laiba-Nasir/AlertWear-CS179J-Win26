# VERTEX

**Real-Time Indoor Worker Safety Tracking System**

![C++](https://img.shields.io/badge/C++-00599C?style=flat&logo=c%2B%2B&logoColor=white)
![ESP32-S3](https://img.shields.io/badge/ESP32--S3-E7352C?style=flat&logo=espressif&logoColor=white)
![SvelteKit](https://img.shields.io/badge/SvelteKit-FF3E00?style=flat&logo=svelte&logoColor=white)
![Node.js](https://img.shields.io/badge/Node.js-339933?style=flat&logo=nodedotjs&logoColor=white)
![MQTT](https://img.shields.io/badge/MQTT-660066?style=flat&logo=mqtt&logoColor=white)
![Docker](https://img.shields.io/badge/Docker-2496ED?style=flat&logo=docker&logoColor=white)

> Senior Capstone · UCR CS 179J · 2026  
> Sponsored by Cal Pak USA

---

## Overview

VERTEX is a real-time indoor positioning and environmental monitoring system designed to keep workers safe in industrial settings where GPS doesn't reach. It uses **Ultra-Wideband (UWB) trilateration** across a mesh of fixed anchor nodes to locate workers with sub-meter accuracy, while continuously monitoring air quality and environmental hazards on-device.

When something goes wrong — a worker collapses, an air quality threshold is crossed, or an emergency button is pressed — VERTEX pushes an immediate alert to on-site supervisors through a live web dashboard.

📺 **Demo:** [youtu.be/Vt9jytgt9IY](https://youtu.be/Vt9jytgt9IY)

---

## The Problem

Traditional worker safety systems rely on line-of-sight visibility or GPS, both of which fail in the environments they're most needed — warehouses, factory floors, refineries, construction interiors. When a worker is injured or unresponsive in a large indoor space, minutes matter, but there's often no reliable way to know *where* they are.

VERTEX solves this by combining:
- Precise indoor positioning without GPS
- Continuous environmental hazard monitoring
- Instant supervisor alerting
- Low-cost hardware that scales with the site

---

## System Architecture

```
┌─────────────────┐      UWB Ranging       ┌─────────────────┐
│   Tag Node      │◄──────(DW3000)────────►│  Anchor Nodes   │
│  (Worker-worn)  │                        │  (fixed refs)   │
│                 │                        │                 │
│  ESP32-S3       │                        │  ESP32-S3       │
│  + DW3000       │                        │  + DW3000       │
│  + BME680       │                        │                 │
│  + Buzzer/LED   │                        │                 │
│  + E-Button     │                        │                 │
└────────┬────────┘                        └────────┬────────┘
         │                                          │
         │             MQTT over Wi-Fi              │
         └──────────────────┬───────────────────────┘
                            ▼
                  ┌───────────────────┐
                  │  Mosquitto Broker │
                  │     (Docker)      │
                  └─────────┬─────────┘
                            ▼
                  ┌───────────────────┐
                  │   Trilateration   │
                  │  Server (Node.js) │
                  └─────────┬─────────┘
                            ▼
                  ┌───────────────────┐
                  │     Dashboard     │
                  │    (SvelteKit)    │
                  └───────────────────┘
```

---

## Features

- **Sub-meter indoor positioning** via UWB two-way ranging across multiple anchors
- **Environmental sensing** (gas, temperature, humidity, pressure) via BME680
- **Custom piggyback protocol** transmits sensor payloads alongside position data without disrupting UWB timing
- **On-device emergency alerting** — physical button, buzzer, RGB LED
- **Live supervisor dashboard** with real-time position, environmental readouts, and alerts
- **Dockerized MQTT broker** for portable, reproducible deployment

---

## Tech Stack

| Layer | Technology |
|---|---|
| Microcontroller | ESP32-S3 |
| UWB Radio | DW3000 |
| Environmental Sensor | BME680 |
| Firmware | C++ |
| Message Bus | MQTT (Mosquitto, Docker) |
| Backend | Node.js |
| Frontend | SvelteKit |
| On-Device Alerts | Piezo buzzer, RGB LED, momentary switch |

---

## Technical Highlights

**Custom sensor piggyback protocol.** Environmental readings are transmitted alongside UWB position payloads without interfering with ranging timing — a real constraint given the sub-microsecond scheduling requirements of UWB two-way ranging exchanges.

**Multi-node firmware coordination.** All anchors and tags run coordinated firmware. Debugging concurrent behavior across multiple physical devices required custom test tooling, logic analyzer captures, and serial output validation.

**End-to-end real-time pipeline.** From ESP32-S3 → MQTT → Node.js trilateration → SvelteKit UI, the full data flow was designed for real-time responsiveness. Position updates and alerts propagate to the dashboard within milliseconds of a physical event.

---

## Team

- **Laiba Nasir** — firmware (ESP32-S3 UWB ranging, sensor integration, MQTT publishing, alert system), Node.js trilateration server, SvelteKit dashboard
- Erick Acuna
- Abby Allers
- Mari Ozuna Garcia

---

## Acknowledgments

Sponsored and guided by **Cal Pak USA**, with hardware mentorship from Moiz Qureshi.

Presented at the **5th Annual UCR Bourns College of Engineering Senior Design Showcase**, where the project earned recognition from professors in the Mechanical and Bioengineering departments — validation that VERTEX solved a problem across engineering disciplines, not just within CS.

---

## Demo

📺 [youtu.be/Vt9jytgt9IY](https://youtu.be/Vt9jytgt9IY)
