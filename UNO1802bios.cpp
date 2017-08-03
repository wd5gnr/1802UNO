#include <Arduino.h>
#include "main.h"
#include "UNO1802.h"


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

    case 0xFF03:
    case 0xFF43:
      {
	char c=d;
	if (c==0xC && fn==0xFF03) c=0x1B;  // no idea why this is true
	Serial.print(c);
	p=5;
      }
      break;

    case 0xFF06:
      {
	int c;
	do 
	  {

	    c=Serial.read();
	  } while (c==-1);
	d=c;
	p=5;
      }
      break;


    case 0xFF09:
      {
	char c;
	do
	  {
	    c=memread(reg[0xf]++);
	    if (c) Serial.print(c);
	  } while (c);
	p=5;
      }
      break;

    case 0xFF81:
      reg[0xf]=8;
      p=5;
      break;
      
    case 0xFF0F:
    case 0xFF69:
      {
	int c;
	int n=0;
	int l=254;
	uint16_t ptr=reg[0xF];
	if (fn==0xFF69) l=reg[0xc]-1;
	do
	  {
	    do 
	      {
		c=Serial.read();
	      } while (c==-1||c==0);
	    if (c==0xD) { c=0; df=0; }
	    if (c==3)  { c=0; df=1; }
	    if (c==8)  
	      {
		if (n) 
		  {
		    ptr--;
		    n--;
		  }
		continue;
	      }
	    if (c && n==l) continue;
	    memwrite(ptr++,c);
	  } while (c!=0);
	
	p=5;
      }
      
      break;
	
// NOTE MUST REDO THIS ONE... DOES NOT WORK IN ROM!
      case 0xFF12: 
	{
	  char *p1=ram+reg[0xf];
	  char *p2=ram+reg[0xd];
	  d=strcmp(p1,p2);
	  p=5;
	}
	
	break;

	
	case 0xFF15:
	  {
	    char c;
	    do
	      {
		c=memread(reg[0xf]++);
	      } while (c && isspace(c));
	    reg[0xf]--;
	    p=5;
	  }
	  break;

      case 0xFF18:
	{
	  char c;
	  do
	    {
	      c=memread(reg[0xf]++);
	      memwrite(reg[0xd]++,c);
	    } while (c);
	}
	p=5;
	
	break;

      case 0xFF1B:
	while (reg[0xC]--)
	  memwrite(reg[0xd]++,reg[0xF]++);
	p=5;
	break;

      case 0xFF54:  // not sure this really works as expected
	 monitor();
	 p=5;
	 break;
	 

      case 0xFF57:
	reg[0xf]=0x3FF;
	p=5;
	break;

	
    default:
        return monitor();  // for now assume BIOS and MONITOR are both set
    }
  
  return 1;
	
}


#endif
