#include <WiFi.h>
#include <WiFiUdp.h>
#include "esp_wifi.h"

const int BUFFER_SIZE = 638;
unsigned char PacketBuffer[BUFFER_SIZE]; //buffer to hold incoming packet data
boolean LEDState = true;
unsigned long currentcounter;
unsigned long previouscounter;
unsigned long StandbyDelay = 1000;
unsigned long currentDelay;

//WiFi Setup
char ssid[] = "*****"; //WiFi SSID
char pass[] = "*****"; //WiFi Password
IPAddress local_ip(192, 168, 1, 14); //Static IP information
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
const int E131Port = 5568;
WiFiUDP Eudp; //UDP socket for Unicast E1.31

void setup()
{
  Serial.begin(2000000); //2 megabit 
  WiFi.mode(WIFI_STA); //wifi client mode
  esp_wifi_set_ps (WIFI_PS_NONE); //disable wifi power saving mode
  WiFi.begin(ssid, pass); //Connect to Network
  WiFi.config(local_ip, gateway, subnet); //Set static IP information
  while (WiFi.status() != WL_CONNECTED)  //wait until we are connected
  {
    delay(500);
    //Serial.println("Connecting");
  }
  Eudp.begin(E131Port); //Open E1.31 port
  pinMode(2, OUTPUT); //set onboard LED to output
  digitalWrite(2, LOW); //turn onboard LED off
  //WiFi.printDiag(Serial); //prints wifi status to serial port
  //Serial.println("Setup Complete"); //prints setup complete text to serial port
  //delay(5000); //pause for 5 seconds 
  ConnectionCompleted();
}

void loop()
{
/*
//Make sure we are still connected to the AP each time through the loop.
if(WiFi.status() != WL_CONNECTED)
{
  ESP.restart(); //restart the board 
}
*/
  /* Start by checking to see if a packet is available on the E1.31 port */
  int packetSize = Eudp.parsePacket(); //parse packet
  
  if (packetSize)
  {
    if(LEDState == false)
    {
      digitalWrite(2, HIGH); //turn LED on
    }
    Eudp.read(PacketBuffer, BUFFER_SIZE); //read packet into buffer
    int count = checkE131Headers(PacketBuffer, BUFFER_SIZE); //pass data to see if it is E1.31
    if (count) //if it comes back genuine
    {
      LEDState = true;
      currentcounter++;
      //Serial.println(PacketBuffer[114]);
      Serial.write("SAC"); //packet header 
      Serial.write(PacketBuffer, BUFFER_SIZE); //write packet to serial port 
      Serial.flush(); //flush transmit buffer at end of for loop
    }
  }

 //Timer 
if(currentcounter != previouscounter) //has the value changed?
  {
    currentDelay = millis(); //store the time since the value has increased 
    previouscounter = currentcounter; //set the previous value equal to the current value
  }
  
  if(millis() - currentDelay > StandbyDelay) //current time minus time since the counter has changed.
  { 
   digitalWrite(2, LOW); //turn LED off. Not receiving E1.31 
   LEDState = false;
  }

} //end loop
