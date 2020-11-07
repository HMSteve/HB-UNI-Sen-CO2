# Universalsensor (TVOC, Luftqualitaet, Temperatur und Luftfeuchte)

Eine AskSinPP-Implementierung eines Innenraum-Sensos mit dem IAQ-Sensor SGPC3 sowie dem Temperatur- und Feuchtesensor SHT31. Als Hardwarebasis dient z.B. meine ATMega1284P-Universalsensorplatine:

[AskSinPP 1284 Board](https://github.com/HMSteve/PCBs/tree/master/AskSinPP_1284_Board)

Da SGPC3 Breakout Boards im Standardformat mit I2C-Schnittstelle nicht am Markt zu finden waren, habe ich ein eigenes erstellt: [SGPC3 Breakout Board](https://github.com/HMSteve/PCBs/tree/master/SGPC3_BreakoutBoard). Grundsaetzlich kann auch der leichter verfuegbare SGP30 genutzt werden, wenn der Sketch entsprechend angepasst wird, jedoch ist dieser bzgl. des Stromverbrauchs nicht fuer den Batteriebetrieb optimiert.


## Software

Sofern noch nicht vorhanden oder nicht aktuell, ist das [Addon](https://github.com/HMSteve/SG-HB-Devices-Addon/raw/master/CCU_RM/sg-hb-devices-addon.tgz) auf der CCU zu installieren. Der Sensor benoetigt mindestens Version 1.10.

Die AskSinPP-Platine wird wie ueblich geflasht und kann dann angelernt werden.


## Disclaimer

Die Nutzung der hier veroeffentlichten Inhalte erfolgt vollstaendig auf eigenes Risiko und ohne jede Gewaehr.


## Lizenz

**Creative Commons BY-NC-SA**<br>
Give Credit, NonCommercial, ShareAlike

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.
