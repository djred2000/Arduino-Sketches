void POST() 
{
   LEDS.showColor(CRGB(255, 0, 0)); //turn all pixels on red
   delay(1000);
   LEDS.showColor(CRGB(0, 255, 0)); //turn all pixels on green
   delay(1000);
   LEDS.showColor(CRGB(0, 0, 255)); //turn all pixels on blue
   delay(1000);
   FastLED.clear(); //clear pixels
   FastLED.show();
   Serial.println("POST Completed");
}
