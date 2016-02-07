#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const int BUFFER_SIZE = 640;
unsigned char PacketBuffer[BUFFER_SIZE]; //buffer to hold incoming packet data
const int DMXSubnet = 0;
const int DMXUniverse = 1;

//WiFi Setup
char ssid[] = "SSID";
char pass[] = "PASSWORD";
IPAddress local_ip(192, 168, 1, 15);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress MulticastIP(239, 255, DMXSubnet, DMXUniverse);
const int E131Port = 5568;
WiFiUDP Eudp; //UDP socket for Unicast E1.31
//WiFiUDP MEudp; //UDP socket for Multicast E1.31 

//FastLED Definitions
const int NUM_LEDS = 170;
const int PixelPin = 0;
CRGB leds[NUM_LEDS];
const int E131_ADDRESS_OFFSET = 125;
const int ChannelWidth = 3;
int Channel;

void setup()
{
  Serial.begin(250000); 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass); //Connect to Network
  WiFi.config(local_ip, gateway, subnet); //Set static IP information
  while (WiFi.status() != WL_CONNECTED)  //wait until we are connected
  {
    delay(500);
  }
  //MEudp.beginMulticast(WiFi.localIP(), MulticastIP, E131Port);
  Eudp.begin(E131Port); //Open E1.31 port
  LEDS.addLeds<WS2811, PixelPin, RBG>(leds, NUM_LEDS);
  LEDS.setBrightness(255);
  POST();
  Serial.println("Setup Complete");
}

void loop()
{
//Make sure we are still connected to the AP each time through the loop.
if(WiFi.status() != WL_CONNECTED)
{
  ESP.restart(); //restart the board 
}
 
  /* Start by checking to see if a packet is available on the E1.31 port */
  int packetSize = Eudp.parsePacket(); //parse packet
  if (packetSize)
  {
    Eudp.read(PacketBuffer, BUFFER_SIZE); //read packet into buffer

    int count = checkE131Headers(PacketBuffer, BUFFER_SIZE); //pass data to see if it is E1.31

    if ( count ) //if it comes back genuine
    {
      if(PacketBuffer[114] == 1) //byte 114 is universe number
      {
      Channel = 1; //reset channel assignment to 1 each time through loop.
      for(int i = 0; i < NUM_LEDS; i++) //pixels 0-170
      {
      leds[i] = CRGB(PacketBuffer[E131_ADDRESS_OFFSET + Channel], PacketBuffer[E131_ADDRESS_OFFSET + (Channel +1)], PacketBuffer[E131_ADDRESS_OFFSET + (Channel + 2)]);
      Channel += ChannelWidth; //increase last channel number by channel width
      }
      FastLED.show(); //Send data to pixels 
      Serial.print("FastLED FPS: ");
      Serial.println(FastLED.getFPS()); //prints frame rate
      }
    }
  }
} //end loop



