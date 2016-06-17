int artNetOpCode(unsigned char* pbuff) 
{
  String test = String((char*)pbuff);
  if ( test.equals("Art-Net") ) 
  {
    if ( pbuff[11] >= 14 ) 
    {            //protocol version [10] hi byte [11] lo byte
      return pbuff[9] *256 + pbuff[8];  //opcode lo byte first
    }
  }
  
  return 0;
}

/* Channel values are bytes 12-523 */
void artDMXReceived(unsigned char* pbuff) 
{
      /* Universe 1 */
      if(pbuff[8] == 0) //byte 8 in the packet contains the universe number 
      {  
      Channel = 1; //reset channel assignment to 1 each time through loop.
      for(int i = 0; i < 171; i++) //pixels 0-170
      {
      leds[i] = CRGB(pbuff[ARTNET_ADDRESS_OFFSET + Channel], pbuff[ARTNET_ADDRESS_OFFSET + (Channel +1)], pbuff[ARTNET_ADDRESS_OFFSET + (Channel + 2)]);
      Channel += ChannelWidth; //increase last channel number by channel width
      }
      FastLED.show(); 
      }

      /* Universe 2 */
      if(pbuff[8] == 1)
      {  
      Channel = 1; //reset channel assignment to 1 each time through loop.
      for(int i = 170; i < 341; i++) //pixels 170-340
      {
      leds[i] = CRGB(pbuff[ARTNET_ADDRESS_OFFSET + Channel], pbuff[ARTNET_ADDRESS_OFFSET + (Channel +1)], pbuff[ARTNET_ADDRESS_OFFSET + (Channel + 2)]);
      Channel += ChannelWidth; //increase last channel number by channel width
      } 
      }

      /* Universe 3 */
      if(pbuff[8] == 2)
      {
      Channel = 1; //reset channel assignment to 1 each time through loop.
      for(int i = 340; i < 372; i++) //pixels 340-371
      {
      leds[i] = CRGB(pbuff[ARTNET_ADDRESS_OFFSET + Channel], pbuff[ARTNET_ADDRESS_OFFSET + (Channel +1)], pbuff[ARTNET_ADDRESS_OFFSET + (Channel + 2)]);
      Channel += ChannelWidth; //increase last channel number by channel width
      } 
      
      }
      
} //end function 
