//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2019-02-28 jp112sdl (Creative Commons)
//- -----------------------------------------------------------------------------------------------------------------------
// 2020-11-07 CO2 Sensor, HMSteve (cc)
//- -----------------------------------------------------------------------------------------------------------------------

//#define NDEBUG   // disable all serial debug messages  //necessary to fit 328p!!!
// #define USE_CC1101_ALT_FREQ_86835  //use alternative frequency to compensate not correct working cc1101 modules
//#define SENSOR_ONLY

#define EI_NOTEXTERNAL
#define M1284P // select pin config for ATMega1284p board
#define useBMP280 //use pressure sensor fort compensation

#define SCD30_MEASUREMENT_INTERVAL 8
#define BAT_VOLT_LOW        18  // 1.8V
#define BAT_VOLT_CRITICAL   16  // 1.6V

#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>
#include <Register.h>
#include <MultiChannelDevice.h>
#include "sensors/Sens_SCD30.h"
#include "sensors/tmBattery.h"  //SG: added from Tom's UniSensor project
#if defined useBMP280
  #include "sensors/Sens_BMP280.h"
#endif

#if defined M1284P
 // Stephans AskSinPP 1284 Board v1.1
 #define CC1101_CS_PIN       4
 #define CC1101_GDO0_PIN     2
 #define CC1101_SCK_PIN      7 
 #define CC1101_MOSI_PIN     5 
 #define CC1101_MISO_PIN     6 
 #define LED_PIN 14             //LEDs on PD6 (Arduino Pin 14) and PD7 (Arduino Pin 15) 
 #define LED_PIN2 15
 #define CONFIG_BUTTON_PIN 13
 #define CC1101_PWR_SW_PIN 27
#else
  // Stephans AskSinPP Universal Board v1.0
  #define LED_PIN 6
  #define CONFIG_BUTTON_PIN 5
#endif


#define PEERS_PER_CHANNEL 6
//#define BATT_SENSOR tmBatteryResDiv<A0, A1, 5700>  //SG: taken from Tom's Unisensor01
// tmBatteryLoad: sense pin A0, activation pin D9, Faktor = Rges/Rlow*1000, z.B. 10/30 Ohm, Faktor 40/10*1000 = 4000, 200ms Belastung vor Messung
// 1248p has 2.56V ARef, 328p has 1.1V ARef
#if defined M1284P
  #define BATT_SENSOR tmBatteryLoad<A6, 12, (uint16_t)(4000.0*2.56/1.1), 200>  
#else
  #define BATT_SENSOR tmBatteryLoad<A6, 12, 4000, 200>  
#endif


// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
  {0xf8, 0x22, 0x01},     // Device ID
  "SGSENCO201",           // Device Serial
  {0xf8, 0x22},           // Device Model Indoor //orig 0xf1d1
  0x10,                   // Firmware Version
  as::DeviceType::THSensor, // Device Type
  {0x01, 0x00}            // Info Bytes
};

/**
   Configure the used hardware
*/
#if defined M1284P
  typedef AvrSPI<CC1101_CS_PIN, CC1101_MOSI_PIN, CC1101_MISO_PIN, CC1101_SCK_PIN> SPIType;
  typedef Radio<SPIType, CC1101_GDO0_PIN> RadioType;
#else
  typedef AvrSPI<10, 11, 12, 13> SPIType;
  typedef Radio<SPIType, 2> RadioType;
#endif

typedef StatusLed<LED_PIN> LedType;
//typedef AskSin<LedType, BatterySensor, RadioType> BaseHal;
typedef AskSin<LedType, BATT_SENSOR, RadioType> BaseHal;
class Hal : public BaseHal {
  public:
    void init (const HMID& id) {
      BaseHal::init(id);
#ifdef USE_CC1101_ALT_FREQ_86835
      // 2165E8 == 868.35 MHz
      radio.initReg(CC1101_FREQ2, 0x21);
      radio.initReg(CC1101_FREQ1, 0x65);
      radio.initReg(CC1101_FREQ0, 0xE8);
#endif
      // measure battery every a*b*c seconds
      battery.init(seconds2ticks(60UL * 60 * 6), sysclock);  // 60UL * 60 for 1hour
      battery.low(BAT_VOLT_LOW);
      battery.critical(BAT_VOLT_CRITICAL);
    }

    bool runready () {
      return sysclock.runready() || BaseHal::runready();
    }
} hal;

DEFREGISTER(Reg0, MASTERID_REGS, DREG_LEDMODE, DREG_LOWBATLIMIT, DREG_TRANSMITTRYMAX, 0x20, 0x21, 0x22, 0x23, 0x24)
class SensorList0 : public RegList0<Reg0> {
  public:
    SensorList0(uint16_t addr) : RegList0<Reg0>(addr) {}

    bool updIntervall (uint16_t value) const {
      return this->writeRegister(0x20, (value >> 8) & 0xff) && this->writeRegister(0x21, value & 0xff);
    }
    uint16_t updIntervall () const {
      return (this->readRegister(0x20, 0) << 8) + this->readRegister(0x21, 0);
    }
    
    bool altitude (uint16_t value) const {
      return this->writeRegister(0x22, (value >> 8) & 0xff) && this->writeRegister(0x23, value & 0xff);
    }
    uint16_t altitude () const {
      return (this->readRegister(0x22, 0) << 8) + this->readRegister(0x23, 0);
    }

    bool tempOffset10 (uint8_t value) const {
      return this->writeRegister(0x24, value & 0xff);
    }
    uint16_t tempOffset10 () const {
      return this->readRegister(0x24, 0);
    }    


    void defaults () {
      clear();
      ledMode(1);
      lowBatLimit(BAT_VOLT_LOW);
      transmitDevTryMax(6);     
      updIntervall(11); //seconds
      altitude(62); //meters
      tempOffset10(8); //temperature offset for SCD30 calib: 15 means 1.5K
    }
};


class WeatherEventMsg : public Message {
  public:
    void init(uint8_t msgcnt, int16_t temp, uint8_t humidity, uint16_t pressureNN, uint16_t co2, uint8_t volt, bool batlow) {
      uint8_t t1 = (temp >> 8) & 0x7f;
      uint8_t t2 = temp & 0xff;
      if ( batlow == true ) {
        t1 |= 0x80; // set bat low bit
      }
      // BIDI|WKMEUP every 20th msg
      uint8_t flags = BCAST;
      if ((msgcnt % 20) == 1) {
          flags = BIDI | WKMEUP;
      }      
      // Message Length (first byte param.): 11 + payload. Max. payload: 17 Bytes (https://www.youtube.com/watch?v=uAyzimU60jw)
      Message::init(17, msgcnt, 0x70, flags, t1, t2);
      pload[0] = humidity & 0xff;
      pload[1] = (pressureNN >> 8) & 0xff;           
      pload[2] = pressureNN & 0xff;          
      pload[3] = (co2 >> 8) & 0xff;           
      pload[4] = co2 & 0xff;         
      pload[5] = volt & 0xff;
    }
};

class WeatherChannel : public Channel<Hal, List1, EmptyList, List4, PEERS_PER_CHANNEL, SensorList0>, public Alarm {
    WeatherEventMsg msg;
    Sens_SCD30    scd30;
    #if defined useBMP280
      Sens_BMP280 bmp280;
    #endif   
    uint16_t      millis;
    uint16_t      pressureAmb = 1013; //mean pressure at sea level in hPa
    uint16_t      pressureNN = 0; //dummy value to be returned if no sensor measurement

  public:
    WeatherChannel () : Channel(), Alarm(10), millis(0) {}
    virtual ~WeatherChannel () {}

    virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
      uint8_t msgcnt = device().nextcount();
      // reactivate for next measure
      tick = delay();
      clock.add(*this);
      #if defined useBMP280
        bmp280.measure(this->device().getList0().altitude());
        pressureAmb = bmp280.pressureAmb()/10;
        pressureNN = bmp280.pressureNN();
      #endif
      scd30.measure(pressureAmb);
      DPRINT("Temp x10 / Hum / PressureNN x10 / PressureAmb / Batt x10 / CO2 = ");
      DDEC(scd30.temperature());DPRINT(" / ");
      DDEC(scd30.humidity());DPRINT(" / ");
      DDEC(pressureNN);DPRINT(" / ");
      DDEC(pressureAmb);DPRINT(" / ");     
      DDEC(device().battery().current() / 100);DPRINT(" / ");
      DDECLN(scd30.carbondioxide());
      msg.init( msgcnt, scd30.temperature(), scd30.humidity(), pressureNN, scd30.carbondioxide(), device().battery().current() / 100, device().battery().low());
      if (msg.flags() & Message::BCAST) {
        device().broadcastEvent(msg, *this);
      }
      else
      {
        device().sendPeerEvent(msg, *this);
      }
    }

    uint32_t delay () {
      return seconds2ticks(this->device().getList0().updIntervall());
    }
    void setup(Device<Hal, SensorList0>* dev, uint8_t number, uint16_t addr) {
      Channel::setup(dev, number, addr);
      scd30.init(this->device().getList0().altitude(), device().getList0().tempOffset10(), SCD30_MEASUREMENT_INTERVAL);
      #if defined useBMP280
        bmp280.init();
      #endif     
      sysclock.add(*this);
    }

    uint8_t status () const {
      return 0;
    }

    uint8_t flags () const {
      return 0;
    }
};

class SensChannelDevice : public MultiChannelDevice<Hal, WeatherChannel, 1, SensorList0> {
  public:
    typedef MultiChannelDevice<Hal, WeatherChannel, 1, SensorList0> TSDevice;
    SensChannelDevice(const DeviceInfo& info, uint16_t addr) : TSDevice(info, addr) 
    {
    }
    virtual ~SensChannelDevice () {}

    virtual void configChanged () {
      TSDevice::configChanged();
      DPRINTLN("* Config Changed       : List0");
      DPRINT(F("* LED Mode             : ")); DDECLN(this->getList0().ledMode());    
      DPRINT(F("* Low Bat Limit        : ")); DDECLN(this->getList0().lowBatLimit()); 
      DPRINT(F("* Sendeversuche        : ")); DDECLN(this->getList0().transmitDevTryMax());                   
      DPRINT(F("* SENDEINTERVALL       : ")); DDECLN(this->getList0().updIntervall());
      DPRINT(F("* Hoehe ueber NN       : ")); DDECLN(this->getList0().altitude());
      DPRINT(F("* Temp Offset x10      : ")); DDECLN(this->getList0().tempOffset10());      
    }
};

SensChannelDevice sdev(devinfo, 0x20);
ConfigButton<SensChannelDevice> cfgBtn(sdev);

void setup () {
  //SG: switch on MOSFET to power CC1101
  pinMode(CC1101_PWR_SW_PIN, OUTPUT);
  digitalWrite (CC1101_PWR_SW_PIN, LOW);

  
  DINIT(57600, ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  buttonISR(cfgBtn, CONFIG_BUTTON_PIN);
  sdev.initDone();
  DPRINT("List0 dump: "); sdev.getList0().dump();
}

void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if ( worked == false && poll == false ) {
    if (hal.battery.critical()) {
      // this call will never return
      hal.activity.sleepForever(hal);
    }    
    // if nothing to do - go to sleep
    hal.activity.savePower<Sleep<>>(hal);
  }
}
