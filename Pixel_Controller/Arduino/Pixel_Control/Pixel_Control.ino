/*
 * This code will run on any Arduino but I recommend a Teensy 3.2 for it's better pixel driving capability.
 * 
 * 
 * 
 */
#include <Time.h>
#include <TimeLib.h>
#include <Wire.h>
#include <DS1307RTC.h>
#define USE_OCTOWS2811 //Using OctoWS2811 output on Teensy 3.2 for past writing of WS2811 pixel data. 
#include<OctoWS2811.h>
#include <FastLED.h>

//Forward function declarations 
void TimeProcess();
void POST();
void Standby();

//General Definitions
const int ledpin = 13; //LED on board for monitoring
const int BUFFER_SIZE = 638; //size of data buffer to hold E1.31 and ArtNet data
const int E131_ADDRESS_OFFSET = 125; //byte 126 of the E1.31 packet contains channel 1 value
const int ARTNET_ADDRESS_OFFSET = 17; //byte 18 of the ArtNet packet contains channel 1 value
const int E131_START_CODE = 0; 
const int NTP_PACKET_SIZE = 48; //NTP time packet size is only 4 bytes
boolean SetTime = false;
boolean TimeRequestSent = false; 
const int E131StartUniverse = 1; //First E1.31 universe
const int E131UniverseCount = 4; //Total number of E1.31 universes being received
int E131CurrentUniverse;  //Current universe we are processing 
int StartLED = 0;
const int NumLEDsPerUniverse = 170;
int EndLED;
int startms;
int endms;

//Timer Setup
unsigned int currentcounter = 0; //counter for data reception
unsigned int previouscounter = 0; //counter for data reception 
unsigned long currentDelay = 0; //current time value for ArtNet and E1.31 reception
tmElements_t tm; //time variable
time_t previousMinute; //time variable
time_t ActualTime; //time variable 

// buffers for receiving data
unsigned char DataBuffer[BUFFER_SIZE]; //buffer to hold incoming E1.31 and ArtNet Data
unsigned char Header[3]; //header buffer


//FastLED Parallel Output using OctoWS2811 on Teensy. 8 parallel outputs.  
//Ports: 2,14,7,8,6,20,21,5
const int NUM_LEDS_PER_STRIP = 100;
const int NUM_STRIPS = 8;
CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP];
const int NUM_LEDS = NUM_STRIPS * NUM_LEDS_PER_STRIP;

/*
//Standard FastLED Output. Uncomment if you don't want to use OCTOWS2811. 
const int NUM_LEDS = 571;
CRGB leds[NUM_LEDS];
*/

const int ChannelWidth = 3; //each pixel needs 3 channels for control
int Channel = 1; //current channel being used 
byte current_hue; //hue value for rainbow effect
int ledNumber; //current led Pixel being used for rainbow effect
int currentProgram = 1; //initial pixel pattern to run
byte random_delay; //variable for random delay 
byte random_color; //variable for random color
byte random_color2; //variable for another random color
byte brightness; //brightness variable 
byte thishue; //for fill rainbow
byte deltahue = 1; //for fill rainbow

void setup() 
{
pinMode(ledpin, OUTPUT);  //pin 13 to let us know if we are receiving any data
digitalWrite(ledpin, LOW); //turn LED off
Serial.begin(250000); //USB serial port for monitoring/debugging
Serial1.begin(2000000); //ESP8266 serial connection

/*
//Standard FastLED Ouput. Multiple ouputs in one array. This is not parallel output! 
FastLED.addLeds<WS2811, 2, RGB>(leds, 0, 100); //pixels 0-100
FastLED.addLeds<WS2811, 3, RGB>(leds, 100, 100); //pixels 101-200
FastLED.addLeds<WS2811, 4, RGB>(leds, 200, 100); //pixels 201-300
FastLED.addLeds<WS2811, 5, RGB>(leds, 300, 100); //pixels 301-400
FastLED.addLeds<WS2811, 6, RGB>(leds, 400, 100); //pixels 401-500
FastLED.addLeds<WS2811, 7, RGB>(leds, 500, 71); //pixels 501-571
*/

//FastLED OctoWS2811 Output. Parallel output. 
LEDS.addLeds<OCTOWS2811, RGB>(leds, NUM_LEDS_PER_STRIP); //OctoWS2811 output on Teensy 3.2 

LEDS.setCorrection(TypicalLEDStrip); //color correction 
LEDS.setBrightness(255); //full brightness 
POST(); //Run power on self test function each time board boots 
Serial.println("Setup Completed");
}

void loop() 
{
//Set NTP time once each time board boots 
while(SetTime == false) //keep trying to set the time until SetTime becomes true 
{
  if(TimeRequestSent == false)
  {
  Serial1.write('Z'); //Write "Z" to ESP8266.
  Serial.println("Time Request Sent");
  TimeRequestSent = true; //only send the request once per loop 
  }
  Serial1.readBytes(Header,3); //read 3 bytes at a time 
  if(Header[0] == 'N' && Header[1] == 'T' && Header[2] == 'P') //check for NTP Time Header 
{
  Serial1.readBytes(DataBuffer,BUFFER_SIZE); //read NTP data into buffer 
  Serial.println("NTP Time Data Received");
  TimeProcess(); //pass the data to be processed 
}
}

//Reads serial data and determines what kind of data it is then processes it 
if (Serial1.available()) 
{
Serial1.readBytes(Header,3); //read 3 bytes at a time 
if(Header[0] == 'S' && Header[1] == 'A' && Header[2] == 'C') //check for E1.31 Header
{
  Serial1.readBytes(DataBuffer,BUFFER_SIZE); //read data into buffer 
  digitalWrite(ledpin, HIGH); //turn LED pin on for debugging 
  currentcounter++;
  E131Received();
}
else if(Header[0] == 'A' && Header[1] == 'R' && Header[2] == 'T') //check for ArtNet Header
{
  Serial1.readBytes(DataBuffer,BUFFER_SIZE);
  digitalWrite(ledpin, HIGH);
  currentcounter++;
  artDMXReceived();
}
}

 //Timer runs standby pixel functions when no E1.31 or ArtNet data is being received. 
if(currentcounter != previouscounter) //has the value changed?
  {
    currentDelay = millis(); //store the time since the value has increased 
    previouscounter = currentcounter; //set the previous value equal to the current value
  }
  
  if(millis() - currentDelay > 30000) //current time minus time since the counter has changed. After 30 seconds of no data being received 
  { 
   digitalWrite(13, LOW); //turn LED off. Not receiving E1.31 or ArtNet. 
   Standby(); //run pixel standby function 
  }
} //end loop



