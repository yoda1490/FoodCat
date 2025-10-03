# Automatic IoT Cat Feeder (ESP8266)

A WiFi-enabled automatic cat feeder based on the ESP8266 (esp12e), featuring a web interface for scheduling, manual feeding, settings management, OTA updates, and persistent logs. Secure authentication is included, and the system is designed for easy integration with home automation setups.

## Features

- **Web Interface:** Control and configure the feeder from any browser on your local network.
- **WiFi Management:** Easy setup and configuration using WiFiManager.
- **NTP Synchronization:** Accurate feeding schedules using network time.
- **Manual & Scheduled Feeding:** Feed your cat instantly or on a schedule.
- **Persistent Settings & Logs:** Uses LittleFS for storing settings and logs.
- **OTA Updates:** Update firmware wirelessly.
- **Authentication:** Basic authentication with hashed passwords.
- **Physical Buttons:** Manual feed and WiFi reset buttons.
- **Reverse Proxy Friendly:** Designed to run behind an HTTPS reverse proxy.

## Hardware Requirements

- ESP8266 (esp12e or compatible)
- Relay or motor driver for food dispenser
- Physical buttons (feed, WiFi reset)
- Status LED
- Power supply

## Getting Started

### 1. Clone the Repository

```sh
git clone [https://github.com/yourusername/esp8266-catfeeder.git](https://github.com/yoda1490/FoodCat)
cd FoodCat
```

### 2. PlatformIO Setup

- Install [PlatformIO](https://platformio.org/) in VS Code.
- Open the project folder in VS Code.

### 3. Configure WiFi and Settings

- On first boot, the device starts in AP mode for WiFi setup.
- Access the configuration portal via the device's WiFi hotspot.

### 4. Upload Filesystem Data

```sh
pio run --target uploadfs
```

### 5. Upload Firmware

```sh
pio run --target upload
```

### 6. Access the Web Interface

- Find the device's IP address from your router or serial monitor.
- Open `http://<device-ip>/` in your browser.

## File Structure

```
src/        # Main firmware source code
data/       # Web interface files (HTML, CSS, JS)
platformio.ini
```

## Security Notes

- Passwords are stored as hashes.
- For best security, run the device behind an HTTPS reverse proxy.

## License

This project is licensed under the [MIT License](LICENSE).

---


**Contributions welcome!**
