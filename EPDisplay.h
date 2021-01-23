
// EPaper setup
#include <GxEPD.h>
#include <GxGDEH0154D67/GxGDEH0154D67.h>

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

#define EPD_RST_PIN  21       
#define EPD_BUSY_PIN 20   
#define EPD_DC_PIN   22
#define EPD_CS_PIN   23 

GxIO_Class io(SPI, EPD_CS_PIN, EPD_DC_PIN, EPD_RST_PIN);
GxEPD_Class display(io, EPD_RST_PIN, EPD_BUSY_PIN);

typedef struct {
  uint16_t co2 = 0;
  float temperature = 0;
  uint8_t humidity = 0;
  bool lowbatt = false;
} DisplayDataType;

DisplayDataType DisplayData;


void MeasurementsDisplay()
// uint16_t co2, float temperature, uint8_t humidity
{
  int16_t tbx, tby; 
  uint16_t tbw, tbh, x, y;
  
  String strco2  = String(DisplayData.co2, DEC);
  String strtemp = String(DisplayData.temperature, 1);
  String strhum  =  String(DisplayData.humidity, DEC);  
  String strstatic = "";   
  
  display.setRotation(1);
  display.setTextColor(GxEPD_BLACK);
  display.fillScreen(GxEPD_BLACK);
  display.fillRect(2,2,196,196,GxEPD_WHITE);
  display.fillRect(0,99,200,2,GxEPD_BLACK);   
  display.fillRect(99,100,2,100,GxEPD_BLACK);  

  strstatic = "ppm CO2";
  display.setFont(&FreeMonoBold12pt7b);   
  display.getTextBounds(strstatic, 0, 0, &tbx, &tby, &tbw, &tbh);  
  //DPRINT("TEXTBOX: x="); DPRINT(tbx); DPRINT(" y="); DPRINT(tby); DPRINT(" w="); DPRINT(tbw); DPRINT(" h="); DPRINTLN(tbh);
  x = display.width() - tbw - tbx - 10 ;
  y = display.height() / 2 - tbh - tby - 10;  
  display.setCursor(x, y); 
  strstatic = "ppm CO";
  display.print(strstatic); 
  x = display.getCursorX();
  y = display.getCursorY() + 5;
  display.setCursor(x, y);   
  display.setFont(&FreeMonoBold9pt7b);   
  display.print("2");       
  
  strstatic = "oC";
  display.setFont(&FreeMonoBold12pt7b);   
  display.getTextBounds(strstatic, 0, 0, &tbx, &tby, &tbw, &tbh);  
  x = display.width() / 2 - tbw - tbx - 10 ;
  y = display.height() / 2 - tbh - tby - 10 + display.height() / 2;  
  display.setCursor(x, y - 5); 
  display.setFont(&FreeMonoBold9pt7b);     
  display.print("o"); 
  x = display.getCursorX();  
  display.setCursor(x, y);  
  display.setFont(&FreeMonoBold12pt7b);      
  display.print("C");   

  strstatic = "%rH";
  display.setFont(&FreeMonoBold12pt7b);   
  display.getTextBounds(strstatic, 0, 0, &tbx, &tby, &tbw, &tbh);  
  x = display.width() / 2 - tbw - tbx - 10 + display.width() / 2;
  display.setCursor(x, y); 
  display.print(strstatic);   
  
  
  display.setFont(&FreeMonoBold24pt7b);  
  display.getTextBounds(strco2, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  x = ((display.width() - tbw) / 2) - tbx;
  y = ((display.height() / 2 - tbh) / 2) - tby - display.height() / 20;
  display.setCursor(x, y);
  display.print(strco2);
  
  display.setFont(&FreeMonoBold18pt7b);
  display.getTextBounds(strtemp, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  x = ((display.width() / 2 - tbw) / 2) - tbx;
  y = ((display.height() / 2 - tbh) / 2) - tby + display.height() / 2 - display.height() / 40;
  display.setCursor(x, y);
  display.print(strtemp);

  display.getTextBounds(strhum, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  x = ((display.width() / 2 - tbw) / 2) - tbx + display.width() / 2;
  //y = ((display.height() / 2 - tbh) / 2) - tby + display.height() / 2;  
  display.setCursor(x, y);
  display.print(strhum);  

  if (DisplayData.lowbatt)
  {
      display.drawRect(15,25,11,5,GxEPD_BLACK);       
      display.drawRect(10,30,20,30,GxEPD_BLACK);         
      display.fillRect(10,60,20,10,GxEPD_BLACK);    
  }
}


void EmptyBattDisplay()
{
  int16_t tbx, tby; 
  uint16_t tbw, tbh, x, y;
  
  String strstatic = "";   

  DPRINTLN("Empty batt display");
  display.setRotation(1);
  display.setTextColor(GxEPD_BLACK);
  display.fillScreen(GxEPD_BLACK);
  display.fillRect(2,2,196,196,GxEPD_WHITE);


  display.drawRect(50,30,90,30,GxEPD_BLACK);       
  display.drawRect(140,35,10,20,GxEPD_BLACK);         
  display.drawLine(70,80,120,10,GxEPD_BLACK);    

  display.setFont(&FreeMonoBold12pt7b);  
  strstatic = "Connect"; 
  display.getTextBounds(strstatic, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  x = ((display.width() - tbw) / 2) - tbx;
  y = ((display.height() - tbh) / 2) - tby + display.height() / 4 - 12;
  display.setCursor(x, y);
  display.print(strstatic);
  strstatic = "USB Charger!";
  display.getTextBounds(strstatic, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  x = ((display.width() - tbw) / 2) - tbx;
  y = ((display.height() - tbh) / 2) - tby + display.height() / 4 + 12;
  display.setCursor(x, y);
  display.print(strstatic);  
}



// all library classes are placed in the namespace 'as'
using namespace as;


class ePaperType : public Alarm {

private:
  bool                 mUpdateDisplay;
  bool                 shMeasurementsDisplay;
  bool                 shEmptyBattDisplay;

public:
  ePaperType () :  Alarm(0), mUpdateDisplay(false), shEmptyBattDisplay(false) {}
  virtual ~ePaperType () {}

  bool showEmptyBattDisplay() {
    return shEmptyBattDisplay;
  }

  void showEmptyBattDisplay(bool s) {
    shEmptyBattDisplay = s;
  }

  bool mustUpdateDisplay() {
    return mUpdateDisplay;
  }

  void mustUpdateDisplay(bool m) {
    mUpdateDisplay = m;
  }

  void init() {
    /*
    u8g2Fonts.begin(display);
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(0);
    */
  }

 
  void setRefreshAlarm (uint32_t t) {
    //isWaiting(true);
    sysclock.cancel(*this);
    Alarm::set(millis2ticks(t));
    sysclock.add(*this);
  }
  
  virtual void trigger (__attribute__((unused)) AlarmClock& clock) {
    DPRINTLN("ePaper refresh triggered");
    if (this->mustUpdateDisplay()) {
      this->mustUpdateDisplay(false);
      if (this->showEmptyBattDisplay() == true ) {
          display.drawPaged(EmptyBattDisplay);
          this->showEmptyBattDisplay(false);
      } 
      else {
        display.drawPaged(MeasurementsDisplay);
      }
    }
  }
  
};
