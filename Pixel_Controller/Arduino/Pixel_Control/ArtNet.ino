void artDMXReceived() //only one universe 
{
      Channel = 1; //reset channel assignment to 1 each time through loop.
      for(int i = 0; i < 170; i++) //set values for 170 pixels. Max number per universe. 
      {
      leds[i] = CRGB(DataBuffer[ARTNET_ADDRESS_OFFSET + Channel], DataBuffer[ARTNET_ADDRESS_OFFSET + (Channel +1)], DataBuffer[ARTNET_ADDRESS_OFFSET + (Channel + 2)]);
      Channel += ChannelWidth; //increase last channel number by channel width
      }
      FastLED.show(); //Send data to pixels after all three universes received
      Serial.print("FastLED FPS: ");
      Serial.println(FastLED.getFPS()); //prints frame rate to serial port 
} //end function 

