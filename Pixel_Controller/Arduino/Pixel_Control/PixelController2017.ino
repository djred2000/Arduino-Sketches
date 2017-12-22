#include <Time.h>
#include <TimeLib.h>
#include <Wire.h>
#include <DS1307RTC.h>
#define USE_OCTOWS2811
#include<OctoWS2811.h>
#include <FastLED.h>

//Adjustable Values
const int E131StartUniverse = 1; //First E1.31 universe to process
const int E131UniverseCount = 5; //Total number of E1.31 universes being processed
const int NumLEDsPerUniverse = 170; //Number of LEDs per universe. No more than 170 per universe (3 channels for LED)
const int NUM_LEDS_PER_STRIP = 100; //Number of LEDs per ouput pin
const int NUM_STRIPS = 8; //number of output pins. Recommend keeping at 8
const int NUM_LEDS = 850; 
const int GlobalBrightness = 255; //Global LED brightness set at startup
const int StandbyDelay = 30000; //Delay in milliseconds before Standby function begins when no E1.31 is received 
const int ChangeTime = 600; //Time in seconds to change to the next standby function 

//General Definitions
const int ledpin = 13; //LED on board for monitoring
const int BUFFER_SIZE = 638; //size of data buffer to hold E1.31 
const int E131_ADDRESS_OFFSET = 125; //byte 126 of the E1.31 packet contains channel 1 value
const int E131_START_CODE = 0; 
boolean SetTime = false;
boolean TimeRequestSent = false; 
int E131CurrentUniverse;  //Current universe we are processing 
int StartLED = 0;
int EndLED;
int startms;
int endms;

//Timer Setup
unsigned int currentcounter = 0; //counter for data reception
unsigned int previouscounter = 0; //counter for data reception 
unsigned long currentDelay = 0; //current time value for ArtNet and E1.31 reception
tmElements_t tm; //time variable
time_t previousTime = 0; //time variable
time_t ActualTime; //time variable 

// buffers for receiving data
unsigned char DataBuffer[BUFFER_SIZE]; //buffer to hold incoming E1.31 and ArtNet Data
unsigned char Header[3]; //header buffer

//FastLED Parallel Output using OctoWS2811 on Teensy 
//Ports: 2,14,7,8,6,20,21,5
CRGB leds[NUM_LEDS];

const int ChannelWidth = 3; //each pixel needs 3 channels for control
int Channel = 1; //current channel being used 
byte current_hue; //hue value for rainbow effect
int ledNumber; //current led Pixel being used for rainbow effect
int currentProgram = 1; //initial pixel pattern to run
uint8_t random_delay; //variable for random delay 
uint8_t random_color; //variable for random color
uint8_t random_color2; //variable for another random color
uint8_t brightness; //brightness variable 
uint8_t thishue; //for fill rainbow
uint8_t deltahue = 1; //for fill rainbow
uint8_t color1 = 0;

void setup() 
{
pinMode(ledpin, OUTPUT);  //pin 13 to let us know if we are receiving any data
digitalWrite(ledpin, LOW); //turn LED off
Serial.begin(250000); //USB serial port for monitoring/debugging
Serial1.begin(2000000); //ESP8266 serial connection
LEDS.addLeds<OCTOWS2811, RGB>(leds, NUM_LEDS_PER_STRIP); //OctoWS2811 output on Teensy 3.2 
LEDS.setCorrection(TypicalLEDStrip); //color correction 
LEDS.setBrightness(GlobalBrightness); //Overall Brightness 
POST(); //Run power on self test function each time board boots 
InitializeRTC(); //Check RTC and set time 
//Serial.println("Setup Completed");
}

void loop() 
{
//Reads serial data and determines what kind of data it is then processes it 
if (Serial1.available()) 
{
Serial1.readBytes(Header,3); //read 3 bytes at a time 
if(Header[0] == 'S' && Header[1] == 'A' && Header[2] == 'C') //check for E1.31 Header
{
  Serial1.readBytes(DataBuffer,BUFFER_SIZE); //read data into buffer 
  digitalWrite(ledpin, HIGH); //turn LED on
  currentcounter++;
  E131Received();
}

}

 //Timer runs standby pixel functions when no E1.31 is being received. 
if(currentcounter != previouscounter) //has the value changed?
  {
    currentDelay = millis(); //store the time since the value has increased 
    previouscounter = currentcounter; //set the previous value equal to the current value
  }
  
  if(millis() - currentDelay > StandbyDelay) //current time minus time since the counter has changed. After 30 seconds of no data being received 
  { 
   digitalWrite(13, LOW); //turn LED off. Not receiving E1.31 
   Standby(); //run pixel standby function 
  }
} //end loop



