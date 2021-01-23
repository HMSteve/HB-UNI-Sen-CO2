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
    if( s == Button::released ) {
      device.channel(0).toggleTrafficLight();
      device.led2.set(LedStates::send);  
    }    
    else if( s == Button::longpressed ) {
      bool fc = device.channel(0).forceCalibSCD30();
      if (fc==true) {
        DPRINTLN("SCD30: forced calibration done");       
        device.led2.set(LedStates::key_long);
      }
      else {
        DPRINTLN("SCD30: forced calibration FAILED");          
        device.led2.set(LedStates::failure);
      }
    }
  }
};

}
