# ESP32 WiFi + OTA Template

Minimales ESP32-Projekt mit WiFi-Konnektivitaet und Over-The-Air (OTA) Updates.

## Inhaltsverzeichnis
- [Features](#features)
- [Projektstruktur](#projektstruktur)
- [Konfiguration](#konfiguration)
- [Build und Upload](#build-und-upload)
- [OTA ohne feste IP](#ota-ohne-feste-ip)
- [Logging](#logging)
- [Security und Betrieb](#security-und-betrieb)
- [Tests und CI](#tests-und-ci)
- [Entwicklung](#entwicklung)
- [Referenzen](#referenzen)

## Features
- WiFi Connection mit Reconnect-Strategie (Backoff + Jitter)
- OTA Updates (fail-closed ohne Passwort)
- Logging via Serial
- Secrets ausserhalb des Repos (`../_secrets`)
- USB- oder OTA-Upload

## Projektstruktur
```text
esp32_template/
├── src/
├── scripts/
├── test/
├── platformio.ini
└── README.md
```

Externe Dateien (nicht im Repo):
```text
../_secrets/
├── WifiSecret.h
├── OtaSecret.h
└── platformio_override.ini
```

## Konfiguration

### WiFi Credentials
Datei: `../_secrets/WifiSecret.h`
```cpp
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PWD  "YOUR_PASSWORD"
```

### OTA Passwort
Datei: `../_secrets/OtaSecret.h`
```cpp
#define OTA_PASSWORD "your_ota_password"
```

Wichtig: Standard ist fail-closed. Wenn `OTA_PASSWORD` leer ist und `OTA_ALLOW_INSECURE_NO_PASSWORD false` bleibt, wird OTA deaktiviert.

### OTA Upload-Auth fuer PlatformIO (lokal)
Datei: `../_secrets/platformio_override.ini`
```ini
[env:az-delivery-devkit-v4-ota]
upload_flags =
  --auth=YOUR_OTA_PASSWORD
```

Diese Datei wird ueber `extra_configs = ../_secrets/platformio_override.ini` geladen.

## Build und Upload

### USB
```bash
pio run -e az-delivery-devkit-v4-usb
pio run -e az-delivery-devkit-v4-usb -t upload -t monitor --upload-port COM3
```

### OTA
Es gibt keine feste IP mehr in `platformio.ini`.

## OTA ohne feste IP

### Option A: CLI mit IP
```bash
pio run -e az-delivery-devkit-v4-ota --upload-port 192.168.X.X -t upload
```

### Option B: Interaktives Skript (Prompt)
```powershell
.\scripts\upload_ota.ps1
```
Das Skript fragt die IP ab und startet dann den OTA-Upload.
Unter Windows erscheint dafuer eine InputBox (mit letztem Wert als Vorschlag).
Die zuletzt verwendete IP wird in `../_secrets/last_ota_ip.txt` gespeichert und beim naechsten Start als Default vorgeschlagen.
Das Skript wechselt automatisch ins Projektverzeichnis, daher funktioniert es auch bei Aufruf aus `scripts/`.

Alternative unter Windows:
```bat
scripts\upload_ota.bat
```

## Logging
Beispiele:
```cpp
Trace::log(TraceLevel::INFO, "WiFi connected");
Trace::logf(TraceLevel::INFO, "RSSI: %d dBm", WiFi.RSSI());
```

Statusmeldungen unterscheiden jetzt auch OTA-Status (`enabled`/`disabled`).

## Security und Betrieb
- OTA nur in vertrauenswuerdigen Netzen aktivieren.
- Keine Secrets ins Repo committen.
- Fuer produktive Geraete `OTA_PASSWORD` setzen.
- Bei WLAN-Ausfall: Reconnect mit Backoff + Jitter.

## Tests und CI
- Firmware-Build: `pio run -e az-delivery-devkit-v4-usb`
- Native Tests: `pio test -e native` (benoetigt `gcc/g++` lokal)
- CI Workflow: `.github/workflows/ci.yml`

## Entwicklung

### Secrets initial anlegen (ohne Ueberschreiben bestehender Dateien)
Windows (PowerShell):
```powershell
.\scripts\setup_secrets.ps1
```

Linux / macOS / CI:
```bash
bash scripts/setup_secrets.sh
```

### Build-Cache leeren
```bash
rm -rf .pio/build
```
PowerShell:
```powershell
Remove-Item -Recurse -Force .pio\build
```

## Referenzen
- Arduino Core fuer ESP32: https://github.com/espressif/arduino-esp32
- ArduinoOTA: https://github.com/espressif/arduino-esp32/tree/master/libraries/ArduinoOTA
- PlatformIO: https://platformio.org
