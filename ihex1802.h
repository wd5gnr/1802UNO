#include "ihex.h"
#include "ihexout.h"

class ihex1802 : public ihexread
{
protected:
  int getch(void) ;
  void setmem(uint16_t a, uint8_t d);
};

class ihexo1802 : public ihexout
{
  int putch(int c);
};


  
