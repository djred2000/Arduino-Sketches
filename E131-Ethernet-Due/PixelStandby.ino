//Power on self test for pixels and ears
void POST() 
{
   LEDS.showColor(CRGB::Red); //turn all pixels on red
   delay(1000);
   LEDS.showColor(CRGB::Green); //turn all pixels on green
   delay(1000);
   LEDS.showColor(CRGB::Blue); //turn all pixels on blue
   delay(1000);
   FastLED.clear(); //clear pixels
   FastLED.show();
   Serial.println("POST Completed");
}

//This function runs when we are not receiving E1.31 or ArtNet data
void Standby() 
{
  
if(currentProgram > 7) //current program greater than 7
{
currentProgram = 1;  //restart at 1
}

ActualTime = RTC.get(); //Get current time from RTC each time the Standby function is called

 if(hour(ActualTime) >= 2 && hour(ActualTime) <= 13) //hour is between 9PM and 8AM (overnight) EST. We are GMT-5. We want the pixels off. 
  {
    FastLED.clear();
    FastLED.show();
  }
 else //time is between 8AM and 9PM (daytime). We want the pixels to run in standby. 
 { 

   if(ActualTime - previousMinute >= 900) //change the program every 15 minutes. 15 minutes = 900 seconds. 
    {
   currentProgram++; //advance program
   previousMinute = ActualTime; //set to current minute on advance
   random8(random_color2); //choose a random color for the static color function
    }

 switch(currentProgram) //switch case to control programs
    {
     case 1: rainbowFade(); break; 
     case 2: simpleFade(); break; 
     case 3: colorChase(); break; 
     case 4: colorDraw(); break; 
     case 5: Twinkle(); break; 
     case 6: staticWhite(); break;
     case 7: cylon(); break;
    } 
    
  }
}

//Draws a moving rainbow across all pixels
void rainbowFade()
{
//SerialUSB.println("Rainbow Fade");
thishue++;                                                  
fill_rainbow(leds, NUM_LEDS, thishue, deltahue); 
FastLED.show();
} //end rainbow fade

//Fades all pixels up and down and chooses a new color each time 
void simpleFade()
{
//SerialUSB.println("Simple Fade");
random_color = random8(0,255);
random_delay = random8(5,20);
for(brightness = 0; brightness < 255; brightness++)
{
for(ledNumber = 0; ledNumber < NUM_LEDS; ledNumber++)
{
leds[ledNumber] = CHSV(random_color, 255, brightness);
}
FastLED.show(); 
delay(random_delay); 
}

for(brightness = 255; brightness > 0; brightness--)
{
for(ledNumber = 0; ledNumber < NUM_LEDS; ledNumber++)
{
leds[ledNumber] = CHSV(random_color, 255, brightness);
}
FastLED.show(); 
delay(random_delay); 
}
} //end simple fade

//Chases one pixel at a time and chooses a new color when it gets to the end 
void colorChase()
{
//SerialUSB.println("Color Chase");
random_color = random8(0,255);
for(ledNumber = 0; ledNumber < NUM_LEDS; ledNumber++)
{ 
leds[ledNumber] = CHSV( random_color, 255, 255);
FastLED.show();
leds[ledNumber] = CHSV( random_color, 255, 0);
delay(50);
}
random_color = random8(0,255);
for(ledNumber = NUM_LEDS-1; ledNumber > 0; ledNumber--)
{ 
leds[ledNumber] = CHSV( random_color, 255, 255);
FastLED.show();
leds[ledNumber] = CHSV( random_color, 255, 0);
delay(50);
}   
} //end color chase

//Draws one color across all pixels and then draws another on the way back
void colorDraw()
{
//SerialUSB.println("Color Draw");
random_color = random8(0,255);
for(ledNumber = 0; ledNumber < NUM_LEDS; ledNumber++)
{ 
leds[ledNumber] = CHSV( random_color, 255, 255);
FastLED.show();
delay(30);
}
random_color = random8(0,255);
for(ledNumber = NUM_LEDS-1; ledNumber > 0; ledNumber--)
{ 
leds[ledNumber] = CHSV( random_color, 255, 255);
FastLED.show();
delay(30);
}  
} //end color draw

//Simple twinkle function across all pixels with random colors
void Twinkle()
{
//SerialUSB.println("Twinkle");
int i = random16(0,NUM_LEDS); 
random_color = random8(0,255);
if (i < NUM_LEDS) 
{
leds[i] = CHSV(random_color, 255, 255);            
for (int j = 0; j < NUM_LEDS; j++)
{
leds[j].fadeToBlackBy(1);
}
}
FastLED.show();
} //end twinkle

//All pixels white 
void staticWhite()
{
//SerialUSB.println("Static White");
for(ledNumber = 0; ledNumber < NUM_LEDS; ledNumber++)
{ 
leds[ledNumber] = CRGB::White;
}
FastLED.show(); 
} //end static white

void fadeall() 
{ 
  for(int i = 0; i < NUM_LEDS; i++) 
  { 
    leds[i].nscale8(255); 
  } 
}

//Cylon effect across all pixels. 
void cylon() 
{ 
  //SerialUSB.println("Cylon");
  static uint8_t hue = 0;
  // First slide the led in one direction
  for(int i = 0; i < NUM_LEDS; i++) 
  {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show(); 
    // now that we've shown the leds, reset the i'th led to black
    //leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
 
  // Now go in the other direction.  
  for(int i = (NUM_LEDS)-1; i >= 0; i--) 
  {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    //leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
}
