#include <Arduino.h>
#include "1802.h"
#include "main.h" // need serialread
// Input from any port gives you the data register
// except port 1 is serial input
uint8_t input(uint8_t port)
{
  if (port==SER_INP)
  {
    int rv=Serialread();
    if (rv==-1) rv=0;
    return rv;
  }
  return idata;
}

// Output to any port writes to the data display
void output(uint8_t port, uint8_t val)
{
  if (port==SER_OUT) Serial.print((char)val);
  else if (port==LED_PORT) data=val;
  else if (port==A0_PORT) adlow=val;
  else if (port==A1_PORT) adhigh=val;
  else if (port==CTL_PORT)
  {
    noserial=val&1;  // set 1 to use serial port as I/O, else use as front panel
    addisp=(val&2)==2; // set bit 1 to 1 to use address displays
  }
}
