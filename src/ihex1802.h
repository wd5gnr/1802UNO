#include "ihex.h"

class ihex1802 : public ihexread
{
protected:
  int getch(void) ;
  void setmem(uint16_t a, uint8_t d);
};
