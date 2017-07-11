// 1802 Simulation for KIM-UNO hardware
// Al Williams July 2017
#include <Arduino.h>
#include <stdint.h>
#include <EEPROM.h>

#include "1802.h"
#include "main.h"

#define MAXMEM 0x3FF  // maximum memory address; important: only 1K of EEPROM to store stuff in
#define LED_PORT 4     // port for DATA LED display
#define SW_PORT 4      // Front panel switch port
#define SER_INP 1     // UART input port
#define SER_OUT 1     // UART output port
#define CTL_PORT 7    // Control port


// CPU states... run, load memory, or set address
int runstate=0;
int loadstate=0;
int addstate=0; // this is bogus but handy
int tracemode=0;

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
// DA - ef1 (run) or single step (halt)
// PC - memory protect on
// SST - load
// otherwise we build up a hex number in data (which is 16-bits but only
// the lower 8 bits show and are used in most cases)
// Should we stop running?

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
  if (ch=='@')  // load memory from serial
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
    while (state==1)
   {
    int c;
    while ((c=Serial.read())==-1);
    if ((c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F'))
    {
      val<<=4;
      if (c>='a') c=c-'a'+10; else if (c>='A') c=c-'A'+10; else c-='0';
      val+=c;
    }
    else if (c=='.') state=3; else
    {
      ram[ptr++]=val;
      val=0;
    }
   }
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
  if (ch==KEY_ST && runstate==1) { runstate=0; caddress=reg[p]; }
  // Should we start running?
  if (ch==KEY_GO && runstate==0 && loadstate==0 && addstate==0) runstate=1;
// Should we go to load state (preload data display in case MP is on)
  if (ch=='[' && runstate==0 && loadstate==0 && addstate==0) { loadstate=1; data=ram[caddress&MAXMEM]; }
  // reset?
  if (ch==KEY_RS && loadstate==0) { reset(); runstate=0; addstate=0; }
  // Stop load state
  if (ch==KEY_ST && loadstate==1) loadstate=0;
  // Load extended data register into caddress
  if (ch==KEY_AD && runstate==0 && loadstate==0 && addstate==0) addstate=1;
  // EF4 push
//  if (ch=='+') ef4=1; else ef4=0;   // EF4 now set in keyboard routine
  // EF1 push
  if (ch==KEY_DA && runstate==1) ef1=1; else ef1='0';
  if (ch==KEY_DA && runstate==0)
  {
    data=ram[reg[p]&MAXMEM];
    setaddress(reg[p]);
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
  if (loadstate==1 && ef4==1)  {  if (mp) data=ram[caddress&MAXMEM]; else ram[caddress&MAXMEM]=data; caddress++; loadstate=2;  }
  // state 2 waits for EF4 release
  if (loadstate==2 && ef4==0) { loadstate=1; }
  // run if required
  if (runstate==1) rv=run();
  // copy address if required
  if (addstate) { caddress=data; addstate=0; setaddress(caddress); }
  // stop running if there was an error
  if (rv<=0 && runstate==1)
  {
    runstate=0;
    caddress=reg[p];
  }

// address should show load address or execution address
  if (runstate) setaddress(reg[p]);
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
  else if (port==CTL_PORT)
  {
    noserial=val&1;  // set 1 to use serial port as I/O, else use as front panel
  }
}

// Emulation engine
int run(void)
{
  uint8_t inst=ram[reg[p]&MAXMEM];
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
    d=ram[reg[N]&MAXMEM];
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
      uint16_t tpc=ram[reg[p]];
      uint16_t nxt=++reg[p];
      switch (N)
      {
        case 0:
        nxt=tpc;
        break;
        case 1:
        case 9:
        if (q!=(N&1)) nxt=tpc;
        break;
        case 2:
        if (d==0) nxt=tpc;
        break;
        case 3:
        case 11:
        if (df!=(N&1)) nxt=tpc;
        break;
        case 4:
        case 12:
        if (ef1!=(N&1)) nxt=tpc;
        break;
        case 5:
        case 13:
        if (ef2!=(N&1)) nxt=tpc;
        break;
        case 6:
        case 14:
        if (ef3!=(N&1)) nxt=tpc;
        break;
        case 7:
        case 15:
        if (ef4!=(N&1)) nxt=tpc;
        break;
        case 8:
        break;  //does this skip? I don't think so
        case 10:
        if (!d) nxt=tpc;
        break;
      }
      reg[p]=nxt;
      break;
    }

    case 4:
    d=ram[reg[N]&MAXMEM];
    reg[inst&0x0f]++;
    break;
    case 5:
    ram[reg[N]&MAXMEM]=d;
    break;
    case 6:
    // IRX + I/O
      if (N==0) reg[x]++;
      else
      if (N==8) ;   // ?
      else if (N<8) output(N,d);
      else d=input(N-8);  // port 1-7

    break;
    case 7:  // misc
    switch (N)
    {
      case 0:
      case 1:  // ths same since we don't manage IE
      work=ram[reg[x]&MAXMEM];
      x=work>>4;
      p=work&0xF;
      reg[x]++;
      break;
      case 2:
      d=ram[reg[x]&MAXMEM];
      reg[x]++;
      break;
      case 3:
      ram[reg[x]&MAXMEM]=d;
      reg[x]--;
      break;
      case 4:
      work=ram[reg[x]]+d+df;
      if (work&0x100) df=1; else df=0;
      d=work;  // conversion will chop off top for us
      break;
      case 5:
      work=ram[reg[x]]-d-(df?0:1);
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
      work=d-ram[reg[x]]-(df?0:1);
      if (work&0x100) df=1; else df=0;
      d=work;
      break;
      case 8:
      // we aren't doing interrupts, so T is never set except by SAV
      ram[reg[x]&MAXMEM]=t;
      break;
      case 9:
      t=x<<4|p;
      ram[reg[2]&MAXMEM]=t;
      x=p;
      reg[2]--;
      break;
      case 0xa:
      case 0xb:
        q=N&1;
        break;
      case 0xc:
      work=d+ram[reg[p]];
      if (work&0x100) df=1; else df=0;
      d=work;
      break;
      case 0xd:
      work=ram[reg[p]]-d-(df?0:1);
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
      work=d-ram[reg[p]]-(df?0:1);
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
    uint16_t tgt=ram[reg[p]&MAXMEM]<<8|ram[(reg[p]+1)&MAXMEM];
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
        d=ram[reg[x]&MAXMEM];
        break;
        case 1:
        d=d|ram[reg[x]&MAXMEM];
        break;
        case 2:
        d=d&ram[reg[x]&MAXMEM];
        break;
        case 3:
        d=d^ram[reg[x]&MAXMEM];
        break;
        case 4:
        work=d+ram[reg[x]&MAXMEM];
        if (work&0x100) df=1; else df=0;
        d=work;
        break;
        case 5:
        work=ram[reg[x]&MAXMEM]-d;
        if (work&0x100) df=1; else df=0;
        d=work;
        break;
        case 6:
        if (d&1) df=1; else df=0;
        d>>=1;
        break;
        case 7:
        work=d-ram[reg[x]&MAXMEM];
        if (work&0x100) df=1; else df=0;
        d=work;
        break;
        case 8:
        d=ram[reg[p]&MAXMEM];
        reg[p]++;
        break;
        case 9:
        d=ram[reg[p]&MAXMEM]|d;
        reg[p]++;
        break;
        case 0xA:
        d=ram[reg[p]&MAXMEM]&d;
        reg[p]++;
        break;
        case 0xB:
        d=ram[reg[p]&MAXMEM]^d;
        reg[p]++;
        break;
        case 0xC:
        work=ram[reg[p]]+d+df;
        if (work&0x100) df=1; else df=0;
        d=work;
        reg[p]++;
        break;
        case 0xd:
        work=ram[reg[p]]-d-(df?0:1);
        if (work&0x100) df=1; else df=0;
        d=work;
        reg[p]++;
        break;
        case 0xe:
        if (d&0x80) df=1; else df=0;
        d<<=1;
        break;
        case 0xf:
        work=d-ram[reg[p]]-(df?0:1);
        if (work&0x100) df=1; else df=0;
        d=work;
        reg[p]++;
        break;





      }
      break;


  }
return 1;
}
