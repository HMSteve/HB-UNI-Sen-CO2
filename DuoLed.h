
namespace as {

template <uint8_t LEDPIN1, uint8_t LEDPIN2, class PINTYPE1=ArduinoPins, class PINTYPE2=ArduinoPins>
class DuoLed : LedStates {
  private:
    Led<PINTYPE1> ledgn;
    Led<PINTYPE2> ledrt;
    
  public:
    DuoLed() {};
    
    virtual ~DuoLed() {};

    void init() 
    {
      ledgn.init(LEDPIN1);
      ledrt.init(LEDPIN2);
    }

    void setGreen() 
    {
      ledgn.ledOn();   
      ledrt.ledOff(); 
    };

    void setAmber() 
    {
      ledgn.ledOn();   
      ledrt.ledOn(); 
    };

    void setRed() 
    {
      ledgn.ledOff();   
      ledrt.ledOn(); 
    };

    void setOff() 
    {
      ledgn.ledOff();   
      ledrt.ledOff(); 
    };

};

}
