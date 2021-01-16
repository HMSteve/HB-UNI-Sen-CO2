namespace as {

template <class DEVTYPE>
class UserButton : public Button {
  DEVTYPE& device;
  
public:
  UserButton (DEVTYPE& dev, uint8_t longpresstime=3) : device(dev) {
    this->setLongPressTime(seconds2ticks(longpresstime));
  }
 
  virtual ~UserButton () {}
 
  virtual void state (uint8_t s) {
    uint8_t old = Button::state();
    Button::state(s);
    DPRINT("usr btn state : ");DPRINTLN(s);
    if( s == Button::released ) {
      device.channel(0).toggleTrafficLight();
      device.led2.set(LedStates::send);  
    }    
    else if( s == Button::longpressed ) {
      DPRINTLN("SCD30: setting forced recalibration factor");
      bool fc = device.channel(0).forceCalibSCD30(SCD30_REFERENCE_CO2);
      if (fc==true) {
        device.led2.set(LedStates::key_long);
      }
      else {
        device.led2.set(LedStates::failure);
      }
    }
  }
};

}
