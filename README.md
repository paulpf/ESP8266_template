# ESP8266 WiFi + OTA Template (D1 mini / ESP8266-12F)

Minimales ESP8266-Projekt mit WiFi-Konnektivitaet und Over-The-Air (OTA) Updates.

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
ESP8266_template/
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
└── OtaSecret.h
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

Hinweis zum Skript `scripts/upload_ota.ps1`:
- Das Skript liest `OTA_PASSWORD` direkt aus `../_secrets/OtaSecret.h`.
- Es setzt die Upload-Auth zur Laufzeit selbst.
- Keine zusaetzliche `platformio_override.ini` fuer OTA-Auth noetig.

## Build und Upload

### USB (D1 mini)
```bash
pio run -e d1-mini-usb
pio run -e d1-mini-usb -t upload -t monitor --upload-port COM3
```

### OTA
Es gibt keine feste IP mehr in `platformio.ini`.

## OTA ohne feste IP

### Interaktives Skript (Prompt)
```powershell
.\scripts\upload_ota.ps1
```
Das Skript fragt die IP ab und startet dann den OTA-Upload.
Unter Windows erscheint dafuer eine InputBox (mit letztem Wert als Vorschlag).
Die zuletzt verwendete IP wird in `../_secrets/last_ota_ip.txt` gespeichert und beim naechsten Start als Default vorgeschlagen.
Das Skript wechselt automatisch ins Projektverzeichnis, daher funktioniert es auch bei Aufruf aus `scripts/`.
Die OTA-Authentifizierung wird automatisch aus `../_secrets/OtaSecret.h` uebernommen.

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
- Firmware-Build: `pio run -e d1-mini-usb`
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
- Arduino Core fuer ESP8266: https://github.com/esp8266/Arduino
- ArduinoOTA (ESP8266): https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html
- PlatformIO: https://platformio.org
