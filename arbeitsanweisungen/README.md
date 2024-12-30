# Lötregler Arbeitsanweisungen

## Vorbereitung

1. Überblick
   1. Hardware
      1. Elektronik
      2. Mechanik
   2. Software
      1. Arduino IDE
      2. Libraries
      3. Das Programm Selbst
   3. Assembling
      1. Zusammenbau
      2. Funktionstest

## Hardware

### Elektronik

1. Stücklisten öffnen
2. Bauteile komissionieren / prüfen
   1. Mengen
   2. Art / Type
   3. Fehlteile
3. Siebdruck
   1. Drucken
   2. Sichtkontrolle / ggf reparatur
4. Bestückung
   1. Bestücken aller Bauteile laut Stückliste
      1. Polaritäten richtig
      2. Werte richtig
      3. Pin 1 bei IC's
   2. Sichtkontrolle / ggf reparatur
5. Reflow
   1. Richtige Temperatur eingestellt
   2. Richtige Temperatur erreicht
   3. Sichtkontrolle / ggf reparatur
6. Handlöten der THT Bauteile
   1. Bauteil bestücken
   2. Bauteil fixieren
   3. Bauteil mit Handlötkolben löten
   4. Sichtkontrolle / ggf reparatur
7. Stecken des Displays
8. Elektrische prüfung der Platine durchführen

### Mechanik

1. Entscheidung welche Variante des Gehäuses gefertigt wird
2. Öffnen der .STL oder .3MF mit einem Slicer
3. Slicen der Dateien als GCODE
   1. Richtiger Kunststoff
   2. Richtiger Drucker
   3. Stützen an den richtigen Stellen?
   4. Lagenhöhe
   5. Druck Qualität / Geschwindigkeit
4. Transfer der .GCODE auf den Drucker
5. Drucker vorbereiten
   1. Druckbett mit Wasser reinigen
   2. Klebestift auftragen, nur dort wo gedruckt wird
   3. Kunststoff richtig eingespannt
      1. keine Knoten in Fillament
      2. richtiger Kunstoff
      3. Rolle läuft frei / leichtgängig
6. Druck starten
7. Teile durch biegen des Druckbettes entfernen (Vorsicht heiß)
8. Druckbett reinigen
9. Stützen entfernen

## Software

1. Arduino IDE vorbereiten
   1. Libraries installieren
      1. Sketch ==> Bibliotek einbinden ==> Bibliothek verwalten
      2. Alle Libs aus der #Include einbinden
   2. MCU installieren
      1. Werkzeuge ==>  Board ==> Boardverwalter ==> Raspberry Pico RP2040
2. Code anpassen
   1. Namen Eintragen
   2. Varianten im Code auswählen (AVR oder Pico)
3. Überprüfen drücken
   1. Fehler beheben
4. Leiterplatte mit PC verbinden
5. Upload drücken

## Assembling

1. Elektronik auf Grundplatte
2. Rahmen raufstecken
3. Akkuschuh anschweißen
4. Handstück vorbereiten
5. Hansstück anschließen
6. Zugentlastung
7. Betätige für Taster einsetzen
8. Deckel montieren
9. Funktionstest mit Testlötspitzen
10. Leistungskalibrierung
11. Have fun...
