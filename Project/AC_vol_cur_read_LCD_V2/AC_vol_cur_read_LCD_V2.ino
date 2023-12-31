#include "ZMPT101B.h"
#include "ACS712.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

ZMPT101B voltageSensor(35);
ACS712 currentSensor(ACS712_20A, 34);

int mVperAmp = 100;
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

float P=0; //power
float U=0; //voltage
float I=0;  //current
long dt=0;
float CulmPwh=0;
float units=0;
long changeScreen=0;
float lastSample=0;

unsigned long lasttime=0;
long ScreenSelect = 0;


void setup()
{
  Serial.begin(9600);

  delay(100);
  voltageSensor.setSensitivity(0.0025);
  voltageSensor.setZeroPoint(2333);
 
  currentSensor.setZeroPoint(3133);
  currentSensor.setSensitivity(0.15);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); }

  // Clear the buffer
  display.clearDisplay();
  display.display();

  //CalibCurrent();
  //CalibVoltage();

}


void loop()
{
 
  U = voltageSensor.getVoltageAC();
    if(U<55)
      {
        U=0;
        CulmPwh=0;
       }
 

  I = currentSensor.getCurrentAC();
  dt = micros()- lastSample;
  
    if(I<0.15)
    {
        I=0;
        CulmPwh=0;
      }
      
  P = U * I;
  
  CulmPwh = CulmPwh + P*(dt/3600);///uWh
  units= CulmPwh/1000;

  if(millis()-changeScreen>5000)
    {
      ScreenSelect+=1;
      changeScreen=millis();
    }
    
  if(millis()-lasttime>500)
    {
      if((ScreenSelect%4)==0)
      { displayVoltCurrent(); }//Volts and Current
  
      else if( (ScreenSelect%4)==1)
      { displayInstPower();   }//Instantaenous Power
  
      else if( (ScreenSelect%4)==2)
      { displayEnergy();      } //Energy
      
      else if( (ScreenSelect%4)==3)
      { displayUnits();       } //Units
    }
  lastSample=micros();
}


float getVPP()
{
  float result;
  int readValue;                // value read from the sensor
  int maxValue = 0;             // store max value here
  int minValue = 4096;          // store min value here ESP32 ADC resolution
  
   uint32_t start_time = millis();
   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
       readValue = analogRead(34);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the minimum sensor value*/
           minValue = readValue;
       }
   }
   
   // Subtract min from max
   result = ((maxValue - minValue) * 3.3)/4096.0; //ESP32 ADC resolution 4096
      
   return result;
 }
 
void displayVoltCurrent()
{
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  display.setTextSize(3);
  displayCenter(String(U)+"V",3);

  display.setTextSize(3);
  displayCenter(String(I)+"A",33);
  display.display();
  lasttime=millis();
  
}

void displayInstPower()
{
  display.clearDisplay();
  display.setTextColor(WHITE); 
  display.setTextSize(2);
  display.setCursor(0,0);
  displayCenter("Power",3); 
  display.setTextSize(3);
  
     if(P>1000)
      {
        displayCenter(String(P/1000)+"kW",30);
      }
    else
      {
        displayCenter(String(P)+"W",30);
      }

  display.display();
  lasttime=millis();
} 

void displayEnergy()
{
  display.clearDisplay();
  display.setTextColor(WHITE);
  
   if(CulmPwh>1000000000)
     {
        display.setTextSize(2);
        displayCenter("Energy kWh",3);
        display.setTextSize(3);      
        displayCenter(String(CulmPwh/1000000000),30);
     }
   else if(CulmPwh<1000000000 && CulmPwh>1000000)
     {
        display.setTextSize(2);
        displayCenter("Energy Wh",3);
        display.setTextSize(3);      
        displayCenter(String(CulmPwh/1000000),30);
     }
   else if(CulmPwh<1000000 && CulmPwh>1000)
     {
        display.setTextSize(2);
        displayCenter("Energy mWh",3);
        display.setTextSize(3);      
        displayCenter(String(CulmPwh/1000),30);
     }
   else
     {
        display.setTextSize(2);
        displayCenter("Energy uWh",3);
        display.setTextSize(3);      
        displayCenter(String(CulmPwh),30);
     }
  display.display();
  lasttime=millis();
 }

void displayUnits()
{
  display.clearDisplay();
  display.setTextColor(WHITE);
  
    if(units>1000000)
      {
        display.setTextSize(2);
        displayCenter("Units",3); 
        display.setTextSize(3);      
        displayCenter(String(units/1000000),30);
      }
    else if(units<1000000 && units>1000)
      {
        display.setTextSize(2);
        displayCenter("MilliUnits",3);
        display.setTextSize(3);      
        displayCenter(String(units/1000),30);
      }
  else
      {
        display.setTextSize(2);
        displayCenter("MicroUnits",3);
        display.setTextSize(3);      
        displayCenter(String(units),30); 
      }
  display.display();
  lasttime=millis();
 }
 
void CalibCurrent()
{ 
  while(1)
  {
    currentSensor.calibrate();  
    Serial.print("Zero Point Current :");
    Serial.println(currentSensor.getZeroPoint());
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("Current Zero Point :");
    display.setCursor(0,20);
    display.setTextSize(2);
    display.print(currentSensor.getZeroPoint());
    display.display();
    delay(500);
  }
  
}

void CalibVoltage()
{ 
  while(1)
  {
    voltageSensor.calibrate();  
    Serial.print("Zero Point Voltage :");
    Serial.println(voltageSensor.getZeroPoint());
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("Voltage Zero Point :");
    display.setCursor(0,20);
    display.setTextSize(2);
    display.print(voltageSensor.getZeroPoint());
    display.display();
    delay(500);
  }
  
}

void displayCenter(String text, int line) 
{
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
  // display on horizontal center
  display.setCursor((SCREEN_WIDTH - width) / 2, line);
  display.println(text); // text to display
  display.display();
}