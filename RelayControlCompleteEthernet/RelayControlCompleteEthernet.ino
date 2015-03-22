/*
Original code created January 4th, 2014 by Claude Heintz http://lx.claudeheintzdesign.com/lxarduino_sketches.html

modified by: Jared Alexander
 
This code is in the public domain.
sACN E 1.31 is a public standard published by the PLASA technical standards program
http://tsp.plasa.org/tsp/documents/published_docs.php
 
Requires Arduino Ethernet Shield. Modifed to use the lastest version of Ethernet library included in Arduino 1.6.1. No need to install other libraries. 
Receives E1.31 data (SACN) and ArtNet data for control of relays. This sketch is designed for use with SainSmart Relay boards.  
You can only send the Arduino one protocol (E1.31 or ArtNet) at a time. 
This sketch also includes a power on self test function to make sure everything is hooked up properly.
*/

#include <SPI.h>    //Standard Library     
#include <Ethernet.h> //Standard Library
#include <EthernetUdp.h> //Standard Library


/*  set the desired subnet and universe (first universe is 0)
    sACN starts with universe 1 so subtract 1 from sACN universe
    to get DMX_UNIVERSE.                                           */
#define ARTNET_SUBNET 0 //defualt subnet is 0. Should not need to be changed. 
#define ARTNET_UNIVERSE 0 //first universe being used
#define E131_SUBNET 0 //defualt subnet is 0. Should not need to be changed. 
#define ETHERNET_BUFFER_MAX 640
#define ARTNET_ARTDMX 0x5000
#define ARTNET_ARTPOLL 0x2000
#define ARTNET_PORT 0x1936 //standard port
#define ARTNET_START_ADDRESS 18 //Byte 18 in the packet contains channel 1 values.  
#define E131_PORT 5568 //standard port 
#define E131_START_ADDRESS 126 //Byte 126 in the packet contains channel 1 values. Offset is set to one prior. 
#define STATUS_LED 13 //shows us when we are receiving data
#define SDCARD_CONTROL 4 //set pin 4 to high to disable SD card interface on WiFi shield 
#define NUM_RELAYS 8 //total number of relays used 

int channel; //channel increment 

//Relay Pins array
int Relay[] = {22, 23, 24, 25, 26, 27, 28, 29};

//Timer Setup
volatile byte currentcounter = 0; //counter for data reception
byte previouscounter = 0; //counter for data reception 
unsigned long currentDelay = 0; //current time value for ArtNet and E1.31 reception

//Ethernet Configuration
byte mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
IPAddress ip(192, 168, 1, 120);  //IP address of ethernet shield

// buffer to hold E1.31 and Artnet data
unsigned char packetBuffer[ETHERNET_BUFFER_MAX];

// An EthernetUDP instance to let us send and receive packets over UDP
// aUDP creates a socket for an Art-Net port
// suUDP creates a socket for sACN that is unicast to the ip address
EthernetUDP aUDP;
EthernetUDP suUDP;

void setup() 
{
  pinMode(STATUS_LED, OUTPUT); //Initialize status LED
  pinMode(SDCARD_CONTROL, OUTPUT); //Initialize Pin 4
  digitalWrite(SDCARD_CONTROL, HIGH); //Set pin 4 to high since we are not using the SD Card interface
  for(int a = 0; a < NUM_RELAYS; a++) //loop to initialize relay control pins 
  {
   pinMode(Relay[a], OUTPUT); //initialize relay output pins
   digitalWrite(Relay[a], HIGH); //set pins to high for off
  }
  Ethernet.begin(mac,ip);  //Initialize Ethernet shield 
  aUDP.begin(ARTNET_PORT); //Open Artnet Port
  suUDP.begin(E131_PORT); //Open E1.31 Port
  Serial.begin(9600); //Serial for debugging 
  POST(); //Run Power On Self Test Function
  Serial.println("Setup Complete"); //print complete 
}

/* artDMXReceived checks the universe and subnet then
   outputs the data to the relays */

void artDMXReceived(unsigned char* pbuff) 
{
  if ( (pbuff[14] & 0xF) == ARTNET_UNIVERSE ) 
  {
    if ( (pbuff[14] >> 8) == ARTNET_SUBNET ) 
    {
      channel = 0; //reset channel offset to 0 each time through loop
      //loop turns relay on/off based on channel value starting at Artnet start address
      for(int b = 0; b < NUM_RELAYS; b++) 
      {
       if(pbuff[ARTNET_START_ADDRESS + channel] > 127) //if channel value is greater then 127
      {
       digitalWrite(Relay[b], LOW); //turn relay on 
      } 
      else
      {
       digitalWrite(Relay[b], HIGH); //else turn it off 
      }
      channel++; //increase channel offset by 1 each time through for loop
      }
    }
  }
} //end artDMXReceived 

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
   outputs the data to relays  */

void sacnDMXReceived(unsigned char* pbuff, int count) 
{
    if ( pbuff[113] == E131_SUBNET ) 
  {  
      int addressOffset = 125; //first 125 bytes of packet are header information
      if ( pbuff[addressOffset] == 0 ) //start code must be 0
      {  
       channel = 0; //reset channel offset to 0 each time through loop
       //loop turns relay on/off based on channel value starting at E1.31 start address
      for(int c = 0; c < NUM_RELAYS; c++)
      {
       if(pbuff[E131_START_ADDRESS + channel] > 127) //if channel value is greater then 127
      {
       digitalWrite(Relay[c], LOW); //turn relay on
      } 
      else
      {
       digitalWrite(Relay[c], HIGH); //turn relay off 
      }
      channel++; //increment channel offset by 1
      }
      }
      } 
    } //end sacnDMXReceived 
    
  
  
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
  the DMX values to the output. There also is a timer to run a standby 
  program if no data is received for 30 seconds. 

*************************************************************************/

void loop() 
{
  if(currentcounter != previouscounter) //has the value changed?
  {
    currentDelay = millis(); //store the time since the value has increased 
    previouscounter = currentcounter; //set the previous value equal to the current value
  }
  
  if(millis() - currentDelay > 30000) //is the time since the value changed greater than 30 seconds?
  { 
   digitalWrite(STATUS_LED, LOW); //turn LED off. Not receiving E1.31 or ArtNet. 
  }
  
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
      currentcounter++;  //increase counter by 1 each time through 
      digitalWrite(STATUS_LED, HIGH); //turn status LED on
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
        currentcounter++;  //increase counter by 1 each time through 
      digitalWrite(STATUS_LED, HIGH); //turn status LED on
      }
    }
      
    }
  
} //End loop

//Power on self test function to make sure everything is connected properly. 
void POST()
{
  digitalWrite(Relay[0], LOW);
  delay(1000);
  digitalWrite(Relay[0], HIGH);
  digitalWrite(Relay[1], LOW);
  delay(1000);
  digitalWrite(Relay[1], HIGH);
  digitalWrite(Relay[2], LOW);
  delay(1000);
  digitalWrite(Relay[2], HIGH);
  digitalWrite(Relay[3], LOW);
  delay(1000);
  digitalWrite(Relay[3], HIGH);
  digitalWrite(Relay[4], LOW);
  delay(1000);
  digitalWrite(Relay[4], HIGH);
  digitalWrite(Relay[5], LOW);
  delay(1000);
  digitalWrite(Relay[5], HIGH);
  digitalWrite(Relay[6], LOW);
  delay(1000);
  digitalWrite(Relay[6], HIGH);
  digitalWrite(Relay[7], LOW);
  delay(1000);
  digitalWrite(Relay[7], HIGH);
  Serial.println("POST Complete");
}


