/*  artNetOpCode checks to see that the packet is actually Art-Net
    and returns the opcode telling what kind of Art-Net message it is.  */

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


