// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  //Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(NTPBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  NTPBuffer[0] = 0b11100011;   // LI, Version, Mode
  NTPBuffer[1] = 0;     // Stratum, or type of clock
  NTPBuffer[2] = 6;     // Polling Interval
  NTPBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  NTPBuffer[12]  = 49;
  NTPBuffer[13]  = 0x4E;
  NTPBuffer[14]  = 49;
  NTPBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Nudp.beginPacket(address, 123); //NTP requests are to port 123
  Nudp.write(NTPBuffer, NTP_PACKET_SIZE);
  Nudp.endPacket();
}

//Get NTP Time from server and send to Arduino
void SendTime()
{
 sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
    int cb = Nudp.parsePacket();
    // We've received a packet, read the data from it
    Nudp.read(NTPBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    Serial.write("NTP");
    for(int i = 40 ; i < 44; i++)
    {
      Serial.write(NTPBuffer[i]);
    }
    Serial.flush(); //flush transmit buffer 
}




