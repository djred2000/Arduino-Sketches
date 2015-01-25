#include <SPI.h>         // needed for Arduino versions later than 0018
#include <WiFi.h>
#include <WiFiUdp.h>

                              


// enter desired universe and subnet  (sACN first universe is 1)
#define DMX_SUBNET 0
#define DMX_UNIVERSE 1
#define SACN_PORT 5568
#define SACN_BUFFER_MAX 640

//the initial ip and MAC address will get changed by beginMulti
//the multicast ip address should correspond to the desired universe/subnet


char ssid[] = "Rue Paw";  //  your network SSID (name)
char pass[] = "phfops@2014";       // your network password
IPAddress ip(192, 168, 1, 120);  //IP address of ethernet shield



// buffers for receiving and sending data
unsigned char packetBuffer[SACN_BUFFER_MAX]; //buffer to hold incoming packet,

// An EthernetUDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

//setup initializes Ethernet, opens the UDP port and initializes pixels   
void setup() 
{
WiFi.begin(ssid, pass); //connect to WiFi network
WiFi.config(ip); //set static ip address
delay(10000); //wait 10 seconds to make sure we are connected
Udp.begin(SACN_PORT); //open SACN port
Serial.begin(9600); //initialize serial port for debugging
} //end setup

void sacnDMXReceived(unsigned char* pbuff, int count) 
{
    if ( pbuff[113] == DMX_SUBNET ) 
  {  
      int addressOffset = 125; //first 125 bytes of packet are header information
      if ( pbuff[addressOffset] == 0 ) //start code must be 0
      {  
      //place code here to use E1.31 channel values pbuff[126] - pbuff[638]
      
      }
      
    }
  }
  


//checks to see if packet is E1.31 data
int checkACNHeaders(unsigned char* messagein, int messagelength) 
{
  if ( messagein[1] == 0x10 ) 
  {	//header
   String test = String((char*)&messagein[4]);
   if ( test.equals("ASC-E1.17") ) 
   {
    int rootsizeflags = messagein[16] * 256 + messagein[17];
    if ( (0x7000 + messagelength-16) == rootsizeflags ) 
    {
     int framingsizeflags = messagein[38] * 256 + messagein[39];
     if ( (0x7000 + messagelength-38) == framingsizeflags ) 
     {
      int dmpsizeflags = messagein[115] * 256 + messagein[116];
      if ( (0x7000 + messagelength-115) == dmpsizeflags ) 
      {
        int addresscount = messagein[123] * 256 + messagein[124]; // number of addresses plus start code
        if ( (messagelength-125) == addresscount ) 
        {
          return addresscount;
        }
      }
     }
    }
   }
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
            sacnDMXReceived(packetBuffer, count); //process data function
    }
  }
  
} //end loop



