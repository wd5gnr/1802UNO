#include <Arduino.h>
#include "ihex1802.h"
#include "1802.h"

// Intel hex stuff

int ihex1802::getch(void)
{
  int c;
  while ((c=Serialread())==-1);
  return c;
}

void ihex1802::setmem(uint16_t a, uint8_t d)
{
  ram[a&MAXMEM]=d;
}

int ihexo1802::putch(int c)
{
  Serial.print((char)c);
  return 0;
}
