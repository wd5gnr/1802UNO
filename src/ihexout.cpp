#include "ihexout.h"

uint8_t ihexout::digit(uint8_t d)
{
  d&=0xF;// just in case
  if (d<=9) d+='0'; else d+='A'-10;
  return d;
}

     
#define safe_putch(c) if (putch(c)) return -1

int ihexout::write(uint8_t *bytes, uint16_t count, uint16_t base)
{
  uint16_t line;
  uint8_t cksum;
  
  int i;
   
  if (count==0 || bytes==(uint8_t*)0) return -1;
  while (count)
    {
      safe_putch(':');
      cksum=0;
      if (count>=16)
	{
	  safe_putch('1');
	  safe_putch('0');
	  line=16;
	  cksum+=16;
	} else
	{
	  safe_putch(digit(count>>4));
	  safe_putch(digit(count));
	  line=count;
	  cksum+=count;
	}
      safe_putch(digit(base>>12));
      safe_putch(digit(base>>8));
      cksum+=base>>8;
      safe_putch(digit(base>>4));
      safe_putch(digit(base));
      cksum+=base;
      safe_putch('0');
      safe_putch('0');
      for (i=0;i<line;i++)
	{
	  safe_putch(digit((*bytes)>>4));
	  safe_putch(digit(*bytes));
	  cksum+=(*bytes++);
	  base++;
	}
      cksum=~cksum;
      cksum++;
      safe_putch(digit(cksum>>4));
      safe_putch(digit(cksum));
      if (crlf()) return -1;
      count-=line;
    }
  safe_putch(':');
  safe_putch('0');
  safe_putch('0');
  safe_putch('0');
  safe_putch('0');
  safe_putch('0');
  safe_putch('0');
  safe_putch('0');  
  safe_putch('1');  
  safe_putch('F');
  safe_putch('F');
  return crlf();
}


#if 0
#include <stdio.h>

class ihexouttest : public ihexout
{
protected:
  int putch(int c);
};

int ihexouttest::putch(int c)
{
  putchar(c);
  return 0;
}

uint8_t mem[100]=
  {
    0xAA, 0x55
  };

  

    

int main(int argc, char *argv[])
{
  ihexouttest test;
  int i;
  for (i=0;i<45;i++) mem[i+2]=i;
  test.write(mem,45,0);
  return 0;
}

#endif
   
  
