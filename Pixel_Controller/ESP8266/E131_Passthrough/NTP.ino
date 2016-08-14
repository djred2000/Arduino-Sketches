//Functions to receive and process NTP time. 
unsigned long sendNTPpacket(char* address)
{
  memset(NTPBuffer, 0, NTP_PACKET_SIZE);
  NTPBuffer[0] = 0b11100011;   // LI, Version, Mode
  NTPBuffer[1] = 0;     // Stratum, or type of clock
  NTPBuffer[2] = 6;     // Polling Interval
  NTPBuffer[3] = 0xEC;  // Peer Clock Precision
  NTPBuffer[12]  = 49;
  NTPBuffer[13]  = 0x4E;
  NTPBuffer[14]  = 49;
  NTPBuffer[15]  = 52;
  Nudp.beginPacket(address, 123); //NTP requests are to port 123
  Nudp.write(NTPBuffer, NTP_PACKET_SIZE);
  Nudp.endPacket();
}

void SendTime()
{
  while(TimeSent == false)
  {
 sendNTPpacket(timeServer); // send an NTP packet to a time server
  delay(1000);
  int cb = Nudp.parsePacket();
  Nudp.read(NTPBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  if(NTPBuffer[2] == 6 && NTPBuffer[3] == 227)
  {
  Serial.write("NTP"); //send packet header 
  Serial.write(NTPBuffer, NTP_PACKET_SIZE); //Write NTP packet to Serial port 
  Serial.flush(); //flush transmit buffer
  TimeSent = true; 
}
}
}




