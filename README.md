## Component still under testing - No guarantee on full functionality

# ESPHome Daly BMS Multi-Device Component

[![ESPHome](https://img.shields.io/badge/ESPHome-2024.11+-blue.svg)](https://esphome.io/)
[![License: CC BY-NC-SA 4.0](https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc-sa/4.0/)
[![GitHub stars](https://img.shields.io/github/stars/Kicosta/esphome-multi-daly-bms.svg)](https://github.com/Kicosta/esphome-daly-multi-bms/stargazers)

**Advanced ESPHome component supporting multiple Daly BMS units on a shared RS485 bus with centralized UART dispatch.**

**⚠️ Personal Use Only** - Not for commercial use without permission. See [License](#license) section.

## Features

- ✅ **Multi-BMS Support** - Connect 2+ Daly BMS units on the same RS485 bus
- ✅ **Centralized Dispatcher** - No packet collisions or data loss
- ✅ **Individual Addressing** - Each BMS has unique command and response addresses
- ✅ **Full Sensor Support** - Voltage, current, SOC, temperatures, cell voltages, and more
- ✅ **Configurable Update Intervals** - Optimize polling frequency per BMS
- ✅ **Detailed Logging** - Debug support for troubleshooting
- ✅ **Native ESPHome Integration** - Works seamlessly with Home Assistant

---

## Table of Contents

- [Why This Component?](#why-this-component)
- [Architecture](#architecture)
- [Hardware Requirements](#hardware-requirements)
- [Installation](#installation)
- [Configuration](#configuration)
- [Supported Sensors](#supported-sensors)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

---

## Why This Component?

The standard Daly BMS component in ESPHome **cannot handle multiple BMS units** on the same RS485 bus because:

1. **Shared UART Buffer** - Both BMS instances read from the same buffer, causing packet collisions
2. **Hardcoded Response Address** - Original component expects responses only from address `0x01`
3. **No Packet Routing** - No mechanism to route responses to the correct BMS instance

### This Component Solves These Issues:

- **Centralized Dispatcher** reads all UART data and routes packets to the correct BMS instance
- **Configurable Response Addresses** allow each BMS to filter its own responses
- **Zero Packet Loss** - Every response reaches its intended BMS instance

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────┐
│           ESP32 / ESPHome                   │
│                                             │
│  ┌──────────────────────────────────────┐  │
│  │   DalyBmsDispatcher (Singleton)      │  │
│  │   - Reads UART buffer                │  │
│  │   - Identifies response address      │  │
│  │   - Routes to correct BMS instance   │  │
│  └──────────────┬───────────────────────┘  │
│                 │                           │
│       ┌─────────┴─────────┐                │
│       │                   │                │
│  ┌────▼─────┐      ┌─────▼────┐           │
│  │  BMS1    │      │  BMS2    │           │
│  │ (0x40→   │      │ (0x41→   │           │
│  │  0x01)   │      │  0x02)   │           │
│  └────┬─────┘      └─────┬────┘           │
│       │                  │                 │
└───────┼──────────────────┼─────────────────┘
        │                  │
        │     RS485 Bus    │
        │                  │
    ┌───▼──┐           ┌───▼──┐
    │ BMS1 │           │ BMS2 │
    │ Daly │           │ Daly │
    └──────┘           └──────┘
```

### Communication Flow:

1. **BMS Instance** requests data (e.g., `A5 40 90 ...` for BMS1)
2. **Dispatcher** sends request via UART
3. **Physical BMS** responds (e.g., `A5 01 90 ...`)
4. **Dispatcher** reads response, identifies address (`0x01`)
5. **Dispatcher** routes to correct BMS instance (BMS1)
6. **BMS Instance** processes data and publishes sensors

---

## Hardware Requirements

### Required Hardware:

- **ESP32** or **ESP8266** board
- **RS485 to TTL converter** (e.g., MAX485, MAX3485)
- **2 or more Daly BMS units** with RS485 interface
- **Shared RS485 bus** connecting all BMS units

### RS485 Wiring:

```
ESP32          RS485 Module      BMS1 & BMS2
──────         ────────────      ───────────
GPIO17  ──────> TX (DI)          
GPIO16  <────── RX (RO)          
                   A    ─────────> A (all BMS)
                   B    ─────────> B (all BMS)
GND     ───────> GND   ─────────> GND
```

### BMS Configuration:

Each BMS must be configured with **unique addresses**:

- **BMS1**: Command Address = `0x40`, Response Address = `0x01`
- **BMS2**: Command Address = `0x41`, Response Address = `0x02`
- **BMS3**: Command Address = `0x42`, Response Address = `0x03` (if needed)

> **Note:** Use the Daly BMS PC software or Bluetooth app to set these addresses.

---

## Installation

### 1. Download Component

Clone this repository or download as ZIP:

```bash
git clone https://github.com/Kicosta/esphome-daly-bms-multi.git
```

### 2. Extract to ESPHome Directory

Extract the component to your ESPHome configuration directory:

```bash
# If using Home Assistant Add-on
unzip esphome-daly-bms-multi.zip -d /config/esphome/my_components/

# If using standalone ESPHome
unzip esphome-daly-bms-multi.zip -d ~/esphome/my_components/
```

**Verify directory structure:**

```
my_components/
├── daly_bms/
│   ├── __init__.py
│   ├── daly_bms.h
│   ├── daly_bms.cpp
│   ├── sensor.py
│   ├── binary_sensor.py
│   └── text_sensor.py
└── daly_bms_dispatcher/
    ├── __init__.py
    ├── daly_bms_dispatcher.h
    └── daly_bms_dispatcher.cpp
```

### 3. Reference in ESPHome Config

Add to your `.yaml` file:

```yaml
external_components:
  - source:
      type: local
      path: my_components
    components: [daly_bms, daly_bms_dispatcher]
```

---

##  Configuration

### Minimal Configuration

```yaml
esphome:
  name: battery-monitor

esp32:
  board: esp32dev

# External components
external_components:
  - source:
      type: local
      path: my_components
    components: [daly_bms, daly_bms_dispatcher]

# UART for RS485
uart:
  id: uart_bus
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600

# Dispatcher (singleton - configure once)
daly_bms_dispatcher:
  id: bms_dispatcher
  uart_id: uart_bus

# BMS instances
daly_bms:
  - id: bms1
    dispatcher_id: bms_dispatcher
    address: 0x40              # Command address
    response_address: 0x01     # Response address
    update_interval: 8s
    
  - id: bms2
    dispatcher_id: bms_dispatcher
    address: 0x41
    response_address: 0x02
    update_interval: 9s

# Sensors
sensor:
  - platform: daly_bms
    bms_daly_id: bms1
    voltage:
      name: "BMS1 Voltage"
    current:
      name: "BMS1 Current"
    battery_level:
      name: "BMS1 SOC"
    
  - platform: daly_bms
    bms_daly_id: bms2
    voltage:
      name: "BMS2 Voltage"
    current:
      name: "BMS2 Current"
    battery_level:
      name: "BMS2 SOC"
```

### Full Configuration Example

See [`example.yaml`](example.yaml) for a complete configuration with all sensors.

---

##  Supported Sensors

### Sensor Platform

| Sensor | Description | Unit |
|--------|-------------|------|
| `voltage` | Total pack voltage | V |
| `current` | Pack current (+ charge, - discharge) | A |
| `battery_level` | State of Charge (SOC) | % |
| `max_cell_voltage` | Highest cell voltage | V |
| `max_cell_voltage_number` | Cell number with highest voltage | - |
| `min_cell_voltage` | Lowest cell voltage | V |
| `min_cell_voltage_number` | Cell number with lowest voltage | - |
| `max_temperature` | Highest temperature | °C |
| `max_temperature_probe_number` | Probe number with highest temp | - |
| `min_temperature` | Lowest temperature | °C |
| `min_temperature_probe_number` | Probe number with lowest temp | - |
| `remaining_capacity` | Remaining capacity | Ah |
| `cells_number` | Number of cells in series | - |
| `temperature_1` | Temperature sensor 1 | °C |
| `temperature_2` | Temperature sensor 2 | °C |
| `cell_1_voltage` to `cell_16_voltage` | Individual cell voltages | V |

### Binary Sensor Platform

| Sensor | Description |
|--------|-------------|
| `charging_mos_enabled` | Charge MOS status |
| `discharging_mos_enabled` | Discharge MOS status |

### Text Sensor Platform

| Sensor | Description |
|--------|-------------|
| `status` | BMS status (Stationary/Charging/Discharging) |

---

##  Troubleshooting

### Problem: "Could not find __init__.py for daly_bms_dispatcher"

**Cause:** Incorrect directory structure.

**Solution:** Ensure two separate directories exist:
```bash
ls my_components/daly_bms/__init__.py          # Must exist
ls my_components/daly_bms_dispatcher/__init__.py  # Must exist
```

---

### Problem: "BMS2 sensors always show 'unavailable'"

**Cause:** Response address mismatch or dispatcher not configured.

**Solution:**

1. Check YAML configuration:
   ```yaml
   daly_bms:
     - id: bms2
       response_address: 0x02  # Must match BMS physical setting!
   ```

2. Verify BMS hardware address using Daly PC software

3. Check logs for:
   ```
   [V][daly_bms_dispatcher]: Dispatching frame to BMS instance (response_addr=0x02)
   ```

---

### Problem: Data updates very slowly

**Cause:** Update intervals too long.

**Solution:** Reduce `update_interval`:

```yaml
daly_bms:
  - id: bms1
    update_interval: 8s  # Faster updates
```

**Recommended intervals:**
- **5-10s**: Real-time monitoring, active charging/discharging
- **20-30s**: Balanced (recommended)
- **60s**: Low-frequency monitoring, energy saving

---

### Problem: Compilation errors about missing methods

**Cause:** Old ESPHome version or cache issues.

**Solution:**

```bash
# Clean build cache
esphome clean your-config.yaml

# Update ESPHome
pip3 install -U esphome

# Recompile
esphome compile your-config.yaml
```

---

## Debug logging
```yaml
logger:
  level: VERBOSE
```

**Useful log patterns:**

```
# Dispatcher receiving and routing
[V][daly_bms_dispatcher]: Complete frame received: addr=0x01, len=13
[V][daly_bms_dispatcher]: Dispatching frame to BMS instance (response_addr=0x01)

# BMS processing frame
[V][daly_bms]: Processing frame for BMS addr=0x40, response_addr=0x01

# Sensor updates
[D][sensor]: 'BMS1 Voltage': Sending state 54.20V
```

---

##  Performance Tuning

### Update Interval Recommendations

| Scenario | BMS1 | BMS2 | Bus Load | Notes |
|----------|------|------|----------|-------|
| **Real-time** | 5s | 6s | ~52% | For active monitoring |
| **Balanced** ⭐ | 8s | 9s | ~33% | Recommended |
| **Efficient** | 20s | 20s | ~14% | Low power |
| **Power Saving** | 60s | 60s | ~5% | Minimal updates |

### Best Practices:

1. **Stagger intervals** (e.g., 8s and 9s) to avoid simultaneous polling
2. **Start with 20s**, reduce if you need faster updates
3. **Monitor logs** for timeout warnings
4. **Avoid intervals < 5s** unless necessary

---

##  Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Setup

```bash
# Clone repository
git clone https://github.com/Kicosta/esphome-daly-bms-multi.git
cd esphome-daly-bms-multi

# Test with ESPHome
esphome compile example.yaml
```

---

## ⚖️ License Notice

This component is provided for **personal, non-commercial use only** under CC BY-NC-SA 4.0.

**Permitted:**
- Home automation projects
- Personal learning and experimentation
- Sharing modifications with the community

**Not Permitted:**
- Commercial products or services
- Paid installations or consulting
- Resale or redistribution in commercial packages

For commercial licensing, contact: [kicosta@hotmail.com]

See [LICENSE](LICENSE) for full terms.

---

##  Acknowledgments

- Original Daly BMS component by [@s1lvi0](https://github.com/s1lvi0)
- Multi-BMS dispatcher architecture by [Kicosta](https://github.com/Kicosta)
- ESPHome community for the excellent platform

---

##  Related Projects

- [ESPHome](https://esphome.io/) - The platform this component is built for
- [Home Assistant](https://www.home-assistant.io/) - Home automation platform
- [Original Daly BMS Component](https://esphome.io/components/sensor/daly_bms/)

---

** If this component helped you, please star the repository!**

- `dispatcher.py` - Configurazione dispatcher
- `sensor.py`, `binary_sensor.py`, `text_sensor.py` - Sensori
