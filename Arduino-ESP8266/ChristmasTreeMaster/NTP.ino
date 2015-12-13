//This function processes the NTP data and sets the RTC to the current time. It then reads the time from the RTC and prints it. 

void TimeProcess(unsigned char* Buffer)
{
    unsigned long highWord = word(Buffer[0], Buffer[1]);
    unsigned long lowWord = word(Buffer[2], Buffer[3]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    Serial.println(epoch);
    time_t CurrentTime = epoch; //Convert to time variable
    RTC.set(CurrentTime); //set RTC time
    ActualTime = RTC.get(); //Get time from RTC
    Serial.print("Current UTC Time: ");
    Serial.print(hour(ActualTime));
    Serial.print(":");
    Serial.print(minute(ActualTime));
    Serial.print(":");
    Serial.println(second(ActualTime));
    previousMinute = ActualTime;
}


