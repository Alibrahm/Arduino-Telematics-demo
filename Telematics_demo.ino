/*IBRAHIM ALI
A PROTOTYPE DEMO OF A TELEMATIC DEVICE WITH DATALOGGING, WARNINGS AND DETECTION*/
#include "U8glib.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "DS3231.h"
#include "RTClib.h"
#define LEDG 6
#define LEDB 5
#define vibration 7
RTC_DS3231 rtc;
int x;
int y;
int Year;
int Month;
int Date;
int Hour;
int Minute;
int Second;
int tempC;
int interval;
int Minute_last;

File myFile;

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0); // SW SPI Com: SCK = 13, MOSI = 11, CS = 10, A0 = 9, Res = 12
                                                // For OLED display with SH1106 driver. If you use another display,
                                                // then please check the u8glib documentation and website at
                                                // https://github.com/olikraus/u8glib

int xmax=128;                                   // max length x-axis
int ymax=62;                                    // max length y-axis
int xcenter=xmax/2;                             // center of x-axis
int ycenter=ymax/2+10;                          // center of y-axis
int arc=ymax/2;                             
int angle=0;                                   
char* label[] = {"km/h","COOLANT","INTAKE", "VOLT"};    // some custom gauge labels
int labelXpos[] = {53, 45, 49, 53};                     // predefined x-position of a gauge label
#define potmeterPin A1                                  // simulate analogue value with potentiometer
int p, w, m,a=10;
u8g_uint_t xx=0;

// ------------------------------------------------- void gauge() ------------------------------------------
void gauge(uint8_t angle) {

  // draw border of the gauge
  u8g.drawCircle(xcenter,ycenter,arc+6, U8G_DRAW_UPPER_RIGHT);
  u8g.drawCircle(xcenter,ycenter,arc+4, U8G_DRAW_UPPER_RIGHT);
  u8g.drawCircle(xcenter,ycenter,arc+6, U8G_DRAW_UPPER_LEFT);
  u8g.drawCircle(xcenter,ycenter,arc+4, U8G_DRAW_UPPER_LEFT);

  // draw the needle
  float x1=sin(2*angle*2*3.14/360);              // needle position
  float y1=cos(2*angle*2*3.14/360); 
  u8g.drawLine(xcenter, ycenter, xcenter+arc*x1, ycenter-arc*y1);
  u8g.drawDisc(xcenter, ycenter, 5, U8G_DRAW_UPPER_LEFT);
  u8g.drawDisc(xcenter, ycenter, 5, U8G_DRAW_UPPER_RIGHT);
  u8g.setFont(u8g_font_chikita);
 
  // show scale labels
  u8g.drawStr( 20, 42, "0");                   
  u8g.drawStr( 25, 18, "25");
  u8g.drawStr( 60, 14, "50");
  u8g.drawStr( 95, 18, "75");
  u8g.drawStr( 105, 42, "100");
 
  // show gauge label
  u8g.setPrintPos(labelXpos[0],32);           
  u8g.print(label[0]);
 
  // show digital value and align its position
  u8g.setFont(u8g_font_profont22);             
  u8g.setPrintPos(54,60);
  if (w<10){                                    // leading 0 when value less than 10
    u8g.print("0");
  }
  if (w>99) {                                   // position at 100%
    u8g.setPrintPos(47,60);
  }
  u8g.print(w);
}

// ------------------------------------------------- void setup() ------------------------------------------
void setup(void) {
 Serial.begin(9600);
  u8g.setFont(u8g_font_chikita);
  u8g.setColorIndex(1);                         // Instructs the display to draw with a pixel on.

  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);                     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);                       // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);                       // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }

   Serial.print("Initializing SD card...");
  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
  }
  else
  {
    Serial.println("initialization done.");
  }
}

// ------------------------------------------------- void loop() ------------------------------------------

void loop(void) {

 
   p = analogRead(A1);               
 
 w = map(p,0,1023,0,100);                      // map it between 0 and 100
 m = map(p,0,1023,0,90);                       // map needle movement
interval = 1; //interval to write data
  DateTime now = rtc.now();
  Year = now.year();
  Month = now.month();
  Date = now.day();
  Hour = now.hour();
  Minute = now.minute();
  Second = now.second();
  tempC = rtc.getTemperature();
  
  if ((Minute % interval == 0)&(Minute_last!=Minute))
  {
    write_data(tempC);  //write data
    Minute_last = Minute;
  }
   
    x= 800; //Assume the speed limit is this
    if(p > x){
    digitalWrite(LEDB,HIGH);
    digitalWrite(8,HIGH);
    tingika(600);
    delay(200);
    tingika(600);
    }
else{
  digitalWrite(LEDB,LOW);
  digitalWrite(LEDG,LOW);
  digitalWrite(7,LOW);
  }
  // show needle and dial
  xx = m;                                      // 135 = zero position, 180 = just before middle, 0 = middle, 45 = max
  if (xx<45){                                   // position correct
    xx=xx+135;
  }
  else {
    xx=xx-45;
  }
 
  // picture loop
  {
    u8g.firstPage(); 
    do {             
      gauge(xx);
    }
    while( u8g.nextPage() );
  }
} 

void write_data(int temperature_input) //purposely used this function due to time limitation
{
  myFile = SD.open("speedata.txt", FILE_WRITE);
   int Speed = analogRead(potmeterPin);
  // if the file opened okay, write to it:
  if (myFile) {
    myFile.print(Year);
    myFile.print("/");
    myFile.print(Month);
    myFile.print("/");
    myFile.print(Date);
    myFile.print(" ");
    myFile.print(Hour);
    myFile.print(":");
    myFile.print(Minute);
    myFile.print("_SpeedData_");
    //myFile.println(temperature_input);
    myFile.println(Speed);
    myFile.close();
    Serial.println("Write file successful!"); //print out COM Port
  } else {
    Serial.println("error opening test.txt");
  }
}
void tingika(unsigned char delayms)
 {
  digitalWrite(vibration,HIGH); 
 delay(delayms); 
 digitalWrite(vibration,LOW); 
 delay(delayms); 
 } 
