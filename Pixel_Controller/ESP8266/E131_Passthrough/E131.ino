//Simple check to verify data is E1.31. 
int checkE131Headers(unsigned char* messagein, int messagelength) 
{
  if ( messagein[1] == 0x10 && messagein[4] == 0x41 && messagein[12] == 0x37) 
  {  
      int ChannelCount = messagein[123] * 256 + messagein[124]; // number of values plus start code
      return ChannelCount -1; //Return how many values are in the packet.
  }
  return 0;
}
