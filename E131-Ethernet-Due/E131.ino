//checks to see if packet is E1.31 data
int checkACNHeaders(unsigned char* messagein, int messagelength) 
{
  if ( messagein[1] == 0x10 && messagein[4] == 0x41 && messagein[12] == 0x37) 
  {  
      int addresscount = messagein[123] * 256 + messagein[124]; // number of values plus start code
      return addresscount -1; //Return how many values are in the packet.
    }
  return 0;
}

void E131Received(unsigned char* pbuff, int count) 
{
  //Serial.println(pbuff[114]);
      if(pbuff[114] == 1) //byte 0 is universe number
      {
      Channel = 1; //reset channel assignment to 1 each time through loop.
      for(int i = 0; i < 170; i++) //pixels 0-170
      {
      leds[i] = CRGB(pbuff[E131_ADDRESS_OFFSET + Channel], pbuff[E131_ADDRESS_OFFSET + (Channel +1)], pbuff[E131_ADDRESS_OFFSET + (Channel + 2)]);
      Channel += ChannelWidth; //increase last channel number by channel width
      }
      }
      
      if(pbuff[114] == 2)
      {
      Channel = 1; //reset channel assignment to 1 each time through loop.
      for(int i = 170; i < 341; i++) //pixels 170-340
      {
      leds[i] = CRGB(pbuff[E131_ADDRESS_OFFSET + Channel], pbuff[E131_ADDRESS_OFFSET + (Channel +1)], pbuff[E131_ADDRESS_OFFSET + (Channel + 2)]);
      Channel += ChannelWidth; //increase last channel number by channel width
      } 
      }
      
      if(pbuff[114] == 3) 
      {
      Channel = 1; //reset channel assignment to 1 each time through loop.
      for(int i = 340; i < NUM_LEDS; i++) //pixels 340-500
      {
      leds[i] = CRGB(pbuff[E131_ADDRESS_OFFSET + Channel], pbuff[E131_ADDRESS_OFFSET + (Channel +1)], pbuff[E131_ADDRESS_OFFSET + (Channel + 2)]);
      Channel += ChannelWidth; //increase last channel number by channel width
      }
      FastLED.show(); //Send data to pixels after all three universes received
      Serial.print("FastLED FPS: ");
      Serial.println(FastLED.getFPS());
      }
} //end function 
