void sendNTPpacket(char* address)
{
  Serial.println("Sending NTP Time Request");
  memset(NTPBuffer, 0, NTP_PACKET_SIZE);
  NTPBuffer[0] = 0b11100011;  
  NTPBuffer[1] = 0;     
  NTPBuffer[2] = 6;     
  NTPBuffer[3] = 0xEC;  
  NTPBuffer[12]  = 49;
  NTPBuffer[13]  = 0x4E;
  NTPBuffer[14]  = 49;
  NTPBuffer[15]  = 52;
  NTPUDP.beginPacket(address, 123); //NTP requests are to port 123
  NTPUDP.write(NTPBuffer, NTP_PACKET_SIZE);
  NTPUDP.endPacket();
}

void TimeProcess()
{
  sendNTPpacket(timeServer);
  delay(2000);
  if (NTPUDP.parsePacket()) 
  {
    NTPUDP.read(NTPBuffer, NTP_PACKET_SIZE);
    Serial.println("NTP Time Data Received"); 
    unsigned long highWord = word(NTPBuffer[40], NTPBuffer[41]);
    unsigned long lowWord = word(NTPBuffer[42], NTPBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = secsSince1900 - seventyYears;
    //Serial.println(epoch);
    time_t CurrentTime = epoch;
    RTC.set(CurrentTime);
    ActualTime = RTC.get();
    Serial.print("Current UTC Time: ");
    Serial.print(hour(ActualTime));
    Serial.print(":");
    Serial.print(minute(ActualTime));
    Serial.print(":");
    Serial.println(second(ActualTime));
    previousMinute = ActualTime;
    if(RTC.chipPresent() == 1)
    {
    TimeSet = true;
    }
}
}


