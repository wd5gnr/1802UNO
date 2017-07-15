// 1802 Simulation for KIM-UNO hardware
// Al Williams July 2017
#include <Arduino.h>
#include <stdint.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>  // note: AVR only!

#include "1802.h"
#include "main.h"
#include "ihex1802.h"

#define MAXMEM 0x3FF  // maximum memory address; important: only 1K of EEPROM to store stuff in
#define LED_PORT 4     // port for DATA LED display
#define SW_PORT 4      // Front panel switch port
#define SER_INP 1     // UART input port
#define SER_OUT 1     // UART output port
#define CTL_PORT 7    // Control port
#define A0_PORT 2     // LSD address display output port
#define A1_PORT 3     // MSD address display output port

// CPU states... run, load memory, or set address
int runstate=0;
int loadstate=0;
int addstate=0; // this is bogus but handy
int tracemode=0;
int addisp=0;
uint8_t adhigh=0;  // high address display for I/O
uint8_t adlow=0;  // low address display for I/O



unsigned int caddress;  // current load address
unsigned int data;      // data

int noserial=0;

// CPU registers and stuff
uint16_t reg[16];
uint8_t p,x,d,df,q,t;
uint8_t I,N;
uint16_t work;
uint8_t ef1, ef2, ef3, ef4;
uint8_t mp=0;  // memory protect

// RAM
uint8_t ram[MAXMEM+1]; 		// main 1KB RAM		 0x000-0x3FF
// ROM
const uint8_t PROGMEM rom[]=
  {
    0x7A,
    0x7B,
    0x30,
    0x00    // assumes rombase is XX00
  };

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

// Do an execution cycle
int exec1802(int ch)
{
  int rv=1;

// commands
// Go = run
// ST = Stop run or load
// RS = Reset
// AD = Copy extended data to load address
// + - ef4
// DA - load
// PC - memory protect on
// SST - step
// otherwise we build up a hex number in data (which is 16-bits but only
// the lower 8 bits show and are used in most cases)
// Should we stop running?
  ch=ch>='a'&&ch<='z'?ch&0xDF:ch;  // convert to upper case

// meta keys (1 second press)
  if (ch=='>')  // save 1K to EEPROM
  {
    int ptr;
    reset();
    for (ptr=0;ptr<=(MAXMEM<0x3FF?MAXMEM:0x3FF);ptr++) EEPROM.write(ptr,ram[ptr]);
    return 0;
  }
  if (ch=='<' && mp==0 && runstate==0)  // load 1K from EEPROM if not MP and not running
  {
    int ptr;
    for (ptr=0;ptr<=(MAXMEM<0x3FF?MAXMEM:0x3FF);ptr++) ram[ptr]=EEPROM.read(ptr);
    reset();
    return 0;
  }
  if (ch=='@' && mp==0)  // load memory from serial
  {
    uint16_t ptr=0;
    uint8_t val=0;
    int state=0;
    while (state==0)
    {
      int c;
      while ((c=Serial.read())==-1);
      if ((c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F'))
      {
        ptr<<=4;
        if (c>='a') c=c-'a'+10; else if (c>='A') c=c-'A'+10; else c-='0';
        ptr+=c;
      }
     else
     {
       state=1;
     }
    }
    while (state==1||state==4)
   {
    int c;
    while ((c=Serial.read())==-1);
    if (state==4 && (c<'0'&&c!='.')) continue; else state=1;  // skip multiple whitespace
    if ((c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F'))
    {
      val<<=4;
      if (c>='a') c=c-'a'+10; else if (c>='A') c=c-'A'+10; else c-='0';
      val+=c;
    }
    else if (c=='.') state=3; else
    {
      ram[ptr++]=val;
      state=4;
      val=0;
    }
   }
   return 1;
  }
  if (ch=='Y' || ch=='y') // write intel hex
    {
      ihexo1802 writer;
      Serial.println("");
      writer.write(ram,MAXMEM);
      return 1;
    }

  if (ch=='X' || ch=='x')  // read intel hex
    {
      ihex1802 reader;
      reader.read();
      return 1;
    }
  if (ch=='?')  // dump memory to serial
  {
    uint16_t ptr;
    int group=0;
    Serial.println("@0000:");
    for (ptr=0;ptr<=MAXMEM;ptr++)
    {
      Serial.print(ram[ptr],HEX);
      group++;
      if (group==16)
      {
        Serial.print('\n');
        group=0;
      }
      else Serial.print(' ');
    }
    Serial.print(".\n");
    return 1;
    }
    if (ch==';') tracemode^=1;   // trace toggle
    if (ch=='*')    // dump processor state
    {
      int j;
      for (j=0;j<16;j++)
      {
        Serial.print(j,HEX); Serial.print(':'); Serial.println(reg[j],HEX);
      }
      Serial.print("DR:"); Serial.println(d,HEX);
      Serial.print("DF:"); Serial.println(df);
      Serial.print("X:"); Serial.println(x);
      Serial.print("P:"); Serial.println(p);
      Serial.print("Q:"); Serial.println(q);
      return 1;
    }
// regular keys
  if (ch==KEY_ST && runstate==1) { runstate=0; caddress=reg[p]; }  // stop
  // Should we start running?
  if (ch==KEY_GO && runstate==0 && loadstate==0 && addstate==0) runstate=1;
// Should we go to load state
  if (ch==KEY_DA && runstate==0 && loadstate==0 && addstate==0) { loadstate=1;  data=memread(caddress);   }
  // reset?
  if (ch==KEY_RS) { reset(); runstate=0; addstate=0;  loadstate=0;}
  // Stop load state
  if (ch==KEY_ST && loadstate==1) loadstate=0;
  // Load extended data register into caddress
  if (ch==KEY_AD && runstate==0 && loadstate==0 && addstate==0) addstate=1;
  // EF4 push
//  if (ch=='+') ef4=1; else ef4=0;   // EF4 now set in keyboard routine
  // EF1 push
  if (ch==KEY_SST && runstate==0)
  {
    data=memread(reg[p]);
    setaddress(reg[p]);  // we don't care if the program is using address display here
    run();
    runstate=2;
  }
  if (runstate==2&&ch!=KEY_DA) runstate=0;
  // set memory protect on without race (state 2 while held, then state 1)
  if (ch==KEY_PC)
  {
    if (mp==0) mp=2;
    if (mp==1) mp=0;
  }
  else
  {
    if (mp==2) mp=1;
  }


// build up hex number
  if (ch>='0' && ch<='9') data=data<<4|(ch-'0');
  if (ch>='A' && ch<='F') data=data<<4|(ch-'A'+10);
  // in case of serial input
  if (ch>='a' && ch<='f') data=data<<4|(ch-'a'+10);

  // Ok now we can see what state we are in
  // if loading (or displaying if mp==1)
  if (loadstate==1 && ef4==1)  {  if (mp||caddress>=rombase) data=memread(caddress); else ram[caddress&MAXMEM]=data;  loadstate=2;  }
  // state 2 waits for EF4 release
  if (loadstate==2 && ef4==0) { loadstate=1; data=memread(++caddress); }
  // run if required
  if (runstate==1) rv=run();
  // copy address if required
  if (addstate) { caddress=data; addstate=0; setaddress(caddress); }  // don't care about address display setting here
  // stop running if there was an error
  if (rv<=0 && runstate==1)
  {
    runstate=0;
    caddress=reg[p];
  }

// address should show load address or execution address
  if (runstate)
    {
      if (addisp)
	setaddress(adhigh<<8|adlow);
      else
	setaddress(reg[p]);
    }

  if (loadstate) setaddress(caddress);
  // set up data display
  setdata(data);
  // set up DP LEDs
 setdp(0,q);
 setdp(1,loadstate!=0);
 setdp(2,runstate);
 setdp(4,mp!=0);
  return rv;
}



// Input from any port gives you the data register
// except port 1 is serial input
uint8_t input(uint8_t port)
{
  if (port==SER_INP)
  {
    int rv=Serial.read();
    if (rv==-1) rv=0;
    return rv;
  }
  return data;
}

// Output to any port writes to the data display
void output(uint8_t port, uint8_t val)
{
  if (port==SER_OUT) Serial.print((char)val);
  else if (port==LED_PORT) data=val;
  else if (port==A0_PORT) adlow=val;
  else if (port==A1_PORT) adhigh=val;
  else if (port==CTL_PORT)
  {
    noserial=val&1;  // set 1 to use serial port as I/O, else use as front panel
    addisp=(val&2)==2; // set bit 1 to 1 to use address displays
  }
}

// Emulation engine
int run(void)
{
  uint8_t inst=memread(reg[p]);
  reg[p]++;
  I=inst>>4;
  N=inst&0xF;
  if (tracemode)
  {
    Serial.print(reg[p]-1,HEX);
    Serial.print(':');
    Serial.print(inst,HEX);
    Serial.print(' ');
    Serial.println(d,HEX);
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
        if (!d) nxt=tpc;
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
      if (work&0x100) df=1; else df=0;
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
      if (work&0x100) df=1; else df=0;
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
      if (work&0x100) df=1; else df=0;
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
      if (work&0x100) df=1; else df=0;
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
      case 8:
        reg[p]+=2;
        break;
      case 1:
      case 9:
      if (q==(N==1)) reg[p]=tgt;
      reg[p]+=2;
      break;
      case 2:
      if (d==0) reg[p]=tgt;
      reg[p]+=2;
      break;
      case 3:
      case 0xb:
      if (df==(N==3)) reg[p]=tgt;
      reg[p]+=2;
      break;
      case 0xA:
      if (d!=0) reg[p]=tgt;
      reg[p]+=2;
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
        if (work&0x100) df=1; else df=0;
        d=work;
        break;
        case 6:
        if (d&1) df=1; else df=0;
        d>>=1;
        break;
        case 7:
	  work=d-memread(reg[x]);
        if (work&0x100) df=1; else df=0;
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
	  work=memread(reg[p])+d+df;
	  if (work&0x100) df=1; else df=0;
	  d=work;
	  reg[p]++;
        break;
        case 0xd:
	  work=memread(reg[p])-d-(df?0:1);
        if (work&0x100) df=1; else df=0;
        d=work;
        reg[p]++;
        break;
        case 0xe:
        if (d&0x80) df=1; else df=0;
        d<<=1;
        break;
        case 0xf:
	  work=d-memread(reg[p])-(df?0:1);
	  if (work&0x100) df=1; else df=0;
	  d=work;
	  reg[p]++;
        break;





      }
      break;


  }
return 1;
}


// Intel hex stuff

int ihex1802::getch(void)
{
  int c;
  while ((c=Serial.read())==-1);
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
