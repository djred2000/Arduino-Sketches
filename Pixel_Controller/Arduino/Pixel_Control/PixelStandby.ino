//This function runs when we are not receiving E1.31
void Standby()
{ 

if(millis() - previousTime > ChangeTime)
  {
   //Serial.print("Current program: ");
   //Serial.println(currentProgram); 
   currentProgram++; //advance program
   previousTime = millis(); //store the new time
   random8(random_color2); //choose a random color for the static color function
   //Serial.print("Change program to: ");
   //Serial.println(currentProgram);
  }
  
if(currentProgram > 9) //current program greater than 9
{
currentProgram = 1;  //restart at 1
}

 switch(currentProgram) //switch case to control programs
    {
     case 1: rainbowFade(); break;
     case 2: alternating(); break;
     case 3: everyother(); break;
     case 4: simpleFade(); break;
     case 5: colorDraw(); break;
     case 6: ColorStep(); break;
     case 7: Twinkle(); break;
     case 8: staticColor(); break;
     case 9: cylon(); break;  
     case 10: OffMode(); break;
    } 
} //end standby 

//Draws a moving rainbow across all pixels
void rainbowFade()
{
thishue++;                                                  
fill_rainbow(leds, NUM_LEDS, thishue, deltahue); 
FastLED.delay(10);
FastLED.show();
} //end rainbow fade

//Alternates all pixels between 2 colors
void alternating()
{
  FastLED.showColor(CRGB::Red);
  delay(500);
  FastLED.showColor(CRGB::Green);
  delay(500);
} //end alternating

//Chase type effect alternating 2 colors across all pixels 
void everyother()
{
for(int i = 0; i < NUM_LEDS; i+=2)
{
  leds[i] = CRGB(255,0,0);
}
for(int i = 1; i < NUM_LEDS; i+=2)
{
  leds[i] = CRGB(0,0,255);
}
FastLED.show();
delay(500);
for(int i = 1; i < NUM_LEDS; i+=2)
{
  leds[i] = CRGB(255,0,0);
}
for(int i = 0; i < NUM_LEDS; i+=2)
{
  leds[i] = CRGB(0,0,255);
}
FastLED.show();
delay(500);
} //end everyother 

//Fades all pixels up and down and chooses a new color each time 
void simpleFade()
{
random_color = random8(0,255);
random_delay = random8(5,20);
for(brightness = 0; brightness < 255; brightness++)
{
for(ledNumber = 0; ledNumber < NUM_LEDS; ledNumber++)
{
leds[ledNumber] = CHSV(random_color, 255, brightness);
}
FastLED.show();
FastLED.delay(random_delay); 
}

for(brightness = 255; brightness > 0; brightness--)
{
for(ledNumber = 0; ledNumber < NUM_LEDS; ledNumber++)
{
leds[ledNumber] = CHSV(random_color, 255, brightness);
}
FastLED.show();
FastLED.delay(random_delay); 
}
} //end simple fade

//Draws one color across all pixels and then draws another on the way back
void colorDraw()
{
random_color = random8(0,255);
for(ledNumber = 0; ledNumber < NUM_LEDS; ledNumber++)
{ 
leds[ledNumber] = CHSV( random_color, 255, 255);
FastLED.show();
FastLED.delay(15);
}
random_color = random8(0,255);
for(ledNumber = NUM_LEDS-1; ledNumber > 0; ledNumber--)
{ 
leds[ledNumber] = CHSV( random_color, 255, 255);
FastLED.show();
FastLED.delay(15);
}  
} //end color draw

void ColorStep()
{
  for(int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CHSV(color1,255,255);
  }
  FastLED.show();
  FastLED.delay(500);
  color1+=25;
}

//Simple twinkle function across all pixels with random colors
void Twinkle()
{
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
void staticColor()
{
for(ledNumber = 0; ledNumber < NUM_LEDS; ledNumber++)
{ 
leds[ledNumber] = CHSV(random_color2, 255, 255);
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
    FastLED.delay(10);
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
    FastLED.delay(10);
  }
}

void OffMode()
{
  FastLED.clear();
  FastLED.show();
  delay(5000);
  //Serial.println("Off");
}

