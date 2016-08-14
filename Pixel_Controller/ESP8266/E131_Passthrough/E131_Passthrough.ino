/*
 * This sketch receives E1.31 and Artnet data over WiFi and write its out over the serial port. It also received NTP time data for setting time in a RTC module.   
 * This runs best on a NodeMCU ESP8266 board. Using serial port 1. 
 */
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const int ARTNET_ARTDMX = 0x5000; //20480 as a decimal ArtNet OP Code
const int BUFFER_SIZE = 638;
unsigned char PacketBuffer[BUFFER_SIZE]; //buffer to hold incoming packet data
unsigned int currentcounter = 0; //counter for data reception
unsigned int previouscounter = 0; //counter for data reception 
unsigned long currentDelay = 0; //current time value for ArtNet and E1.31 reception

//WiFi Setup. Change to your specific network setup. 
char ssid[] = "*****";
char pass[] = "*****";
IPAddress local_ip(192, 168, 1, 15);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
const int E131Port = 5568;
const int ART_NET_PORT = 6454;
WiFiUDP Eudp; //UDP socket for Unicast E1.31
WiFiUDP Audp; //UDP socket for ArtNet
WiFiUDP Nudp; //UDP socket for NTP Time

/* NTP Definitions */
unsigned int NTPport = 2390;      // local port to listen for UDP packets
char timeServer[] = "time.nist.gov";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte NTPBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
char NTPRequest;
boolean TimeSent = false;

void setup()
{
  Serial.begin(2000000); //2,000,000 bits per second 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass); //Connect to Network
  WiFi.config(local_ip, gateway, subnet); //Set static IP information
  while (WiFi.status() != WL_CONNECTED)  //wait until we are connected
  {
    delay(500);
  }
  Eudp.begin(E131Port); //Open E1.31 port
  Audp.begin(ART_NET_PORT); //Open ArtNet port
  Nudp.begin(NTPport); //Open NTP Port
  pinMode(2, OUTPUT); //onboard LED
  digitalWrite(2, HIGH); //On ESP8266 control is reversed. HIGH = off, LOW = on. 
}

void loop()
{
//Make sure we are still connected to the AP each time through the loop.
if(WiFi.status() != WL_CONNECTED)
{
  ESP.restart(); //restart the board 
}
  
  if (Serial.available() > 0) //Check for incoming serial data
  {
    NTPRequest = Serial.read(); //read data
    if (NTPRequest == 'Z') //Arduino will send Z when it wants time
    {
      TimeSent = false;
      SendTime(); //Send NTP time to Arduino.
    }
  }

  /* Start by checking to see if a packet is available on the E1.31 port */
  int packetSize = Eudp.parsePacket(); //parse packet
  if (packetSize)
  {
    Eudp.read(PacketBuffer, BUFFER_SIZE); //read packet into buffer
    int count = checkE131Headers(PacketBuffer, BUFFER_SIZE); //pass data to see if it is E1.31
    if (count) //if it comes back genuine
    {
      digitalWrite(2, LOW);
      currentcounter++;
      Serial.write("SAC"); //packet header 
      Serial.write(PacketBuffer, BUFFER_SIZE); //write packet to serial port 
      Serial.flush(); //flush transmit buffer at end of for loop
    }
  }
  else
  {
    /* If no E1.31 packet is available then check the ArtNet port */
    int packetSize = Audp.parsePacket(); //Parse Packet

    if ( packetSize ) //ArtNet packet size is 530
    {
      Audp.read(PacketBuffer, BUFFER_SIZE); //Read packet into buffer
      int opcode = artNetOpCode(PacketBuffer); //Check to see if packet is ArtNet
      if (opcode == ARTNET_ARTDMX) //If code returned is correct
      {
        digitalWrite(2, LOW);
        currentcounter++;
        Serial.write("ART"); //packet header
        Serial.write(PacketBuffer, BUFFER_SIZE); //write packet to serial port 
        Serial.flush(); //flush transmit buffer at end of for loop
      }
    }
  }
  
 //Timer for onboard LED. Light is on when receiving data. Off when no data is being received. 
if(currentcounter != previouscounter) //has the value changed?
  {
    currentDelay = millis(); //store the time since the value has increased 
    previouscounter = currentcounter; //set the previous value equal to the current value
  }
  
  if(millis() - currentDelay > 5) //current time minus time since the counter has changed. 
  { 
   digitalWrite(2, HIGH); //turn LED off. Not receiving E1.31 or ArtNet. 
  }
  
} //end loop



