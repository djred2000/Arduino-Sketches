#include <Time.h>
#include <TimeLib.h>
#include <Wire.h>
#include <DS1307RTC.h>
#include <FastLED.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

//Ethernet Definitions
byte mac[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};
IPAddress ip(192, 168, 1, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dnsServer(192, 168, 1, 1);
const int ARTNET_ARTDMX = 0x5000;
const int ARTNET_ARTPOLL = 0x2000;
const int ARTNET_PORT = 0x1936; 
const int E131_PORT = 5568; 
EthernetUDP E131UDP;
EthernetUDP ARTNETUDP; 
EthernetUDP NTPUDP;

//General Definitions
const int ledpin = 13; //LED on board for monitoring
const int E131_ADDRESS_OFFSET = 125; //byte 125 of the E1.31 packet contains channel 1 value
const int ARTNET_ADDRESS_OFFSET = 11; //byte 12 of the ArtNet packet contains channel 1 value
const int E131_GWTS_START = 109; //GWTS ears start on byte 105 and go for 6 bytes. RGB each ear.
const int ARTNET_GWTS_START = 12; //GWTS ears start on byte 12 and go for 6 bytes. RGB each ear.
const int E131_START_CODE = 0; //

//Data Reception Timer Setup
unsigned int currentcounter = 0; //counter for data reception
unsigned int previouscounter = 0; //counter for data reception 
unsigned long currentDelay = 0; //current time value for ArtNet and E1.31 reception
tmElements_t tm; //time variable
time_t previousMinute; //time variable
time_t ActualTime; //time variable 
boolean TimeSet = false;
unsigned int NTPPort = 8888;       // local port to listen for UDP packets
char timeServer[] = "time.nist.gov"; // time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte NTPBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// buffers for receiving data
const int EthernetBufferMax = 640;
unsigned char DataBuffer[EthernetBufferMax]; //buffer to hold incoming E1.31 and ArtNet Data

//FastLED Definitions
#define PIXELTYPE WS2811
const int NUM_STRIPS = 3;
const int NUM_LEDS_PER_STRIP = 100;
const int NUM_LEDS = 571;
const int DATA_PIN1 = 25; 
const int DATA_PIN2 = 26;
const int DATA_PIN3 = 27;
const int DATA_PIN4 = 28;
const int ChannelWidth = 3; //each pixel needs 3 channels for control
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
digitalWrite(13, LOW); //turn LED off
Serial.begin(250000); //USB serial port for monitoring/debugging. Capped at 250,000 bps on programming port. Use USBSerial instead. 
Ethernet.begin(mac, ip, dnsServer, gateway, subnet);
ARTNETUDP.begin(ARTNET_PORT);
E131UDP.begin(E131_PORT);
NTPUDP.begin(NTPPort);
Serial.print("IP Address: ");
Serial.println(Ethernet.localIP());
//SerialUSB.begin(0); //Due USB Serial on Native port
FastLED.addLeds<PIXELTYPE, DATA_PIN1, RGB>(leds, 0, 150); //pixel strip 1
FastLED.addLeds<PIXELTYPE, DATA_PIN2, RGB>(leds, 150, 150); //pixel strip 2
FastLED.addLeds<PIXELTYPE, DATA_PIN3, RGB>(leds, 300, 150); //pixel strip 3
FastLED.addLeds<PIXELTYPE, DATA_PIN4, RGB>(leds, 450, 121); //pixel strip 4
LEDS.setBrightness(255);
while(TimeSet == false)
{
  TimeProcess();
}
POST(); //Run power on self test function each time board boots 
Serial.println("Setup Completed");
}

void loop() 
{
  int packetSize = ARTNETUDP.parsePacket();
  if( packetSize )
  {
    ARTNETUDP.read(DataBuffer, EthernetBufferMax);
    int opcode = artNetOpCode(DataBuffer);
    if ( opcode == ARTNET_ARTDMX ) 
    {
      Serial.println("ArtNet Packet Received");
      artDMXReceived(DataBuffer);
      currentcounter++;
      digitalWrite(13, HIGH);
    } 
  } 
    else 
    {
    packetSize = E131UDP.parsePacket();
    if( packetSize ) 
    {
      E131UDP.read(DataBuffer, EthernetBufferMax);
     
      int count = checkACNHeaders(DataBuffer, packetSize);
      if ( count ) 
      {
        Serial.println("E131 Packet Received");
        E131Received(DataBuffer, count);
        currentcounter++;
        digitalWrite(13, HIGH);
      }
    }
    }

 //Timer runs standby pixel functions when no E1.31 or ArtNet data is being received. 
if(currentcounter != previouscounter) //has the value changed?
  {
    currentDelay = millis(); //store the time since the value has increased 
    previouscounter = currentcounter; //set the previous value equal to the current value
  }
  
  if(millis() - currentDelay > 15000) //current time minus time since the counter has changed. After 30 seconds of no data being received 
  { 
   digitalWrite(13, LOW); //turn LED off. Not receiving E1.31 or ArtNet. 
   Standby(); //run pixel standby function 
   
  }
}



