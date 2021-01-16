//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2019-02-28 jp112sdl (Creative Commons)
//- -----------------------------------------------------------------------------------------------------------------------
// 2020-11-07 CO2 Sensor, HMSteve (cc)
//- -----------------------------------------------------------------------------------------------------------------------

//#define NDEBUG   // disable all serial debug messages  //necessary to fit 328p!!!
//#define SENSOR_ONLY

#define EI_NOTEXTERNAL
#define useBME280 //use pressure sensor fort compensation

#define SCD30_MEASUREMENT_INTERVAL 8  // seconds
#define SCD30_REFERENCE_CO2 410 // 410ppm used for forced calib in fresh air 
#define BAT_VOLT_LOW        22  // 2.2V for 2x Eneloop 
#define BAT_VOLT_CRITICAL   20  // 2.0V for 2x Eneloop

#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>
#include <Register.h>
#include <MultiChannelDevice.h>
#include "sensors/Sens_SCD30.h"
#include "sensors/tmBattery.h"  //SG: added from Tom's UniSensor project
#if defined useBME280
  #include "sensors/Sens_BME280.h"
#endif
#include "EPDisplay.h"
#include "DuoLed.h"
#include "UserButton.h"


// Pin definitions Stephan's HB-UNI-Sen-CO2 Board v1.0
#define CC1101_CS_PIN       4
#define CC1101_GDO0_PIN     2
#define CC1101_SCK_PIN      7 
#define CC1101_MOSI_PIN     5 
#define CC1101_MISO_PIN     6 
#define LED_PIN 12              
#define LED2_PIN 15
#define DUOLED_GN_PIN 18
#define DUOLED_RT_PIN 19
#define CONFIG_BUTTON_PIN 14
#define USER_BUTTON_PIN  13 
#define CC1101_PWR_SW_PIN 27


#define PEERS_PER_CHANNEL 6
// tmBatteryLoad: sense pin A6, activation pin 26, Faktor = Rges/Rlow*1000, z.B. 10/30 Ohm, Faktor 40/10*1000 = 4000, 200ms Belastung vor Messung
// 1248p has 2.56V ARef, 328p has 1.1V ARef, so output needs to be scaled up
#define BATT_SENSOR tmBatteryLoad<A6, 26, (uint16_t)(4000.0*2.56/1.1), 200>  



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




//Configure the used hardware
typedef AvrSPI<CC1101_CS_PIN, CC1101_MOSI_PIN, CC1101_MISO_PIN, CC1101_SCK_PIN> SPIType;
typedef Radio<SPIType, CC1101_GDO0_PIN> RadioType;
typedef StatusLed<LED_PIN> LedType;
typedef AskSin<LedType, BATT_SENSOR, RadioType> BaseHal;
typedef StatusLed<LED2_PIN> Led2Type;
typedef DuoLed<DUOLED_GN_PIN, DUOLED_RT_PIN> DuoLedType;


class Hal : public BaseHal {
  public:
    void init (const HMID& id) {
      BaseHal::init(id);
      // measure battery every a*b*c seconds
      battery.init(seconds2ticks(60UL * 30 * 1), sysclock);  
      battery.low(BAT_VOLT_LOW);
      battery.critical(BAT_VOLT_CRITICAL);
    }

    bool runready () {
      return sysclock.runready() || BaseHal::runready();
    }
} hal;


DEFREGISTER(Reg0, MASTERID_REGS, DREG_LEDMODE, DREG_TRANSMITTRYMAX, 0x20, 0x21, 0x22, 0x23, 0x24)
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
      transmitDevTryMax(3);     
      updIntervall(11); //seconds
      altitude(62); //meters
      tempOffset10(0); //temperature offset for SCD30 calib: 15 means 1.5K
    }
};


class WeatherEventMsg : public Message {
  public:
    void init(uint8_t msgcnt, int16_t temp, uint8_t humidity, uint16_t pressureNN, uint16_t co2, uint16_t volt, bool batlow) {
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
      Message::init(18, msgcnt, 0x70, flags, t1, t2);
      pload[0] = humidity & 0xff;
      pload[1] = (pressureNN >> 8) & 0xff;           
      pload[2] = pressureNN & 0xff;          
      pload[3] = (co2 >> 8) & 0xff;           
      pload[4] = co2 & 0xff;         
      pload[5] = (volt >>8) & 0xff;
      pload[6] = volt & 0xff;      
    }
};




class WeatherChannel : public Channel<Hal, List1, EmptyList, List4, PEERS_PER_CHANNEL, SensorList0>, public Alarm {
    WeatherEventMsg     msg;
    uint16_t            millis;
    uint16_t            pressureAmb = 1013; //mean pressure at sea level in hPa
    uint16_t            pressureNN = 0; //dummy value to be returned if no sensor measurement
    uint16_t            co2 = 0;
    uint16_t            temp10 = 0;
    uint8_t             humidity = 0;
    DuoLedType          trafficLight; 
    bool                trafficLightEnabled = false;    
    Sens_SCD30 scd30;        
#if defined useBME280
    Sens_BME280       bme280;
#endif       
       

  public:
    WeatherChannel () : Channel(), Alarm(10), millis(0) {}
    
    virtual ~WeatherChannel () {}

    virtual void trigger (__attribute__ ((unused)) AlarmClock& clock) {
      uint8_t msgcnt = device().nextcount();
      // reactivate for next measure
      tick = delay();
      clock.add(*this);
      #if defined useBME280
        bme280.measure(this->device().getList0().altitude());
        pressureAmb = bme280.pressure()/10;
        pressureNN = bme280.pressureNN();
      #endif
      scd30.measure(pressureAmb);
      co2 = scd30.carbondioxide();
      temp10 = scd30.temperature();
      humidity = scd30.humidity();
      
      DPRINT("Temp x10 / Hum / PressureNN x10 / PressureAmb / Batt x10 / CO2 = ");
      DDEC(scd30.temperature());DPRINT(" / ");
      DDEC(scd30.humidity());DPRINT(" / ");
      DDEC(pressureNN);DPRINT(" / ");
      DDEC(pressureAmb);DPRINT(" / ");     
      DDEC(device().battery().current() / 10);DPRINT(" / ");
      DDECLN(scd30.carbondioxide());
      
      msg.init( msgcnt, scd30.temperature(), scd30.humidity(), pressureNN, scd30.carbondioxide(), device().battery().current() / 10, device().battery().low());
      if (msg.flags() & Message::BCAST) 
      {
        device().broadcastEvent(msg, *this);
      }
      else
      {
        device().sendPeerEvent(msg, *this);
      }
      
      DisplayData.co2 = co2;
      DisplayData.temperature = temp10 / 10.0;
      DisplayData.humidity = humidity;   
      DisplayData.lowbatt = device().battery().low();    
      //display.drawPaged(MeasurementsDisplay);
     
      setTrafficLight();
    }


    uint32_t delay () {
      return seconds2ticks(this->device().getList0().updIntervall());
    }
    
    void setup(Device<Hal, SensorList0>* dev, uint8_t number, uint16_t addr) {
      Channel::setup(dev, number, addr);
      scd30.init(this->device().getList0().altitude(), device().getList0().tempOffset10(), SCD30_MEASUREMENT_INTERVAL, false);
      #if defined useBME280
        bme280.init();
      #endif
      trafficLight.init();
      sysclock.add(*this);
    }

    uint8_t status () const {
      return 0;
    }

    uint8_t flags () const {
      return 0;
    }


    bool forceCalibSCD30(uint16_t calibValue = 400)
    {
      return(scd30.setForcedRecalibrationFactor(calibValue));
    }


    void stopSCD30()
    {
      scd30.stop_measurements();   
    }


    void setTrafficLight()
    {
      if (trafficLightEnabled)
      {
        if (co2 < 1000) trafficLight.setGreen();
        if ((co2 >= 1000) and (co2 < 2000)) trafficLight.setAmber();
        if (co2 >= 2000) trafficLight.setRed();
      }
      else
      {
        trafficLight.setOff();
      }
    }

    void toggleTrafficLight()
    {
      trafficLightEnabled = !trafficLightEnabled;
      setTrafficLight();
    }
};




class SensChannelDevice : public MultiChannelDevice<Hal, WeatherChannel, 1, SensorList0> {
  public:
    typedef MultiChannelDevice<Hal, WeatherChannel, 1, SensorList0> TSDevice;
    Led2Type    led2;
    
    SensChannelDevice(const DeviceInfo& info, uint16_t addr) : TSDevice(info, addr) 
    {
      led2.init();
    }
    
    virtual ~SensChannelDevice () {}

    virtual void configChanged () {
      TSDevice::configChanged();
      DPRINTLN("* Config Changed       : List0");
      DPRINT(F("* LED Mode             : ")); DDECLN(this->getList0().ledMode());    
      DPRINT(F("* Sendeversuche        : ")); DDECLN(this->getList0().transmitDevTryMax());                   
      DPRINT(F("* SENDEINTERVALL       : ")); DDECLN(this->getList0().updIntervall());
      DPRINT(F("* Hoehe ueber NN       : ")); DDECLN(this->getList0().altitude());
      DPRINT(F("* Temp Offset x10      : ")); DDECLN(this->getList0().tempOffset10());      
    }
};



SensChannelDevice sdev(devinfo, 0x20);
ConfigButton<SensChannelDevice> cfgBtn(sdev);
UserButton<SensChannelDevice> usrBtn(sdev);


void setup () {
  //SG: switch on MOSFET to power CC1101 and sensors
  pinMode(CC1101_PWR_SW_PIN, OUTPUT);
  digitalWrite (CC1101_PWR_SW_PIN, LOW);
  DINIT(57600, ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  buttonISR(cfgBtn, CONFIG_BUTTON_PIN);
  buttonISR(usrBtn, USER_BUTTON_PIN);  
  sdev.initDone();
  //DPRINT("List0 dump: "); sdev.getList0().dump();
  display.init();
}


void loop() {
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if ( worked == false && poll == false ) {
    if (hal.battery.critical()) {
      // this call will never return
      DPRINTLN("!!!Shutting down due to critical battery voltage!!!");
      sdev.channel(0).stopSCD30();           
      display.drawPaged(EmptyBattDisplay);
      //wait a bit to let paged epd refresh finish
      delay(5000);
      pinMode(CC1101_PWR_SW_PIN, INPUT);
      hal.activity.sleepForever(hal);      
    }    
    // if nothing to do - go to sleep
    DPRINTLN("...zzz");
    hal.activity.savePower<Sleep<>>(hal);
  }
}
