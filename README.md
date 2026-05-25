HTL Lötregler Mini
==================

Einfacher Regler für JBC Lötspitzen der Serie T210, T245 und T470, Weller RT-Spitzen und TS100-Spitzen sollten auch funktionieren.

Vorrangig SMD-Bauteile, Größe 3216. 
Bauteilauswahl für Handbestückung ohne SMD-Schablone optimiert

Design ist möglichst kostengünstig, speziell für Schüler-Budget. 


![image](render-v21.0.0.png)

# Firmware-Status

Die neue Firmware-Entwicklung in diesem Branch erfolgt fuer den **Raspberry Pi Pico** mit **PlatformIO** und dem **Pico SDK in C**.

- Das aktive Projekt liegt jetzt im Repository-Root:
  - `platformio.ini`
  - `src/`
  - `include/`
  - `lib/LoetreglerLogic/`
  - `test/`
- Die alten Sketche unter `arduino/` bleiben als Referenz erhalten, sind aber nicht mehr der bevorzugte Entwicklungsweg fuer die Pico-Firmware.
- Die neue Firmware ist bewusst in **C statt C++** geschrieben, damit sie im Unterricht moeglichst leicht nachvollziehbar bleibt.

# PlatformIO-Befehle

## Build

```bash
pio run -e pico
```

## Upload

```bash
pio run -e pico -t upload
```

## Tests

Alle Logik-Tests:

```bash
pio test -e native
```

Einen einzelnen Testlauf starten:

```bash
pio test -e native -f test_logic
```

## Lint / statische Analyse

```bash
pio check -e pico
```

# Neue Firmware-Struktur

- `src/main.cpp`
- `src/main.c`
  - Hauptprogramm, Anzeige, Menue, Regelung und Sicherheitslogik
- `src/button_input.c` / `include/button_input.h`
  - entprellte Taster mit einfachen Press-/Release-Ereignissen
- `src/settings_store.c` / `include/settings_store.h`
  - Laden und Speichern der Einstellungen im Flash
- `src/ssd1306_simple.c` / `include/ssd1306_simple.h`
  - sehr einfacher OLED-Treiber in C fuer das 128x32-I2C-Display
- `lib/LoetreglerLogic/`
  - rein logische, testbare Funktionen fuer Menue, Grenzwerte und Einstellungen
- `test/test_logic/`
  - Unit-Tests fuer die Menue- und Einstellungslogik

# Menue-Funktionen der neuen Firmware

Das neue, bewusst einfach gehaltene Menue bietet aktuell diese Punkte:

- Starttemperatur
- Gruss auf dem Startbildschirm
- Maximale Temperatur
- Zeit bis zur Abschaltung
- Standby-Temperatur
- Schrittweite fuer Solltemperatur
- Ruecksetzen auf Werkseinstellungen

# Merkmale:
## Controller
* ATmega328, Kompatibel zu Arduino Nano
* 5V, 16MHz
* externe 2.5V Spannungsreferenz für ADC

## Stromversorgung
* Spannungsversorgung: 15 bis 48V, Akku oder Netzteil
* Tiefentladeschutz
* 10A Schmelzsicherung
* 48V TVS-Schutzdiode
* Ein-/Aus-Taster
* Schaltregler von Vin auf 12V und 5V
* 6.3 mm Flachsteckverbinder für die direkte Kontaktierung von 18V Makita Akkus, oder Hofer ActivEnergy Akkus mit 20 oder 40V. 

## Leistungsteil
* Highside N-Kanal MOS-FET: 80V, 10mR (geringe Verlustleistung, hohe Spannungsfestigkeit)
* zugehöriger Gatetreiber: notwendig für N-Kanal MOS-FET


## Sensorik
* Messverstärker zur Temperaturmessung der Lötspitze (24 µV/K)
* Spannungsmessung der Versorgung
* Messung der Leiterplattentemperatur
* Strommessung des Heizelements über 1mR-Shunt

## Benutzerschnittstelle
* OLED Display mit 128x32 Pixel, 23mm Diagonale
* 4 Bedientaster (Rauf, Runter, Enter, Zurück)

## Kommunikation
* Arduino USB
* AVR ISP
* Wahlweise
  * RS232
  * oder RS485 zur Kommunikation mit einem Steuerbaren Netzgerät (12, 24, 48V). 
