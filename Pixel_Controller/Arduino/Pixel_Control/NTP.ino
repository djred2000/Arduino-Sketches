//Process NTP time data and set RTC
void TimeProcess()
{
  if(DataBuffer[2] == 6 && DataBuffer[3] == 227) //NTP packet constants
  {
    unsigned long highWord = word(DataBuffer[40], DataBuffer[41]);
    unsigned long lowWord = word(DataBuffer[42], DataBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = secsSince1900 - seventyYears;
    //Serial.println(epoch);
    time_t CurrentTime = epoch; //Convert to time variable
    RTC.set(CurrentTime); //set RTC time
    if(RTC.chipPresent() == 1)
    {
    ActualTime = RTC.get(); //Get time from RTC
    Serial.print("Current UTC Time: ");
    Serial.print(hour(ActualTime));
    Serial.print(":");
    Serial.print(minute(ActualTime));
    Serial.print(":");
    Serial.println(second(ActualTime));
    previousMinute = ActualTime;
    SetTime = true;
    }
    else
  {
    Serial.println("No RTC chip present. Unable to set time.");
    SetTime = true;
  }
  }
}

