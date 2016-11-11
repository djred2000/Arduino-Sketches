#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <GwtS.h>

GWTS ears; //GWTS instance

//Network settings 
char ssid[] = "SSID";
char pass[] = "PASSWORD";
IPAddress local_ip(192, 168, 1, 14);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const int E131_BUFFER_SIZE = 638; //size of E1.31 data packet
const int IRledPin = 5; //pin IR LED is connect to
unsigned int localPort = 5568; //E1.31 network port
unsigned int currentcounter = 0; //counter for data reception
unsigned int previouscounter = 0; //counter for data reception 
unsigned long currentDelay = 0; //current time value for ArtNet and E1.31 reception
boolean standby = false; //mode select
uint8_t E131Buffer[E131_BUFFER_SIZE]; //array to store E1.31 packet
byte cmdbuf[6]; //buffer to store GWTS fade data for checksum calculation
unsigned char checksum; //GWTS checksum variable

WiFiUDP udp; //UDP instance to read E1.31 packets

//checks to see if packet is E1.31 data
int checkE131Headers(unsigned char* messagein, int messagelength) 
{
  if ( messagein[1] == 0x10 && messagein[4] == 0x41 && messagein[12] == 0x37) 
  {  
      int ChannelCount = messagein[123] * 256 + messagein[124]; // number of values plus start code
      return ChannelCount -1; //Return how many values are in the packet.
    }
  return 0;
}

void setup()
{
  pinMode(2,OUTPUT); //onboard LED for diagnostics
  pinMode(IRledPin, OUTPUT); //IR LED output
  digitalWrite(2, HIGH); //Set onboard LED off
  Serial.begin(250000); 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass); //Connect to Network
  WiFi.config(local_ip, gateway, subnet);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  udp.begin(localPort);
  //Flash onboard LED twice if connection is successful
  digitalWrite(2, LOW);
  delay(300);
  digitalWrite(2, HIGH);
  delay(300);
  digitalWrite(2, LOW);
  delay(300);
  digitalWrite(2, HIGH);
}

void loop()
{
 int packetSize = udp.parsePacket(); //parse packet
 
  if (packetSize)
  {
    udp.read(E131Buffer, E131_BUFFER_SIZE); //read packet into buffer

    int count = checkE131Headers(E131Buffer, E131_BUFFER_SIZE); //pass data to see if it is E1.31

    if ( count ) //if it comes back genuine
    {
      digitalWrite(2, LOW); //turn onboard LED on
      currentcounter++; //increase counter
      standby = false; //not in standby 
      
  if(E131Buffer[132] == 10) //fade control is on channel 7
   {
      fadedown(); //if value is equal to 10 send the fade down command
   }
     if(E131Buffer[132] == 12) //fade control is on channel 7
   {
      fadeup(); //if value is equal to 12 send the fade up command
   }
   
   GWTSOutput(); //output using the GWTS library 

    }
  }

  if(currentcounter != previouscounter) //has the value changed?
  {
    currentDelay = millis(); //store the time since the value has increased 
    previouscounter = currentcounter; //set the previous value equal to the current value
  }
  
  if(millis() - currentDelay > 10000) //current time minus time since the counter has changed. After 10 seconds of no data being received 
  {
    if(standby == false) //only want to send standby once we stop receiving E1.31 
    { 
   digitalWrite(2, HIGH); //turn LED off. Not receiving E1.31
   powerOnReset(); //send reset to GWTS ears 
   standby = true;
    }
  }


  
} //end loop

void GWTSOutput()
{
    ears.set_colors(E131Buffer[126],E131Buffer[127],E131Buffer[128],E131Buffer[129],E131Buffer[130],E131Buffer[131]); //ouput GWTS data 
}




