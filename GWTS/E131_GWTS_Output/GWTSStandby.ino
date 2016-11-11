//Pulse IR 
void pulseIR(long microsecs, int hilo) 
{
  while (microsecs > 0) 
  {
    digitalWrite(IRledPin, hilo);  // this takes about 5 microseconds to happen
    delayMicroseconds(13);         // hang out for 8 microseconds
    digitalWrite(IRledPin, LOW);   // this also takes about 5 microseconds
    delayMicroseconds(12);         // hang out for 8 microseconds
    microsecs -= 26;
  }

}
//Send Byte 
void sendbyte(byte b)
{
  pulseIR(400, HIGH); // Start bit
  byte i=0;
  while(i<8)
  {
    pulseIR(400, (b>>(i++)&1)?LOW:HIGH); // Data Bits
  }
  pulseIR(400, LOW); // Stop bit
}

//Calculate Checksum
unsigned char calc_crc(unsigned char *data, unsigned char length)
{
  // Courtesy of rjchu and Timon - A godsend
  unsigned char crc = 0;
  while(length--) 
  {
    crc ^= *data++;
    unsigned n = 8; 
    do crc = (crc & 1) ? (crc >> 1) ^ 0x8C : crc >> 1; 
    while(--n);
  }
  return crc;
}

void powerOnReset()
{
  sendbyte(0x92);
  sendbyte(0x20);
  sendbyte(0x48);
  sendbyte(0x80);
  sendbyte(0x13);
}

void fadedown()
{
  //fade up - 94 24 48 85 58 XX(speed variable) XX(Checksum)
  cmdbuf[0] = 0x94;
  cmdbuf[1] = 0x24;
  cmdbuf[2] = 0x48;
  cmdbuf[3] = 0x85;
  cmdbuf[4] = 0x58;
  cmdbuf[5] = E131Buffer[133]; //channel 8
  checksum = calc_crc(cmdbuf, 6);
  sendbyte(cmdbuf[0]);
  sendbyte(cmdbuf[1]);
  sendbyte(cmdbuf[2]);
  sendbyte(cmdbuf[3]);
  sendbyte(cmdbuf[4]);
  sendbyte(E131Buffer[133]);
  sendbyte(checksum);
  Serial.println(checksum);
}

void fadeup()
{
  //fade up - 94 24 48 86 58 XX(speed variable) XX(Checksum)
  cmdbuf[0] = 0x94;
  cmdbuf[1] = 0x24;
  cmdbuf[2] = 0x48;
  cmdbuf[3] = 0x86;
  cmdbuf[4] = 0x58;
  cmdbuf[5] = E131Buffer[133]; //channel 8
  checksum = calc_crc(cmdbuf, 6);
  sendbyte(cmdbuf[0]);
  sendbyte(cmdbuf[1]);
  sendbyte(cmdbuf[2]);
  sendbyte(cmdbuf[3]);
  sendbyte(cmdbuf[4]);
  sendbyte(E131Buffer[133]);
  sendbyte(checksum);
  Serial.println(checksum);
}







