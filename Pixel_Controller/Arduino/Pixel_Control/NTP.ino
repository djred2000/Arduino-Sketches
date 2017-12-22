void InitializeRTC()
{
  if(RTC.chipPresent() == 0)
 {
    ActualTime = RTC.get(); //Get time from RTC
    /*
    Serial.print("Current UTC Time: ");
    Serial.print(hour(ActualTime));
    Serial.print(":");
    Serial.print(minute(ActualTime));
    Serial.print(":");
    Serial.println(second(ActualTime));
    */
    previousTime = ActualTime;
   digitalWrite(ledpin, HIGH); 
   delay(200);
   digitalWrite(ledpin, LOW);
   delay(200);
   digitalWrite(ledpin, HIGH); 
   delay(200);
   digitalWrite(ledpin, LOW);
   delay(200);
   digitalWrite(ledpin, HIGH); 
   delay(200);
   digitalWrite(ledpin, LOW);
   delay(200);
 }
}

