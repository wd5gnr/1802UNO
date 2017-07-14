#ifndef __IHEXOUT_H
#define __IHEXOUT_H

#include <stdint.h>

class ihexout
{
 public:
  int write(uint8_t *bytes,uint16_t count,uint16_t base=0);
 protected:
  virtual int putch(int c)=0;
  virtual int crlf(void)  { if (putch('\r')==-1) return -1; return putch('\n');  }
  uint8_t digit(uint8_t d);
};


#endif
