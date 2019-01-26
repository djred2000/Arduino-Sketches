#include <WiFi.h>
#include <WiFiUdp.h>

const int BUFFER_SIZE = 638;
unsigned char PacketBuffer[BUFFER_SIZE]; //buffer to hold incoming packet data

//WiFi Setup
char ssid[] = "RuePaw";
char pass[] = "ritas4life89";
IPAddress local_ip(192, 168, 1, 14);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
const int E131Port = 5568;
WiFiUDP Eudp; //UDP socket for Unicast E1.31

void setup()
{
  Serial.begin(2000000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass); //Connect to Network
  WiFi.config(local_ip, gateway, subnet); //Set static IP information
  while (WiFi.status() != WL_CONNECTED)  //wait until we are connected
  {
    delay(500);
    //Serial.println("Connecting");
  }
  Eudp.begin(E131Port); //Open E1.31 port
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  //WiFi.printDiag(Serial);
  //Serial.println("Setup Complete");
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
    if (count) //if it comes back genuine
    {
      digitalWrite(2, HIGH); //turn onboard LED on
      //Serial.println(PacketBuffer[114]);
      Serial.write("SAC"); //packet header 
      Serial.write(PacketBuffer, BUFFER_SIZE); //write packet to serial port 
      Serial.flush(); //flush transmit buffer at end of for loop
    }
  }
  else
  {
    digitalWrite(2, LOW); //turn onboard LED off 
  }
} //end loop



