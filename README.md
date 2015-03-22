# Arduino-Sketches
These sketches are for controlling RGB Pixels and other devices using the Artnet or E1.31 protocols.  

The PIXEL CONTROL COMPLETE sketch allows control of RGB lights that work with the FastLED library. I am using WS2811 12 volt pixels. The lights can be controlled via ArtNet or E1.31 over ethernet. The sketch works best on an Arduino Mega 2560 and requires the Ethernet Shield. I also included control of Disney Glow with Show Ears. 

The RELAY CONTROL COMPLETE sketch allows control of relays connected to digital pins on an Arduino using ArtNet or E1.31 protocols. The sketch works best using an Arduino Mega 2560 and will require either the ethernet shield or the WiFi shield. I am using the Sainsmart 8-channel relay board. 
