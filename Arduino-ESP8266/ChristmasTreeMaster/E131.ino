void E131Received(unsigned char* pbuff, int count) 
{
    if (pbuff[11] == E131_START_CODE && pbuff[1] == 114 && pbuff[3] == 2) //data check to throw out bad data
    { 
      
      if(pbuff[0] == 1) //byte 0 is universe number
      { 
      Serial.println("Universe 1");
      Channel = 1; //reset channel assignment to 1 each time through loop.
      for(int i = 0; i < 171; i++) //pixels 0-170
      {
      leds[i] = CRGB(pbuff[E131_ADDRESS_OFFSET + Channel], pbuff[E131_ADDRESS_OFFSET + (Channel +1)], pbuff[E131_ADDRESS_OFFSET + (Channel + 2)]);
      Channel += ChannelWidth; //increase last channel number by channel width
      }
      }
      
      if(pbuff[0] == 2)
      {
      Serial.println("Universe 2"); 
      Channel = 1; //reset channel assignment to 1 each time through loop.
      for(int i = 170; i < 341; i++) //pixels 170-340
      {
      leds[i] = CRGB(pbuff[E131_ADDRESS_OFFSET + Channel], pbuff[E131_ADDRESS_OFFSET + (Channel +1)], pbuff[E131_ADDRESS_OFFSET + (Channel + 2)]);
      Channel += ChannelWidth; //increase last channel number by channel width
      }  
      }
      
      if(pbuff[0] == 3) 
      {
      Serial.println("Universe 3"); 
      Channel = 1; //reset channel assignment to 1 each time through loop.
      for(int i = 340; i < 372; i++) //pixels 340-371
      {
      leds[i] = CRGB(pbuff[E131_ADDRESS_OFFSET + Channel], pbuff[E131_ADDRESS_OFFSET + (Channel +1)], pbuff[E131_ADDRESS_OFFSET + (Channel + 2)]);
      Channel += ChannelWidth; //increase last channel number by channel width
      }
      FastLED.show(); //Send data to pixels after all three universes received
      ears.set_colors(pbuff[E131_GWTS_START],pbuff[E131_GWTS_START +1],pbuff[E131_GWTS_START +2],pbuff[E131_GWTS_START +3],pbuff[E131_GWTS_START +4],pbuff[E131_GWTS_START +5]);
      }
       
 } //end if
} //end function 
