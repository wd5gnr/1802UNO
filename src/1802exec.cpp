
#include <Arduino.h>
#include <EEPROM.h>
#include "1802.h"
#include "main.h"
#include "ihex1802.h"

// CPU states... run, load memory, or set address
int runstate=0;
int loadstate=0;
int addstate=0; // this is bogus but handy
int tracemode=0;
uint8_t ef4term=0;

// Properly print two hex digits
void print2hex(uint8_t v)
{
  if (v<0x10) Serial.print('0');
  Serial.print(v,HEX);
}

void print4hex(uint16_t v)
{
  print2hex(v>>8);
  print2hex(v);
}

void updateLEDdata(void)
{
  // address should show load address or execution address
  if (runstate)
    {
      if (addisp) setaddress(adhigh<<8|adlow);
      else setaddress(reg[p]);
    }

  if (loadstate) setaddress(caddress);
  // set up data display
  setdata(data);
  // set up DP LEDs
 setdp(0,q);
 setdp(1,loadstate!=0);
 setdp(2,runstate);
 if (ef4==0) ef4=ef4term;
 setdp(3,ef4);
 setdp(4,mp!=0);

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
  #if MONITOR==1
  if (ch=='\\')
    {
      monitor();
      return 1;
    }
  #endif

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
  // end temporary
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
       state=4;
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
  if (ch=='!')
  {
    for (int i=0;i<3;i++)
    for (int j=0;j<2;j++)
    {
      if (i==2&&j==0) Serial.print(" ");
      Serial.print(threeHex[i][j],HEX);
    }
    Serial.println("");
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
      print2hex(ram[ptr]);
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
        Serial.println();
        Serial.print('R');
        Serial.print(j,HEX);
        Serial.print(':');
        print4hex(reg[j]);
        Serial.println("");
      }
      Serial.print("DR:"); print2hex(d); Serial.println();
      Serial.print("DF:"); Serial.println(df);  // all single digit
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
  if (ch==KEY_RS) { reset(); return 1; }
  // Stop load state
  if (ch==KEY_ST && loadstate==1) loadstate=0;
  // Load extended data register into caddress
  if (ch==KEY_AD && runstate==0 && loadstate==0 && addstate==0) addstate=1;
  // EF4 push
//  if (ch=='+') ef4=1; else ef4=0;   // EF4 now set in keyboard routine
   if (ch=='$') ef4term=ef4term?0:1;
   if (ef4==0) ef4=ef4term;  // let ef4term override ef4 (need this for run even though LED update sets it too)
   
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
  if (ch>='0' && ch<='9') {idata=idata<<4|(ch-'0'); data=idata; }
  if (ch>='A' && ch<='F') {idata=idata<<4|(ch-'A'+10); data=idata; }
  // in case of serial input
  if (ch>='a' && ch<='f') {idata=idata<<4|(ch-'a'+10); data=idata; }

  // Ok now we can see what state we are in
  // if loading (or displaying if mp==1)
  if (loadstate==1 && ef4==1)  {  if (mp||caddress>=rombase[0]) data=memread(caddress); else ram[caddress&MAXMEM]=data;  loadstate=2;  }
  // state 2 waits for EF4 release
  if (loadstate==2 && ef4==0) { loadstate=1; data=memread(++caddress); }
  // run if required
#if MONITOR==1
  if (runstate==1 && monactive==0) { rv=run(); }
#else
  if (runstate==1) rv=run();
#endif
  // copy address if required
  if (addstate) { caddress=data; addstate=0; setaddress(caddress); addstate=0; }  // don't care about address display setting here
  // stop running if there was an error
  if (rv<=0 && runstate==1)
  {
    runstate=0;
    caddress=reg[p];
  }

  updateLEDdata();
  return rv;
}
