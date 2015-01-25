/*
Original code created January 4th, 2014 by Claude Heintz http://lx.claudeheintzdesign.com/lxarduino_sketches.html

modified by: Jared Alexander
 
This code is in the public domain.
sACN E 1.31 is a public standard published by the PLASA technical standards program
http://tsp.plasa.org/tsp/documents/published_docs.php
 
Requires Arduino Ethernet Shield. Modifed to use the lastest version of EthernetUdp library included in Arduino 1.0.6. No need to install other libraries. 
Receives E1.31 data (sACN) and outputs to FastLED library. 
This sketch receives multiple universes.  It also runs a POST function each time the board boots to make sure pixels are working properly. 
*/


#include <FastLED.h> //install latest version from here: https://github.com/FastLED/FastLED
#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h> //standrd ethernet library
#include <EthernetUdp.h>  //standard ethernet library

                            
// enter desired universe and subnet  (sACN first universe is 1)
#define DMX_SUBNET 0 //defualt subnet is 0. Should not need to be changed. 
#define DMX_START_UNIVERSE 1 //first universe being used
#define PIXELS_PER_UNIVERSE 170 //total number of pixels being user per universe.  Max is 170. 170 x 3 = 510 channels. 
#define UNIVERSE_COUNT 2 //total number of universes being received 
#define SACN_PORT 5568 //standard port 
#define SACN_BUFFER_MAX 640 //packet buffer size for E1.31
#define SACN_ADDRESS_OFFSET 125 //First 125 bytes of E1.31 packet are header information and do not contain channel values. 
int b; //stores current universe being received.
int channel; //channel for pixels
int channelwidth = 3; //each pixels needs three channels. 
int startPixel; //start pixel for each loop


//the initial ip and MAC address will get changed by beginMulti
//the multicast ip address should correspond to the desired universe/subnet

byte mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //MAC address of ethernet shield
IPAddress ip(192, 168, 1, 120);  //IP address of ethernet shield


// buffers for receiving and sending data
unsigned char packetBuffer[SACN_BUFFER_MAX]; //buffer to hold incoming packet,


// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

//Pixel Setup
#define NUM_LEDS 340 //total number of pixels
#define DATA_PIN 30 //pin pixels are connected to 
CRGB leds[NUM_LEDS]; //pixel array

//setup initializes Ethernet, opens the UDP port and initializes pixels   
void setup() 
{
  Ethernet.begin(mac,ip); //initialize ethernet shield and set MAC and IP addresses
  Udp.begin(SACN_PORT); //open E1.31 port
  Serial.begin(9600); //start serial for debugging 
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS); //initialize pixels 
  POST(); //run power on self test function
  Serial.print("Setup Complete"); //print complete 
} //end setup

void sacnDMXReceived(unsigned char* pbuff, int count) 
{
    if ( pbuff[113] == DMX_SUBNET ) 
  {  
      int addressOffset = 125; //first 125 bytes of packet are header information
      if ( pbuff[addressOffset] == 0 ) //start code must be 0
      {  
      b = pbuff[114];  //Byte 114 is the universe number in the packet
      //Serial.print("Universe: ");
      //Serial.println(pbuff[114]); //print universe number packet contains
      if ( b >= DMX_START_UNIVERSE && b <= DMX_START_UNIVERSE + UNIVERSE_COUNT ) //is the universe received equal to the start universe or within the universe count range 
      {
      startPixel = (b - DMX_START_UNIVERSE) * PIXELS_PER_UNIVERSE; //set start channel based on universe received. Universe 1 would be 0. Universe 2 would be 170.
      channel = 1; //reset channel assignment to 1 each time through loop.
      for(int i = startPixel; i < startPixel + PIXELS_PER_UNIVERSE; i++) //loop for LED pixels
      {
      leds[i] = CRGB(pbuff[SACN_ADDRESS_OFFSET + channel], pbuff[SACN_ADDRESS_OFFSET + (channel +1)], pbuff[SACN_ADDRESS_OFFSET + (channel +2)]); //assign channel values to pixels 
      channel +=channelwidth; //increase last channel number by channel width
      }
      }
      }
      
    }
    FastLED.show();
  }
  
//checks to see if packet is E1.31 data
int checkACNHeaders(unsigned char* messagein, int messagelength) 
{
  if ( messagein[1] == 0x10 && messagein[4] == 0x41 && messagein[12] == 0x37) 
  {	
      int addresscount = messagein[123] * 256 + messagein[124]; // number of values plus start code
      return addresscount -1; //Return how many values are in the packet.
    }
  return 0;
}


void loop() 
{
  int packetSize = Udp.parsePacket(); //store UPD packet
  
 
  if(packetSize)
  {
    Udp.read(packetBuffer,SACN_BUFFER_MAX); //read UDP packet
    
    int count = checkACNHeaders(packetBuffer, packetSize);
    if (count) 
    {
      Serial.println("E1.31 Packet Received");
      sacnDMXReceived(packetBuffer, count); //process data function
    }
  }
  
} //end loop

//Power on self test for pixels
void POST() 
{
   LEDS.showColor(CRGB(255, 0, 0)); //turn all pixels on red
   delay(1000);
   LEDS.showColor(CRGB(0, 255, 0)); //turn all pixels on green
   delay(1000);
   LEDS.showColor(CRGB(0, 0, 255)); //turn all pixels on blue
   delay(1000);
   FastLED.clear(); //clear pixel data
   FastLED.show(); //update pixels 
   Serial.println("POST Completed");
}

