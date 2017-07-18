#include "1802.h"
#include "main.h"
#if MONITOR==1
#include <Arduino.h>

/*
Commands:

Note that backspace sort of works, kind of.

R - Display all registers
R2 - Display register 2
R2=AA - Set register 2 to AA

Note: X=10, P=11, D=12, DF=13, Q=14, and T=15 -- display all registers to see that list

M 400 - Display 100 hex bytes at 400
M 400 10 - Display 10 hex bytes at 400
M 400=20 30 40; - Set data starting at 400 (end with semicolon)

G 400 - Goto 400
G 400 3 - Goto 400 with P=3

B - List all enabled breakpoints
B0 - - Disable Breakpoint 0 (that is a dash as in B0 -)
BF @200 - Set breakpoint F to address 200
BF P3 - Set breakpoint when P=3 
BF I7A - Set breakpoint when instruction 7A executes 

Note would be possible to do data write (or even read) breakpoints
Would also be possible to do rnages of addresses, if desired

N - Execute next instruction


I 2 - Show input from N=2
O 2 10 - Write 10 to output N=2

Note: The keypad and display are dead while the monitor is in control

X - Exit to front panel mode (not running)

C - Exit and continue running (if already running)
Q - Same as C

*/


static int cmd;  // command
static uint16_t arg;  // one argument
static int terminate;  // termination character


uint16_t readhex(int *term, uint16_t def=0xFFFF)
{
  uint16_t val=def;
  int first=1;
  int c;
  while (1)
    {
      c=Serial.read();
      if (c==-1) continue;
      if (c==8 && first==0) { val>>=4; continue; }
      c=toupper(c);
      if (!isxdigit(c))
	{
	  if (first && c!='\r' && c!=';' && c!=8 && c!=0x1b) continue;
	  if (term) *term=c;
	  return val;
	}
      if (first) val=0; else val*=16;
      first=0;
      if (c>='A') c=c-'A'+10; else c-='0';
      val+=c;
    }
}

  


BP bp[16];
  
void dispbp(int bpn)
{
  if (bp[bpn].type==0) return;
  Serial.print("\nBP");
  Serial.print(bpn);
  Serial.print(": ");
  if (bp[bpn].type==1) Serial.print("@");
  if (bp[bpn].type==2) Serial.print("P");
  if (bp[bpn].type==3) Serial.print("I");
  print4hex(bp[bpn].target);
}


int nobreak;

void mon_status(void)
{
  print4hex(reg[p]);
  Serial.print(": ");
  print2hex(memread(reg[p]));
  Serial.print(" D=");
  print2hex(d);
  Serial.println();
}



int mon_checkbp(void)
{
  int i;
  int mon=0;
  if (nobreak) return 1;
  for (i=0;i<sizeof(bp)/sizeof(bp[0]);i++)
    {
      if (bp[i].type==0) continue;
      else if (bp[i].type==1 && bp[i].target==reg[p]) mon=1;
      else if (bp[i].type==2 && bp[i].target==p) mon=1;
      else if (bp[i].type==3 && bp[i].target==memread(reg[p])) mon=1;
    }
  if (mon) 
    {
      Serial.println("\nBreak");
      mon_status();
      return monitor();
    }
  return 1;
}


// Need to work on making the line ends consistent and check the backspace and Esc codes
// and new lines too -- mostly done. There are a few wacky backpsace cases left

int waiteol(void)
{
  int c;
  do
    {
      c=Serial.read();

    } while (c!='\r' && c!=0x1b);
  return c;
}



int monitor(void)
{
  while (1)
    {
      Serial.print("\r\n>");
      do { cmd=Serial.read(); 
      } while (cmd==-1);
      cmd=toupper(cmd);
      if (!strchr("RMGBIOXQCN?",cmd))
	{
	  Serial.print('?');
	  continue;
	}
      arg=readhex(&terminate);
      if (terminate==0x1b|| terminate==8) continue;
      switch(cmd)
	{
	case '?':
	  Serial.println(F("<R>egister, <M>emory, <G>o, <B>reakpoint, <N>ext, <I>nput, <O>utput, e<X>it, <Q>uit, <C>ontinue"));
	  Serial.print(F("Examples: R   RB   RB=2F00   M 100 10    M 100=AA 55 22; B 0 @101"));
	  break;

	case 'N':
	  nobreak=1;
	  mon_status();
	  run();
	  nobreak=0;
	  break;
	  
	case 'B':
	if (arg>=0x10)
	  {
	    int i;
	    for (i=0;i<sizeof(bp)/sizeof(bp[0]);i++) dispbp(i);
	  }
	if (terminate!='\r')
	  {
	    int cc;
	    do 
	      {
		cc=Serial.read();
	      } while (cc==-1||cc==' '||cc=='\t');
	    if (cc==0x1b) break;
	    if (cc=='\r') terminate='\r';
	    else if (cc=='-') { if (waiteol()=='\r') bp[arg].type=0;   break;    }
	    
	    if (cc=='@')
	      {
		cc=readhex(&terminate);
		if (terminate==0x1b) break;
		bp[arg].target=cc;
		bp[arg].type=1;
		break;
	      }
	    if (cc=='p'||cc=='P')
	      {
		cc=readhex(&terminate);
		if (terminate==0x1b) break;
		bp[arg].target=cc&0xF;
		bp[arg].type=2;
		break;
	      }
	    if (cc=='i'||cc=='I')
	      {
		cc=readhex(&terminate);
		if (terminate==0x1b) break;
		bp[arg].target=cc&0xFF;
		bp[arg].type=3;
		break;
	      }
	    
		
	  }
	if (terminate=='\r')
	  {
	    dispbp(arg);
	  }
	break;
	
	
	case 'R':
	  if (arg==0xFFFF)
	    {
	      int i;
	      for (i=0;i<=15;i+=2)
		{
		  Serial.print("R");
		  Serial.print(i,HEX);
		  Serial.print(':');
		  print4hex(reg[i]);
		  Serial.print("\tR");
		  Serial.print(i+1,HEX);
		  Serial.print(':');
		  print4hex(reg[i+1]);
		  Serial.println();
		}
	      Serial.print("(10) X:");
	      Serial.print(x,HEX);
	      Serial.print("\t(11) P:");
	      Serial.println(p,HEX);
	      Serial.print("(12) D:");
	      print2hex(d);
	      Serial.print("\t(13) DF:");
	      Serial.println(df,HEX);
	      Serial.print("(14) Q:");
	      Serial.print(q,HEX);	      
	      Serial.print("\t(15) T:");
	      Serial.print(t,HEX);	      
	    }
	  else
	    {
	      if (terminate!='=') Serial.print("R");
	      if (arg<=0xF)
		{
		  if (terminate=='=')
		    {
		      uint16_t v=readhex(&terminate);
		      if (terminate==0x1b) break;
		      reg[arg]=v;
		    }
		  else
		    {
		      Serial.print(arg,HEX);
		      Serial.print(':');
		      print4hex(reg[arg]);
		    }
		}
	      else
		{
		  switch (arg)
		    {
		    case 0x10:
		      if (terminate=='=')
			{
			  uint16_t v=readhex(&terminate);
			  if (terminate==0x1b) break;
			  x=v;
			}
		      else
			{
			  Serial.print("X:");
			  Serial.print(x,HEX);
			}

		      break;
		      
		    case 0x11:
		      if (terminate=='=')
			{
			  uint16_t v=readhex(&terminate);
			  if (terminate==0x1b) break;
			  p=v;
			}
		      else
			{
			  Serial.print("P:");
			  Serial.print(p,HEX);
			}
		      
		      break;
		       
		    case 0x12:
		      if (terminate=='=')
			{
			  uint16_t v=readhex(&terminate);
			  if (terminate==0x1b) break;
			  d=v;
			}
		      else
			{
			  Serial.print("D:");
			  Serial.print(d,HEX);
			}
		      
		      break;
		      
		    case 0x13:
		      if (terminate=='=')
			{
			  uint16_t v=readhex(&terminate);
			  if (terminate==0x1b) break;
			  df=v;
			}
		      else
			{
			  Serial.print("DF:");
			  Serial.print(df,HEX);
			}
		      
		      break;
		      
		    case 0x14:
		      if (terminate=='=')
			{
			  uint16_t v=readhex(&terminate);
			  if (terminate==0x1b) break;
			  q=v;
			}
		      else
			{
			  Serial.print("Q:");
			  Serial.print(q,HEX);
			}
		      
		    case 0x15:
		      if (terminate=='=')
			{
			  uint16_t v=readhex(&terminate);
			  if (terminate==0x1b) break;
			  t=v;
			}
		      else
			{
			  Serial.print("T:");
			  Serial.print(t,HEX);
			}
		      
		      break;
		    }
		  
		}
	    }
	  
	      break;
		  
	case 'Q': { runstate=0; return 0; }
	case 'C': case 'X': return 1;
	case 'I':
	  print2hex(input(arg));
	  break;

	case 'O':
	  {
	    uint8_t v=readhex(&terminate);
	    if (terminate!=0x1b) output(arg,v);
	    break;
	  }
	  
	case 'G':
	  {

	    if (terminate!='\r') p=readhex(&terminate);
	    if (terminate==0x1b) break;
	    reg[p]=arg;
	    runstate=1;
	    return 0;

	  }
	  
	case 'M':
	  {
	    uint16_t arg2=0;
	    if (terminate=='=')
	      {
		uint8_t d;
		do
		  {
		    d=readhex(&terminate);
		    if (terminate==0x1b) break;
		    memwrite(arg++,d);
		  } while (terminate!=';');
	      }
	    else
	      {
		uint16_t i;
		unsigned ct=16;
		if (terminate!='\r') arg2=readhex(&terminate,0);
		if (terminate==0x1b) break;
		if (arg2==0) arg2=0x100;
		for (i=arg;i<arg+arg2;i++)
		  {
		    if (ct%16==0)
		      {
			Serial.println();
			print4hex(i);
			Serial.print(": ");
		      }
		    else if (ct%8==0) Serial.print(' ');
		    ct++;
		    
		    print2hex(memread(i));
		    Serial.print(' ');
		    if (Serial.read()==0x1b) break;
		  }
	      }
	    
	  }
	  break;
	  
	  
	default:
	  Serial.print((char)cmd);
	  Serial.println("?");
	}
    }
}


      
#endif
