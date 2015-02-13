/*
Original code created January 4th, 2014 by Claude Heintz http://lx.claudeheintzdesign.com/lxarduino_sketches.html

modified by: Jared Alexander
 
This code is in the public domain.
sACN E 1.31 is a public standard published by the PLASA technical standards program
http://tsp.plasa.org/tsp/documents/published_docs.php
 
Requires Arduino Ethernet Shield. Modifed to use the lastest version of EthernetUdp library included in Arduino 1.0.6. No need to install other libraries. 
Receives E1.31 data (SACN) and ArtNet data and outputs to Disney Glow with the Show ears and FastLED library for pixel control. GWTS library only works with Arduino Uno with IR LED connected to pin 2. 
The library can be modified to work with a Mega and use different pins. 
This sketch can receive multiple E1.31 universes but only one ArtNet subnet.  You can only send the Arduino one protocol (E1.31 or ArtNet) at a time. 
This sketch also includes a power on self test function to make sure everything is hooked up properly. 
*/

#include <FastLED.h> //install latest version from here: https://github.com/FastLED/FastLED
#include <GwtS.h> //Library found here: https://github.com/Materdaddy/GwtS-Arduino
#include <SPI.h>    //Standard library     
#include <Ethernet.h> //Standard library
#include <EthernetUdp.h> //Standard library

/*  set the desired subnet and universe (first universe is 0)
    sACN starts with universe 1 so subtract 1 from sACN universe
    to get DMX_UNIVERSE.                                           */
#define ARTNET_SUBNET 0 //defualt subnet is 0. Should not need to be changed. 
#define ARTNET_UNIVERSE 0 //first universe being used
#define E131_SUBNET 0 //defualt subnet is 0. Should not need to be changed. 
#define E131_START_UNIVERSE 1 //first universe being used
#define PIXELS_PER_UNIVERSE 170 //total number of pixels being user per universe.  Max is 170. 170 x 3 = 510 channels. 
#define UNIVERSE_COUNT 2 //total number of universes being received for E1.31
#define ETHERNET_BUFFER_MAX 640
#define ARTNET_ARTDMX 0x5000
#define ARTNET_ARTPOLL 0x2000
#define ARTNET_PORT 0x1936 //standard port
#define ARTNET_ADDRESS_OFFSET 17 //Byte 18 in the packet contains channel 1 values.  Offset is set to one prior. 
#define E131_PORT 5568 //standard port 
#define E131_ADDRESS_OFFSET 125 //Byte 126 in the packet contains channel 1 values. Offset is set to one prior. 
#define GWTS_START_ADDRESS_E131 576 //126 plus actual start channel
#define GWTS_START_ADDRESS_ARTNET 467 //126 plus actual start channel

GWTS ears; //initialize GWTS Ears

//Pixel Setup
#define NUM_LEDS 340 //total number of pixels
#define DATA_PIN 30 //pin pixels are connected to 
CRGB leds[NUM_LEDS]; //pixel array
int currentE131Universe; //stores current universe being received.
int channel; //channel for pixels
int channelwidth = 3; //each pixels needs three channels. 
int startPixel; //start pixel for each loop


//network addresses
byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0x2A, 0xBC};
IPAddress ip(192, 168, 1, 120);

// buffer
unsigned char packetBuffer[ETHERNET_BUFFER_MAX];

// An EthernetUDP instance to let us send and receive packets over UDP
// aUDP creates a socket for an Art-Net port
// suUDP creates a socket for sACN that is unicast to the ip address
EthernetUDP aUDP;
EthernetUDP suUDP;

void setup() 
{
  Ethernet.begin(mac,ip);
  aUDP.begin(ARTNET_PORT);
  suUDP.begin(E131_PORT);
  Serial.begin(9600);
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS); //initialize pixels 
  POST(); //run power on self test function
  Serial.println("Setup Complete"); //print complete 
}

/* artDMXReceived checks the universe and subnet then
   outputs the data to FastLED and GWTS */

void artDMXReceived(unsigned char* pbuff) 
{
  if ( (pbuff[14] & 0xF) == ARTNET_UNIVERSE ) 
  {
    if ( (pbuff[14] >> 8) == ARTNET_SUBNET ) 
    {
      channel = 1; //reset channel assignment to 1 each time through loop.
      for(int i = 0; i < PIXELS_PER_UNIVERSE; i++) //loop for LED pixels
      {
      leds[i] = CRGB(pbuff[ARTNET_ADDRESS_OFFSET + channel], pbuff[ARTNET_ADDRESS_OFFSET + (channel +1)], pbuff[ARTNET_ADDRESS_OFFSET + (channel +2)]); //assign channel values to pixels 
      channel +=channelwidth; //increase last channel number by channel width
      }
    }
  }
  ears.set_colors(pbuff[GWTS_START_ADDRESS_ARTNET],pbuff[GWTS_START_ADDRESS_ARTNET + 1],pbuff[GWTS_START_ADDRESS_ARTNET + 2],pbuff[GWTS_START_ADDRESS_ARTNET + 3],pbuff[GWTS_START_ADDRESS_ARTNET + 4],pbuff[GWTS_START_ADDRESS_ARTNET + 5]);
  FastLED.show();
}

/*  artNetOpCode checks to see that the packet is actually Art-Net
    and returns the opcode telling what kind of Art-Net message it is.  */

int artNetOpCode(unsigned char* pbuff) 
{
  String test = String((char*)pbuff);
  if ( test.equals("Art-Net") ) 
  {
    if ( pbuff[11] >= 14 ) 
    {            //protocol version [10] hi byte [11] lo byte
      return pbuff[9] *256 + pbuff[8];  //opcode lo byte first
    }
  }
  
  return 0;
}

/* sacnDMXReceived checks the universe and subnet then
   outputs the data to FastLED and GWTS  */

void sacnDMXReceived(unsigned char* pbuff, int count) 
{
    if ( pbuff[113] == E131_SUBNET ) 
  {  
      int addressOffset = 125; //first 125 bytes of packet are header information
      if ( pbuff[addressOffset] == 0 ) //start code must be 0
      {  
      currentE131Universe = pbuff[114];  //Byte 114 is the universe number in the packet
      if ( currentE131Universe >= E131_START_UNIVERSE && currentE131Universe <= E131_START_UNIVERSE + UNIVERSE_COUNT ) //is the universe received equal to the start universe or within the universe count range 
      {
      startPixel = (currentE131Universe - E131_START_UNIVERSE) * PIXELS_PER_UNIVERSE; //set start channel based on universe received. Universe 1 would be 0. Universe 2 would be 170.
      channel = 1; //reset channel assignment to 1 each time through loop.
      for(int i = startPixel; i < startPixel + PIXELS_PER_UNIVERSE; i++) //loop for LED pixels
      {
      leds[i] = CRGB(pbuff[E131_ADDRESS_OFFSET + channel], pbuff[E131_ADDRESS_OFFSET + (channel +1)], pbuff[E131_ADDRESS_OFFSET + (channel +2)]); //assign channel values to pixels 
      channel +=channelwidth; //increase last channel number by channel width
      }
      }
      }
      
    }
    ears.set_colors(pbuff[GWTS_START_ADDRESS_E131],pbuff[GWTS_START_ADDRESS_E131 + 1],pbuff[GWTS_START_ADDRESS_E131 + 2],pbuff[GWTS_START_ADDRESS_E131 + 3],pbuff[GWTS_START_ADDRESS_E131 + 4],pbuff[GWTS_START_ADDRESS_E131 + 5]);
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

/************************************************************************

  The main loop checks for and reads packets from the two UDP ethernet
  socket connections.  When a packet is recieved, it is checked to see if
  it is valid and then one of the DMXReceived functions is called, sending
  the DMX values to the output.

*************************************************************************/

void loop() 
{
  // first check to see if a packet is available on the Art-Net port
  int packetSize = aUDP.parsePacket();
  if( packetSize )
  {
    aUDP.read(packetBuffer, ETHERNET_BUFFER_MAX);
    /* after reading the packet into the buffer, check to make sure
       that it is an Art-Net packet and retrieve the opcode that
       tells what kind of message it is                         */
    int opcode = artNetOpCode(packetBuffer);
    if ( opcode == ARTNET_ARTDMX ) 
    {
      Serial.println("ArtNet Packet Received");
      artDMXReceived(packetBuffer);
    } 
  } 
    else 
    {
      /* then, if still no packet, check to see if a packet
         is available on the sACN unicastcast port         */
       packetSize = suUDP.parsePacket();
    if( packetSize ) 
    {
      suUDP.read(packetBuffer, ETHERNET_BUFFER_MAX);
      /* after reading the packet into the buffer, check to make sure
       that it is a valid sACN packet.*/
      int count = checkACNHeaders(packetBuffer, packetSize);
      if ( count ) 
      {
        Serial.println("E131 Packet Received");
        sacnDMXReceived(packetBuffer, count);
      }
    }
      
    }
  
} //End loop

//Power on self test function to make sure everything is connected properly. 
void POST()
{
ears.set_colors(255,0,0,255,0,0);   //set ears to red
LEDS.showColor(CRGB(255, 0, 0)); //turn all pixels on red
delay(1000);
ears.set_colors(0,255,0,0,255,0);   //set ears to green
LEDS.showColor(CRGB(0, 255, 0)); //turn all pixels on green
delay(1000);
ears.set_colors(0,0,255,0,0,255);   //set ears to blue
LEDS.showColor(CRGB(0, 0, 255)); //turn all pixels on blue
delay(1000);
ears.set_colors(0,0,0,0,0,0);   //set ears to off
FastLED.clear(); //clear pixels
FastLED.show(); //update pixels 
Serial.println("POST Complete");
}

