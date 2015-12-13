#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define ARTNET_ARTDMX 0x5000 //20480 as a decimal ArtNet OP Code

//WiFi Setup
char ssid[] = "SSID";
char pass[] = "PASSWORD";
IPAddress local_ip(192, 168, 1, 15);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
#define E131Port 5568
#define ART_NET_PORT 6454
WiFiUDP Eudp; //UDP socket for E1.31
WiFiUDP Audp; //UDP socket for ArtNet
WiFiUDP Nudp; //UDP socket for NTP Time

#define BUFFER_SIZE 640
unsigned char PacketBuffer[BUFFER_SIZE]; //buffer to hold incoming packet data

/* NTP Definitions */
unsigned int NTPport = 2390;      // local port to listen for UDP packets
IPAddress timeServer(24, 56, 178, 140); // NTP server
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte NTPBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
char NTPRequest;

void setup()
{
  Serial.begin(750000); //750,000 bits per second to keep up with UDP data coming in
  WiFi.begin(ssid, pass); //Connect to Network
  WiFi.config(local_ip, gateway, subnet); //Set static IP information
  while (WiFi.status() != WL_CONNECTED)  //wait until we are connected
  {
    delay(500);
  }
  Eudp.begin(E131Port); //Open E1.31 port
  Audp.begin(ART_NET_PORT); //Open ArtNet port
  Nudp.begin(NTPport); //Open NTP Port
  delay(10000); //Wait 10 seconds for Arduino to boot
}

void loop()
{
  if (Serial.available() > 0) //Check for incoming serial data
  {
    NTPRequest = Serial.read(); //read data
    if (NTPRequest == 'Z') //Arduino will send Z when it wants time
    {
      SendTime(); //Send NTP time to Arduino.
    }
  }

  /* Start by checking to see if a packet is available on the E1.31 port */
  int packetSize = Eudp.parsePacket(); //parse packet
  if (packetSize)
  {
    Eudp.read(PacketBuffer, BUFFER_SIZE); //read packet into buffer

    int count = checkE131Headers(PacketBuffer, BUFFER_SIZE); //pass data to see if it is E1.31

    if ( count ) //if it comes back genuine
    {
      Serial.write("SAC"); //Send data header
      for (int i = 114; i < 638; i++)
      {
        Serial.write(PacketBuffer[i]); //write bytes 114-638 in packet to serial port. Byte 114 contains packet universe number.
      }
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

      if ( opcode == ARTNET_ARTDMX ) //If code returned is correct
      {
        Serial.write("ART");
        for (int i = 6; i < 530; i++)
        {
          Serial.write(PacketBuffer[i]);
        }
        Serial.flush(); //flush transmit buffer at end of for loop
      }
    }
  }

} //end loop



