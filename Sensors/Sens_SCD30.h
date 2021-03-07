
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

#define COMMAND_STOP_MEASUREMENT        0x0104 // supplemented in order to stop the measurements in case of an empty accumulator battery
#define COMMAND_SOFT_RESET              0xD304 // supplemented to use before begin in case low battery caused undefined state
#define COMMAND_READ_FIRMWARE_VERSION   0xD100

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
            delay(100);
            DPRINT(".");
            if (_scd30.dataAvailable())
            {
              break;
            }
        }
        if (_scd30.dataAvailable())
        {
              _temperature      = (int16_t)(_scd30.getTemperature() * 10.0);
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


    void init(uint16_t altitude, uint16_t temperature_correction, uint16_t measurement_interval, bool auto_self_calib)
    {
        Wire.begin();
        //_scd30.sendCommand(COMMAND_SOFT_RESET);
        delay(200);

        if (_scd30.begin() == false)
        {
          DPRINTLN("Error: no Sensirion SCD30 CO2 sensor found");
        }
        else
        {
          DPRINTLN(F("SCD30 found"));
          _present = true;
          _scd30.setMeasurementInterval(measurement_interval);               // Change number of seconds between measurements: 2 to 1800 (30 minutes)
          _scd30.setAltitudeCompensation(altitude);                          // Set altitude of the sensor in m
          _scd30.setAutoSelfCalibration(auto_self_calib);                    // enable/disable auto self calibration, sensor needs to see fresh air regularly!
          _scd30.setTemperatureOffset((float)temperature_correction/10.0);   //temp offset between on-board sensor and ambient temp
        }
    }


    bool setForcedRecalibrationFactor(uint16_t concentration)
    {
      return(_scd30.setForcedRecalibrationFactor(concentration));
    }


    bool setSamplingInterval(uint16_t sampling_interval)
    {
      return(_scd30.setMeasurementInterval(sampling_interval));
    }


    float getTemperatureOffset(void)
    {
      return _scd30.getTemperatureOffset();
    }


    void stop_measurements()
    {
        _scd30.sendCommand(COMMAND_STOP_MEASUREMENT);
        DPRINTLN("Stop continuous measurements of SCD30");
    }


    void measure(uint16_t pressureAmb)
    {
        _temperature = _carbondioxide = _humidity = 0;
        if (_present == true) {
            _scd30.setAmbientPressure(pressureAmb);
            measureRaw();
            DPRINT(F("SCD30   Temperature   : "));
            DDECLN(_temperature);
            DPRINT(F("SCD30   Humidity      : "));
            DDECLN(_humidity);
            DPRINT(F("SCD30   CO2           : "));
            DDECLN(_carbondioxide);
        }
    }


    void measure()
    {
      measure(1013); //overload: if no ambient pressure passed, use mean pressure at sea level
    }


    void read_firmware_version()
    {
      DPRINTLN("bla");
    }

    int16_t  temperature() { return _temperature; }
    uint32_t carbondioxide() { return _carbondioxide; }
    uint8_t  humidity() { return _humidity; }
};

}

#endif
