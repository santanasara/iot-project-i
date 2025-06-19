# IoT Project - Irrigation System with Home Assistant

## Prerequisites

- [Visual Studio Code](https://code.visualstudio.com/)
- [Wokwi extension for VS Code](https://marketplace.visualstudio.com/items?itemName=wokwi.wokwi-vscode)
- [Docker](https://www.docker.com/get-started) and Docker Compose

## Setup Instructions

### 1. Running the Infrastructure

Start Home Assistant and Mosquitto MQTT broker:

```bash
docker compose up
```

### 2. Configuring Home Assistant

- Access the Home Assistant dashboard at `http://localhost:8123`
1. Complete the initial Home Assistant setup
2. Add the MQTT integration:
- Go to Settings -> Devices & Services
- Click + Add Integration and search for "MQTT"
- Enter the broker address: mosquitto (port: 1883)
- Leave username and password empty



