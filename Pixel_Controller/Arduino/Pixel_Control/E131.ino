void E131Received() 
{
  if(DataBuffer[114] >= E131StartUniverse && DataBuffer[114] <= E131UniverseCount) //first make sure the universe received is one we want to process
  {

  if(DataBuffer[114] == E131StartUniverse) //if current universe is the first in the series, reset all values
  {
    StartLED = 0;
    EndLED = NumLEDsPerUniverse;
    E131CurrentUniverse = E131StartUniverse;
  }
  
if(DataBuffer[114] == E131CurrentUniverse)
{
      Channel = 1; //reset channel assignment to 1 each time through loop.
      for(int i = StartLED; i < EndLED; i++) //set values for 170 pixels. Max number per universe. 
      {
      leds[i] = CRGB(DataBuffer[E131_ADDRESS_OFFSET + Channel], DataBuffer[E131_ADDRESS_OFFSET + (Channel +1)], DataBuffer[E131_ADDRESS_OFFSET + (Channel + 2)]);
      Channel += ChannelWidth; //increase last channel number by channel width
      }
}    
      if(DataBuffer[114] == E131UniverseCount) //we've processed all universes now send data to pixels 
      {
      LEDS.show();
      Serial.print("FastLED FPS: ");
      Serial.println(FastLED.getFPS()); //print frame rate
      }
      else //not all universes have been received. Move on to the next one. 
      {
        StartLED += NumLEDsPerUniverse; //increase by 170
        EndLED += NumLEDsPerUniverse; //increase by 170
        E131CurrentUniverse++; //increase by 1 
      }
}
} //end function 
