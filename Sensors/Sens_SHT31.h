//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-11-02 jp112sdl Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENS_SHT31_h__
#define __SENS_SHT31_h__

#include <Sensors.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>

namespace as {

//https://github.com/adafruit/Adafruit_SHT31
template <uint8_t ADDRESS=0x44>
//class Sens_SHT31 : public Temperature, public Humidity {
class Sens_SHT31 : public Sensor {
  Adafruit_SHT31 _sht31;
  int16_t   _temperature;
  uint8_t   _humidity;

public:
  Sens_SHT31 ()
  : _temperature(0)
  , _humidity(0)
{
}

  void init () {
    _present = _sht31.begin(ADDRESS);
    DPRINT(F("SHT31 "));
    if (_present) {
      DPRINTLN(F("OK"));
    } else {
      DPRINTLN(F("ERROR"));
    }
  }

  bool measure (__attribute__((unused)) bool async=false) {
    if( present() == true ) {
      _temperature = _sht31.readTemperature() * 10;
      _humidity = _sht31.readHumidity();
      DPRINT("SHT31    Temperature   : ");
      DDECLN(_temperature);
      DPRINT("SHT31    Humidity      : ");
      DDECLN(_humidity);
      return true;
    }
    return false;
  }
  int16_t temperature() { return _temperature; }
  uint8_t humidity() { return _humidity; }
};

}

#endif
