//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2020-07-04 Stephan * Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef __SENS_SGPC3_h__
#define __SENS_SGPC3_h__

#include <Sensors.h>
#include <Wire.h>
#include <sgpc3.h>

namespace as {


class Sens_SGPC3 : public Sensor {
  SGPC3 _sgpc3;
  uint16_t   _tvoc;
  uint8_t   _iaq;

public:
  Sens_SGPC3 ()
  : _tvoc(0)
  , _iaq(0)
{
}

  void init () {
    _present = (_sgpc3.ultraLowPower() == 0);
    if (_present) {
      DPRINTLN("SGPC3 ultra low power mode.");
      _present = (_sgpc3.initSGPC3() == 0);
      if (_present) {
        _sgpc3.setBaselineValue(0x89E4);
        DPRINTLN("SGPC3 init done.");
      }
    } else {
      DPRINTLN("SGPC3 ERROR");
    }
  }

  bool measure (int16_t temperature10, uint8_t humidity) {
    if( present() == true ) {
      float abshum = 216.7 * (humidity / 100.0 * 6.112 * exp(17.62 * temperature10 / 10.0 / (243.12 + temperature10 / 10.0)) / (273.15 + temperature10 / 10));
      uint16_t abshum8p8 = ((uint8_t)abshum << 8) + (abshum - (uint16_t)abshum) * 256;
      DPRINT("Temp10 = "); DPRINTLN(temperature10);
      DPRINT("RelHum = "); DPRINTLN(humidity);
      DPRINT("AbsHum = "); DPRINTLN(abshum);
      DPRINT("AbsHum8.8 = 0x"); DHEXLN(abshum8p8);
      _sgpc3.setAbsHumidityCompensation(abshum8p8);
      if (_sgpc3.measureIAQ() == 0) {
        _tvoc = _sgpc3.getTVOC();
        _iaq = 1;
        if (_tvoc > 65) {_iaq = 2;}
        if (_tvoc > 220) {_iaq = 3;}
        if (_tvoc > 660) {_iaq = 4;}
        if (_tvoc > 2200) {_iaq = 5;}
        DPRINT("SGPC3 TVOC in ppb: ");
        DDECLN(_tvoc);
        DPRINT("SGPC3 IAQ level (1 - 5): ");
        DDECLN(_iaq);
        return true;
      }
    }
    return false;
  }

  uint16_t tvoc() { return _tvoc; }
  uint8_t iaq() { return _iaq; }
};

}

#endif
