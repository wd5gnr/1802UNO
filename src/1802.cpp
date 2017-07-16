// 1802 Simulation for KIM-UNO hardware
// Al Williams July 2017
#include <Arduino.h>
#include <stdint.h>
#include <avr/pgmspace.h>  // note: AVR only!

#include "1802.h"
#include "main.h"
#include "ihex1802.h"



int addisp=0;
uint8_t adhigh=0;  // high address display for I/O
uint8_t adlow=0;  // low address display for I/O



unsigned int caddress;  // current load address
unsigned int data;      // data
unsigned int idata;

int noserial=0;

// CPU registers and stuff
uint16_t reg[16];
uint8_t p,x,d,df,q,t;
uint8_t I,N;
uint16_t work;
uint8_t ef1, ef2, ef3, ef4;
uint8_t mp=0;  // memory protect

// RAM
// Note: Hardcode a jump to ROM but don't ever set it again
// So on power up, we go to ROM. If you mess that up, that's on you ;)
uint8_t ram[MAXMEM+1]={0xC0, 0x80, 00}; 		// main 1KB RAM		 0x000-0x3FF
// ROM
#include <1802rom.h>

uint16_t rombase=0x8000;

uint8_t memread(uint16_t a)
{
  if (a<rombase)
    return ram[a&MAXMEM];
  else
    return pgm_read_byte_near(rom+(a-rombase));
}



// reset CPU
void reset()
{
  p=0;
  //d=0; -- D not initilized on real CPU
  df=0;
  x=0;
  q=0;
  reg[0]=0;
  setaddress(0);
  setdata(0);
  addstate=runstate=loadstate=0;
  caddress=0;
  addisp=0;
}





// Emulation engine
int run(void)
{
uint8_t inst=memread(reg[p]);
#if 0
 if ((reg[p]&0xFF00)==0xFF00)
  {
  Serial.print("BIOS: ");
  print4hex(reg[p]);
  Serial.println();
  inst=0;
  }

#endif
  reg[p]++;
  I=inst>>4;
  N=inst&0xF;
  if (tracemode)
  {
    print4hex(reg[p]-1);
    Serial.print(':');
    print2hex(inst);
    Serial.print(' ');
    print2hex(d);
    Serial.println("");
  }
  if (inst==0)  // op code 00 causes simulation to stop
  { // IDL
    return 0;
  }
  switch (I)
  {
    case 0:
      d=memread(reg[N]);
    break;
    case 1:
    reg[N]++;
    break;
    case 2:
    reg[N]--;
    break;
    case 3:
    // handle branches
    {
      uint16_t tpc=memread(reg[p]);
      uint16_t nxt=++reg[p];
      switch (N)
      {
        case 0:
        nxt=tpc;
        break;
        case 1:
        case 9:
        if (q!=((N&8)?1:0)) nxt=tpc;
        break;
        case 2:
        if (d==0) nxt=tpc;
        break;
        case 3:
        case 11:
        if (df!=((N&8)?1:0)) nxt=tpc;
        break;
        case 4:
        case 12:
        if (ef1!=((N&8)?1:0)) nxt=tpc;
        break;
        case 5:
        case 13:
        if (ef2!=((N&8)?1:0)) nxt=tpc;
        break;
        case 6:
        case 14:
        if (ef3!=((N&8)?1:0)) nxt=tpc;
        break;
        case 7:
        case 15:
        if (ef4!=((N&8)?1:0)) nxt=tpc;
        break;
        case 8:
        break;  //does this skip? I don't think so
        case 10:
        if (d!=0) nxt=tpc;
        break;
      }
      reg[p]=(reg[p]&0xFF00)|nxt;
      break;
    }

    case 4:
      d=memread(reg[N]);
    reg[inst&0x0f]++;
    break;
    case 5:
    if (mp==0 && reg[N]<rombase) ram[reg[N]&MAXMEM]=d;
    break;
    case 6:
    // IRX + I/O
      if (N==0) reg[x]++;
      else
      if (N==8) ;   // ?
      else if (N<8)
      {
        output(N,memread(reg[x]));
        reg[x]++;
      }
      else
      {
        d=input(N-8);  // port 1-7
        if (mp==0 && reg[x]<rombase) ram[reg[x]&MAXMEM]=d;
      }

    break;
    case 7:  // misc
    switch (N)
    {
      case 0:
      case 1:  // ths same since we don't manage IE
	work=memread(reg[x]);
      x=work>>4;
      p=work&0xF;
      reg[x]++;
      break;
      case 2:
	d=memread(reg[x]);
      reg[x]++;
      break;
      case 3:
      if (mp==0 && reg[x]<rombase) ram[reg[x]&MAXMEM]=d;
      reg[x]--;
      break;
      case 4:
	work=memread(reg[x])+d+df;
      if (work&0x100) df=1; else df=0;
      d=work;  // conversion will chop off top for us
      break;
      case 5:
	work=memread(reg[x])-d-(df?0:1);
      if (work&0x100) df=0; else df=1;
      d=work;
      break;
      case 6:
      work=d;
      if (df) work|=0x100;
      work>>=1;
      df=d&1;
      d=work;
      break;
      case 7:
	work=d-memread(reg[x])-(df?0:1);
      if (work&0x100) df=0; else df=1;
      d=work;
      break;
      case 8:
      // we aren't doing interrupts, so T is never set except by SAV
      if (mp==0 && reg[x]<rombase) ram[reg[x]&MAXMEM]=t;
      break;
      case 9:
      t=x<<4|p;
      if (mp==0 && reg[x]<rombase) ram[reg[2]&MAXMEM]=t;
      x=p;
      reg[2]--;
      break;
      case 0xa:
      case 0xb:
        q=N&1;
        break;
      case 0xc:
	work=d+memread(reg[p]);
      if (work&0x100) df=1; else df=0;
      d=work;
      break;
      case 0xd:
	work=memread(reg[p])-d-(df?0:1);
      if (work&0x100) df=0; else df=1;
      d=work;
      reg[p]++;
      break;
      case 0xe:
      work=d<<1;
      work|=df;
      if (work&0x100) df=1; else df=0;
      d=work;
      break;
      case 0xf:
	work=d-memread(reg[p])-(df?0:1);
      if (work&0x100) df=0; else df=1;
      d=work;
      reg[p]++;
      break;

    }
    break;
    case 8:
    d=reg[N]&0xFF;
    break;
    case 9:
    d=reg[N]>>8;
    break;
    case 0xA:
    reg[N]=(reg[N]&0xFF00)|d;
    break;
    case 0xB:
    reg[N]=(d<<8)|(reg[N]&0xFF);
    break;
    case 0xC:
    // jumps
    if (N==4) break;  // NOP
    {
      uint16_t tgt=memread(reg[p])<<8|memread(reg[p]+1);

    switch (N)
    {
      case 0:
        reg[p]=tgt;
        break;
      case 8:
        reg[p]+=2;
        break;
      case 1:
      case 9:
      if (q==(N==1)) reg[p]=tgt;
      else reg[p]+=2;
      break;
      case 2:
      if (d==0) reg[p]=tgt;
      else reg[p]+=2;
      break;
      case 3:
      case 0xb:
      if (df==(N==3)) reg[p]=tgt;
      else reg[p]+=2;
      break;
      case 0xA:
      if (d!=0) reg[p]=tgt;
      else reg[p]+=2;
      break;
      case 0xC:  // we don't do IE currently
      break;
      case 5:
      case 0xD:
      if (q==(N==0xD)) reg[p]+=2;
      break;
      case 0xE:
      if (d==0) reg[p]+=2;
      break;
      case 6:
      if (d!=0) reg[p]+=2;
      break;
      case 7:
      case 0xF:
      if (df==(N==0xF)) reg[p]+=2;
      break;
    }
  }
    break;
    case 0xD:
      p=N;
      break;
    case 0xE:
      x=N;
      break;
      case 0xF:  // math
      switch (N)
      {
        case 0:
	  d=memread(reg[x]);
        break;
        case 1:
        d=d|memread(reg[x]);
        break;
        case 2:
        d=d&memread(reg[x]);
        break;
        case 3:
        d=d^memread(reg[x]);
        break;
        case 4:
	  work=d+memread(reg[x]);

        if (work&0x100) df=1; else df=0;
        d=work;
        break;
        case 5:
	  work=memread(reg[x])-d;
        if (work&0x100) df=0; else df=1;
        d=work;

	
        break;
        case 6:
        if (d&1) df=1; else df=0;
        d>>=1;
        break;
        case 7:
	  work=d-memread(reg[x]);
        if (work&0x100) df=0; else df=1;
        d=work;
        break;
        case 8:
	  d=memread(reg[p]);
        reg[p]++;
        break;
        case 9:
	  d=d|memread(reg[p]);
        reg[p]++;
        break;
        case 0xA:
	  d=d&memread(reg[p]);
	  reg[p]++;
        break;
        case 0xB:
	  d=d^memread(reg[p]);
	  reg[p]++;
        break;
        case 0xC:
	  work=memread(reg[p])+d;
	  if (work&0x100) df=1; else df=0;
	  d=work;
	  reg[p]++;
        break;
        case 0xd:
	  work=memread(reg[p])-d;
        if (work&0x100) df=0; else df=1;
        d=work;
        reg[p]++;
        break;
        case 0xe:
        if (d&0x80) df=1; else df=0;
        d<<=1;
        break;
        case 0xf:
	  work=d-memread(reg[p]);
	  if (work&0x100) df=0; else df=1;
	  d=work;
	  reg[p]++;
        break;





      }
      break;


  }
return 1;
}
