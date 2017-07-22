#include <Arduino.h>
#include "main.h"
#include "1802.h"
#if BIOS==1


int bios(uint16_t fn)
{
  switch (fn)
    {
    case 0xFF01:  // made up SCALL
      {
	uint16_t cv;
	cv=memread(reg[3])<<8;
	cv|=memread(reg[3]+1);
	x=2;
	memwrite(reg[2],reg[6]);
	memwrite(reg[2]-1,reg[6]>>8);
	reg[2]-=2;
	reg[6]=reg[3]+2;
	reg[3]=cv;
	p=3;
	reg[4]=0xFF01;  // reset for next call
      }
      
      break;
      
    case  0xFF02: //  made us RET
      reg[2]++;
      reg[3]=reg[6];
      reg[6]=memread(reg[2])<<8;
      reg[6]|=memread(reg[2]+1);
      reg[2]++;
      p=3;
      reg[5]=0xFF02;
      break;

    case 0xFF3F:   // set up SCRT
      reg[4]=0xFF01;
      reg[5]=0xFF02;
      reg[3]=reg[6];  // assume this wasn't a proper call because we were not set up yet...
      p=3;
      break;

    case 0xFF2d:    // Baud rate, not needed here
	break;
      
    case 0xFF66:   // print a string
      {
	char c;
	do
	  {
	    c=memread(reg[6]++);
	    if (c) Serial.print(c);
	  } while (c);
	p=5;
      }
      break;

    default:
        return monitor();  // for now assume BIOS and MONITOR are both set
    }
  
  return 1;
	
}


#endif
