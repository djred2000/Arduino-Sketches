/*
Original code created January 4th, 2014 by Claude Heintz http://lx.claudeheintzdesign.com/lxarduino_sketches.html

modified by: Jared Alexander
 
This code is in the public domain.
sACN E 1.31 is a public standard published by the PLASA technical standards program
http://tsp.plasa.org/tsp/documents/published_docs.php
 
Requires Arduino Ethernet Shield. Modifed to use the lastest version of EthernetUdp library included in Arduino 1.0.6. No need to install other libraries. 
Receives E1.31 data (sACN) and outputs to Disney Glow with the Show ears. GWTS library only works with Arduino Uno with IR LED connected to pin 2. 
The library can be modified to work with a Mega and use different pins. 
This sketch only receives one universe.  
*/

#include <Ethernet.h> //Standard library
#include <EthernetUdp.h> //Standard library
#include <SPI.h>         
#include <GwtS.h> //Library found here: https://github.com/Materdaddy/GwtS-Arduino

GWTS ears; //initialze ears 

// enter desired universe and subnet  (sACN first universe is 1)
#define DMX_SUBNET 0
#define DMX_UNIVERSE 1
#define SACN_PORT 5568
#define SACN_BUFFER_MAX 640
//set start address.  Ears use 6 channels. RGB left ear and RGB right ear. Byte 126 in the packet corresponds to channel 1 on your controller. 
#define GWTS_START_ADDRESS 126  

byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0x2A, 0xBC}; //MAC address of ethernet shield
IPAddress ip(192, 168, 1, 120);  //IP address of ethernet shield

// buffers for receiving and sending data
unsigned char packetBuffer[SACN_BUFFER_MAX]; //buffer to hold incoming packet,

EthernetUDP Udp; //initialize UDP instance
  
void setup() 
{
  Ethernet.begin(mac,ip); //start ethernet and set IP address
  Udp.begin(SACN_PORT); //open port
  Serial.begin(9600); //begin serial for debugging
  POST(); //Test Ears
  Serial.println("Setup Complete"); //print complete
} //end setup

void sacnDMXReceived(unsigned char* pbuff, int count) 
{
    if ( pbuff[113] == DMX_SUBNET ) 
  {
    if ( pbuff[114] == DMX_UNIVERSE ) 
    {  
      int addressOffset = 125;
      if ( pbuff[addressOffset] == 0 ) 
      {  //start code must be 0
      ears.set_colors(pbuff[GWTS_START_ADDRESS],pbuff[GWTS_START_ADDRESS + 1],pbuff[GWTS_START_ADDRESS + 2],pbuff[GWTS_START_ADDRESS + 3],pbuff[GWTS_START_ADDRESS + 4],pbuff[GWTS_START_ADDRESS + 5]); 
      }
      }
    }
    
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
            Serial.println("E1.31 Packet Received"); //print when a valid packet is received 
            sacnDMXReceived(packetBuffer, count); //process data function    
    }
  }
} //End loop

void POST()
{
ears.set_colors(255,0,0,255,0,0);   //set ears to red
delay(1000);
ears.set_colors(0,255,0,0,255,0);   //set ears to green
delay(1000);
ears.set_colors(0,0,255,0,0,255);   //set ears to blue
delay(1000);
ears.set_colors(0,0,0,0,0,0);   //set ears to off
}
