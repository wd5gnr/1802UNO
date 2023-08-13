#ifndef __IHEX_H
#define __IHEX_H
#include <stdint.h>
#include "main.h" // need Serialread

class ihexread
{
public:
  int read(void);
protected:
  virtual int getch(void)=0;
  virtual void setmem(uint16_t add, uint8_t data)=0;
  virtual int error(void)  { return -1; }
private:
  int hex1(void);
  int hex2(void);
  static uint8_t cksumfin(uint8_t cksum);
};

#endif

