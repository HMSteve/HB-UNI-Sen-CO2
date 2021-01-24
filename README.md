# HB-UNI-Sen-CO2

![HB-UNI-Sen-CO2](https://github.com/HMSteve/HB-UNI-Sen-CO2/blob/main/Images/dev_front.jpg)

Eine AskSinPP-Implementierung eines Innenraum-Sensors mit dem NDIR-CO2-Sensor SCD30. Das Geraet kann regulaer in eine Homematic-Umgebung eingebunden werden und liefert Messwerte fuer die CO2-Konzentration, Temperatur, Lueftfeuchte und bei bestuecktem BME280 auch Luftdruck. Ebenso kann es abseits einer Homematic-Installation stand-alone als "CO2-Ampel" verwendet werden: Die Messwerte werden auf einem E-Paper-Display angezeigt und es kann eine Indikator-LED fuer die CO2-Konzentration zugeschaltet werden. Der Betrieb erfolgt mittels zweier AA-NiMH-Akkus, auf Li-Po/Li-Ionen-Akkus wurde unter Sicherheitsaspekten bewusst verzichtet. Eine Ladeschaltung mit Micro-USB-Buchse ist eingebaut. Ueber diese erfolgt eine Schnelladung mit 500mA sowie auf Wunsch parallel zu einer "Erhaltungsladung" die dauerhafte Versorgung des Sensors. Hierzu dient ein uebliches USB-Ladegeraet, das mind. 1A dauerhaft liefern koennen sollte.


## Platine

Auf der Platine sind drei I2C-Steckplaetze fuer Sensoren vorgesehen. Neben dem [Sensirion SCD30](https://www.sensirion.com/de/umweltsensoren/kohlendioxidsensor/kohlendioxidsensoren-co2/) bietet sich ein [4-poliges BME280-Modul](https://www.ebay.de/itm/BME280-Temperatur-Luftdruck-Feuchtigkeit-Sensor-I2C-1-8-5V-Modul/114603492524) zur Luftdruck-Kompensation an. Der Drucksensor ist auch in der Firmware vorgesehen. Der dritte Steckplatz ist aktuell frei.
Als Display findet ein [1.54" Waveshare-E-Paper-Modul](https://www.waveshare.com/1.54inch-e-Paper-Module.htm) Verwendung.

[Schaltplan](https://github.com/HMSteve/HB-UNI-Sen-CO2/blob/main/PCB/HB-Uni-Sen-CO2_Schematic.pdf)

[Stueckliste](https://github.com/HMSteve/HB-UNI-Sen-CO2/blob/main/PCB/HB_UNI_Sen_CO2_BOM.pdf)

Ein paar Fotos vom Aufbau:

![PCB Top](https://github.com/HMSteve/HB-UNI-Sen-CO2/blob/main/Images/pcb_top_1.jpg)

![PCB Bottom](https://github.com/HMSteve/HB-UNI-Sen-CO2/blob/main/Images/pcb_bot_1.jpg)

![PCB Top 2](https://github.com/HMSteve/HB-UNI-Sen-CO2/blob/main/Images/pcb_top_2.jpg)

![PCB Bottom 2](https://github.com/HMSteve/HB-UNI-Sen-CO2/blob/main/Images/pcb_bot_2.jpg)

Hinweis: Da leider das Displaymodul und nicht der aktive Displaybereich mittig auf der Platine angeordnet wurden, ist die Duo-LED vor dem Gehaeuseeinbau durch Zurechtbiegen der Anschluesse ca. 3mm versetzt zu bestuecken. Die 3mm-LEDs duerfen den Platinenrand nur wenig ueberragen, um den Einbau ins Gehaeuse zu ermoeglichen.


## Software

### Addon zur Geraeteunterstuetzung

Sofern noch nicht vorhanden oder nicht aktuell, ist das [Addon](https://github.com/HMSteve/SG-HB-Devices-Addon/raw/master/CCU_RM/sg-hb-devices-addon.tgz) auf der CCU zu installieren. Der Sensor benoetigt mindestens Version 1.23.

### Firmware

Es wird ein ATMega1284p verwendet, da der Speicherplatz des ATMega328p nicht ausreicht. Deshalb ist bei Nutzung der Arduino IDE zunaechst eine zusaetzliche sog. Boardverwalter-URL (https://mcudude.github.io/MightyCore/package_MCUdude_MightyCore_index.json) in den Voreinstellungen zu hinterlegen. Folgende Boardeinstellungen sind dann auszuwaehlen:

![Boardeinstellungen](https://github.com/HMSteve/HB-UNI-Sen-CO2/blob/main/Images/arduino_board_config.jpg)

Danach kann der Bootloader geflashed werden.

Um die Firmware zu kompilieren, muessen sich die .ino sowie die .h Dateien im gleichen Verzeichnis befinden, das ./sensors-Verzeichnis darunter. Zudem muss eine Reihe von Bibliotheken ueber den Library Manager eingebunden werden:
- AskSinPP
- EnableInterrupt
- LowPower
- SparkFun SCD30 Arduino Library
- Adafruit BME280 Library
- GxEPD
- Adafruit GFX

Anschliessend sollte die Firmware problemlos kompilierbar und das Device nach dem Flashen anlernbar sein.


## Bedienung

Es gibt neben der AskSinPP-Config-Taste eine weitere Bedientaste. Ein kurzer Druck auf diese schaltet die Ampel-LED ein bzw. aus, ein langer Druck fuehrt zu einer forced calibration des SCD30. Hierzu sollte sich der Sensor bereits einige Minuten an der frischen Luft ohne nennenswerte Druckschwankungen durch Wind befunden haben. Der lange Tastendruck kalibriert dann den Sensor auf den im WebUI hinterlegten CO2-Referenzwert (im Freien ca. 410ppm). Die automatische Kalibrierung wird nicht verwendet auf Basis der These, dass ein Wegdriften der Auto-Kalibrierung bei nicht ausreichendem regelmaessigem Lueften problematischer ist als die Alterungsdrift nach forced calibration. Langzeitbeobachtungen hierzu fehlen mir jedoch.

Unterschreitet die Akkuspannung 2.2V, wird ein Warnsymbol im Display angezeigt und ein USB-Ladegerate sollte angeschlossen werden. Geschieht das nicht und die Spannung sinkt unter 2.0V, schaltet sich der Sensor zum Schutz vor Tiefentladung ab und zeigt dies an. Ein Reset (Wiedereinschalten) erfolgt automatisch beim Anschluss eines Ladegeraetes. Die gruene LED zeigt das Anliegen der Ladespannung, die gelbe den Schnelladevorgang. Es ist zu beachten, dass die Erwaermung beim Schnelladen natuerlich die Messwerte im Gehaeuse verfaelscht.

Die Schaltschwellen der Ampelfarben, die Hoehe ueber NN sowie ein vom SCD30-Temperaturmesswert zu subtrahierender Offset zur Korrektur der Anzeige koennen im WebUI konfiguriert werden.

![WebUI Settings](https://github.com/HMSteve/HB-UNI-Sen-CO2/blob/main/Images/webui_settings.jpg)

Die Messwerte werden dann so angezeigt:

![WebUI Status](https://github.com/HMSteve/HB-UNI-Sen-CO2/blob/main/Images/webui_status.jpg)

### Hinweise zum Energieverbrauch

Der SCD30 zieht einen nennenswerten Ruhestrom (mind. 5mA). Ein Power Cycling wird vom Hersteller nicht empfohlen, siehe [hier](https://github.com/HMSteve/HB-UNI-Sen-CO2/blob/main/Addl/CD_AN_SCD30_Low_Power_Mode_D2.pdf).
Damit ist fuer das Geraet bei der Voreinstellung einer Sensor-Abtastperiode von 16s eine Akkulaufzeit von hoechstens 5 Tagen zu erwarten. Das heisst, der mobile Betrieb waehrend eines oder einiger (Arbeits)tage ist problemslos moeglich. Der Dauerbetrieb sollte jedoch eher mit USB-Netzteil erfolgen. Dabei versorgt nach Ende der Schnelladung der von der MAX712-Ladeschaltung gelieferte, von R20 gemaess [Datenblatt MAX712](https://github.com/HMSteve/HB-UNI-Sen-CO2/blob/main/Addl/MAX712-MAX713.pdf) feinjustierte Erhaltungsladestrom das Device.  

Hier eine Messung zum Stromverbrauch:

![Power Consumption Detail](https://github.com/HMSteve/HB-UNI-Sen-CO2/blob/main/Images/power_consumption_detail.jpg)

Ein Excel-Workbook zum power budgeting findet sich [hier](https://github.com/HMSteve/HB-UNI-Sen-CO2/blob/main/Addl/BatteryBudgeting.xlsx).


## Gehaeuse

Zwischen Display und Platine sind vier zu druckende Abstandshuelsen einzusetzen. Das Gehaeuse stellt beim Drucken keine groesseren Herausforderungen. Die LED-Loecher sind ggf. mit 10.5mm bzw. 3.5mm aufzubohren, die Loecher im Boden mit 3mm.
Fuer die Befestigung der Platine sind [M3-Einpressmuttern](https://www.amazon.de/dp/B08BCRZZS3) vorgesehen. Die Befestigung der Platine im Gehaeuse erfolgt mit M3x10 Distanzbolzen. Die Rueckwand wird dann mit M3x10 Zylinderkopfschrauben an diesen befestigt.


## Disclaimer

Die Nutzung der hier veroeffentlichten Inhalte erfolgt vollstaendig auf eigenes Risiko und ohne jede Gewaehr.


## Lizenz

**Creative Commons BY-NC-SA**<br>
Give Credit, NonCommercial, ShareAlike

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.
