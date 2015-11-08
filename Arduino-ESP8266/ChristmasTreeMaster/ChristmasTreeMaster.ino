#include <GwtS.h>
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <FastLED.h>

GWTS ears; //GWTS Instance

//#define FASTLED_ALLOW_INTERRUPTS 0
#define ledpin 13 //LED on board for monitoring
#define BUFFER_SIZE 524 //size of data buffer to hold E1.31 and ArtNet data
#define E131_ADDRESS_OFFSET 11 //byte 12 of the E1.31 packet contains channel 1 value
#define ARTNET_ADDRESS_OFFSET 11 //byte 12 of the ArtNet packet contains channel 1 value
#define E131_GWTS_START 105 //GWTS ears start on byte 105 and go for 6 bytes. RGB each ear.
#define ARTNET_GWTS_START 12 //GWTS ears start on byte 12 and go for 6 bytes. RGB each ear.
#define E131_START_CODE 0 //
#define NTP_PACKET_SIZE 4 //NTP time packet size is only 4 bytes
boolean getTime = true;
 
//Data Reception Timer Setup
unsigned int currentcounter = 0; //counter for data reception
unsigned int previouscounter = 0; //counter for data reception 
unsigned long currentDelay = 0; //current time value for ArtNet and E1.31 reception
tmElements_t tm; //time variable
time_t previousMinute; //time variable
time_t ActualTime; //time variable 

// buffers for receiving data
unsigned char DataBuffer[BUFFER_SIZE]; //buffer to hold incoming E1.31 and ArtNet Data
unsigned char NTPBuffer[NTP_PACKET_SIZE]; //buffer to hold NTP data
unsigned char PacketStart[3]; //buffer to store packet header

//FastLED Definitions
#define PIXELTYPE WS2811
#define NUM_STRIPS 3
#define NUM_LEDS_PER_STRIP 100
#define NUM_LEDS 371
#define DATA_PIN1 9 
#define DATA_PIN2 10
#define DATA_PIN3 11
#define DATA_PIN4 12
static int ChannelWidth = 3; //each pixel needs 3 channels for control
int Channel = 1; 
CRGB leds[NUM_LEDS]; //main pixel array
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
Serial.begin(250000); //USB serial port for monitoring/debuggins
Serial1.begin(500000); //ESP8266 serial connection
FastLED.addLeds<PIXELTYPE, DATA_PIN1, RGB>(leds, 0, NUM_LEDS_PER_STRIP); //pixel strip 1
FastLED.addLeds<PIXELTYPE, DATA_PIN2, RGB>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP); //pixel strip 2
FastLED.addLeds<PIXELTYPE, DATA_PIN3, RGB>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP); //pixel strip 3
FastLED.addLeds<PIXELTYPE, DATA_PIN4, RGB>(leds, 3 * NUM_LEDS_PER_STRIP, 71); //pixel strip 4
POST(); //Run power on self test function each time board boots 
Serial.println("Setup Completed");
}

void loop() 
{
  //Set NTP time once each time board boots 
if(getTime == true)
{
  Serial.println("Time Request Sent");
  Serial1.write('Z'); //Write "Z" to ESP8266.
  getTime = false; //we have not yet received the time
}

//Reads serial data and determines what kind of data it is then processes it 
if (Serial1.available()) 
{ 
  currentcounter++;  //increase counter by 1 each time through
 Serial1.readBytes(PacketStart,3); //Read 3 bytes and store them. Each packet has a 3 byte header. 

//Data is E1.31   
if(PacketStart[0] == 'S' && PacketStart[1] == 'A' && PacketStart[2] == 'C') //Check packet header
{
 //Serial.println("E1.31 Packet Received");
 digitalWrite(ledpin, HIGH); //turn on pin 13
 Serial1.readBytes(DataBuffer,BUFFER_SIZE); //read data into buffer  
 E131Received(DataBuffer,BUFFER_SIZE); //Pass data to be processed
}

//Data is ArtNet
if(PacketStart[0] == 'A' && PacketStart[1] == 'R' && PacketStart[2] == 'T') //Check packet header
{
 //Serial.println("ArtNet Packet Received");
 digitalWrite(ledpin, HIGH); //turn on pin 13
 Serial1.readBytes(DataBuffer,BUFFER_SIZE); //read data into buffer
 artDMXReceived(DataBuffer); //pass data to be processed
}

//Data is NTP Time. Sets time in Real Time Clock
if(PacketStart[0] == 'N' && PacketStart[1] == 'T' && PacketStart[2] == 'P') //Check packet header
{
Serial.println("Time Data Received");
Serial1.readBytes(NTPBuffer,NTP_PACKET_SIZE); //read data into buffer and output size 
TimeProcess(NTPBuffer); //pass data to be processed
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
}

