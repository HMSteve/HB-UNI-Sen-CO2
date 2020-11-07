
//---------------------------------------------------------
// Sens_SCD30
// 2019-07-14 Tom Major (Creative Commons)
// https://creativecommons.org/licenses/by-nc-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, NonCommercial, ShareAlike
// +++
// AskSin++ 2016-10-31 papa Creative Commons
//---------------------------------------------------------

#ifndef _SENS_SCD30_H_
#define _SENS_SCD30_H_

#include <Sensors.h>
#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30

#define COMMAND_STOP_MEASUREMENT 0x0104 // supplemented in order to stop the measurements in case of an empty accumulator battery

namespace as {

class Sens_SCD30 : public Sensor {

    int16_t  _temperature;
    uint32_t _carbondioxide;
    uint8_t  _humidity;
    SCD30    _scd30;

    void measureRaw()
    {
        DPRINTLN("");
        while (!_scd30.dataAvailable())
        {
            delay(1000);
            DPRINT(".");
            if (_scd30.dataAvailable())
            {
              break;
            }
        }
        if (_scd30.dataAvailable())
        {
              _temperature      = (int16_t)(_scd30.getTemperature() * 10);
              _carbondioxide    = (uint16_t)(_scd30.getCO2());
              _humidity         = (uint8_t)(_scd30.getHumidity());
              DPRINTLN("");
        }
    }

public:
    Sens_SCD30()
        : _temperature(0)
        , _carbondioxide(0)
        , _humidity(0)
    {
    }

    void init(uint16_t height, uint16_t ambient_pressure, uint16_t temperature_correction, uint16_t update_intervall)
    {
        Wire.begin();

        delay(200);

        if (_scd30.begin() == false)
        {
          DPRINTLN("Error: no Sensirion SCD30 CO2 sensor found");
          DPRINTLN("Please check wiring. Freezing...");
          while (1);
        }
        else
        {
          DPRINTLN(F("SCD30 found"));
          _present = true;

          _scd30.setMeasurementInterval(update_intervall/4);   // Change number of seconds between measurements: 2 to 1800 (30 minutes)
                                                                  // update_intervall = 240 sec; SCD30 measurement intervall: 60 sec to allow SCD30 internal
                                                                  // hardware oversampling

          _scd30.setAltitudeCompensation(height);              // Set altitude of the sensor in m

          _scd30.setAmbientPressure(ambient_pressure);         // Current ambient pressure in mBar: 700 to 1200

          _scd30.setAutoSelfCalibration(true);                 // enable autocalibration

          float offset = _scd30.getTemperatureOffset();
          DPRINT("Current temp offset: ");
          DPRINT(offset);
          DPRINTLN(" deg C");

          _scd30.setTemperatureOffset((float)temperature_correction/10.0); //Optionally we can set temperature offset to 5°C
          DPRINT("New temp offset: ");
          DPRINT((float)temperature_correction/10.0);
          DPRINTLN(" deg C");
        }
    }

    void stop_measurements()
    {
        _scd30.sendCommand(COMMAND_STOP_MEASUREMENT);
        DPRINTLN("Stop continuous measurements of SCD30");
    }

    void measure()
    {
        _temperature = _carbondioxide = _humidity = 0;
        if (_present == true) {
            measureRaw();
            DPRINT(F("SCD30 Temperature x10  : "));
            DDECLN(_temperature);
            DPRINT(F("SCD30 Carbondioxide    : "));
            DDECLN(_carbondioxide);
            DPRINT(F("SCD30 Humidity         : "));
            DDECLN(_humidity);
        }
    }

    int16_t  temperature() { return _temperature; }
    uint32_t carbondioxide() { return _carbondioxide; }
    uint8_t  humidity() { return _humidity; }
};

}

#endif
