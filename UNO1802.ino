// This is mostly Oscar's original KIM UNO code
// Modified by Al Williams for use as an 1802
// For the most part I just hacked away the parts specific to the KIM
// I did add code to theh display part to take care of the decimal points
// in the hex out code, and added a few defines and a header.
// But for the most part, all of this code is straight out of the KIM Uno.

// Original version:
//version date: 20160907
#include "Arduino.h"
#include <stdint.h>
#include <util/delay.h>
#include "main.h"
#include "UNO1802.h"

#define VERSION "1802UNOv21b"
// version b:
// -- OLED code added (all contained in // OLED display -------- [...] // end of OLED display --------
// -- Converted some print commends to use F() so as to save RAM
// -- renamed 1802*.* files to UNO1802*.* files. Because, astonishingly, Arduino IDE does not accept filenames starting with numbers. Really!
// -- renamed main.cpp to UNO1802.ino
// -- removed pullup from LED lines in scankeys (otherwise you'll have problem with SDA/Decimal point line)
// -- added pullup somewhere in scankeys, but forgot where. Point being, new Arduino IDE uses INPUT_PULLUP, not the 2 instruction INPUT, HIGH sequence anymore, it ignores that (golly)
// -- inserted 1802 emulation WITHIN the driveLEDs function, instead of wasting 2ms for every time a LED is lit up.
// -- DISPLAY_DIVISOR reduced from 32 to 8, because updates are now much less costly in performance and we need a bit more LED brightness

// OLED display -----------------------------------------

//#define OLED
// undefine to cut out OLED code 
// - without the physical OLED, the machine would hang otherwise
// - NOTE: if you have an OLED connected, you MUST define OLED. If you don't, you MUST undefine it.   !!!!!!!!!!!!!!!!!!!!!! READ THIS !!!!!!!!!!!!!!!!!!!!

#ifdef OLED
#include "SSD1306Ascii.h"               
#include "SSD1306AsciiAvrI2c.h"
#include "startrek.h" // contains display image
#define I2C_ADDRESS 0x3C
SSD1306AsciiAvrI2c oled;
//void oledRefresh(int newOledMode);
#endif
// end of OLED display ----------------------------------

#define SERIAL_ESCAPE '|'  // turn terminal input into real terminal input



uint8_t curkey = 0;
//uint8_t curkey = KEY_GO;

char threeHex[3][2];        // LED display
int dp[8]={ 0, 0, 0, 0, 0, 0, 0, 0};  // decimal points

byte aCols[8] = { A5, 2,3,4,5,6,7,8 }; // note col A5 is the extra one linked to DP
byte aRows[3] = { 9,10,11 };
byte ledSelect[8] =  { 12, 13, A0, A1, A2, A3, A7, A4 }; // note that A6 and A7 are not used at present. Can delete them.

byte dig[19] = {
// bits     6543210
// digits   abcdefg
          0b01111110,//0
          0b00110000,//1
          0b01101101,//2
          0b01111001,//3
          0b00110011,//4
          0b01011011,//5
          0b01011111,//6
          0b01110000,//7
          0b01111111,//8
          0b01111011,//9
          0b01110111,//a
          0b00011111,//b
          0b01001110,//c
          0b00111101,//d
          0b01001111,//e
          0b01000111,//f
          0b00000001, //g printed as -
          0b00001000, //h printed as _
          0b00000000  //i printed as <space>
};







// get and clear a key (from original code)
  uint8_t getAkey(void)            { return(curkey);  }
  void clearkey(void)             { curkey = 0; }


// Set the state of a decimal point
  void setdp(int pos, int state) { dp[pos]=state; }

// tick counter for display updates
uint8_t tick=0;


// Set up everything
void setup () {

// OLED display -----------------------------------------
#ifdef OLED
  //put the screen bitmap in RAM
  //maybe not the most elegant place to do this, but practical

  uint8_t ixx;
  for (ixx=3; ixx<254; ixx++) // we leave first 3 bytes (branch to 0x8000) untouched
    ram[ixx]= (pgm_read_byte_near (startrek + ixx));

  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.ssd1306WriteCmd(SSD1306_MEMORYMODE);
  oled.ssd1306WriteCmd(0x00);  // horizontal mode
  oled.setFont(font5x7); // OLED fonts are not used - but for future use of a, erm, Super Pixie Terminal Mode?
  // disable twi module, acks, and twi interrupt
  TWCR &= ~(_BV(TWEN) | _BV(TWIE) | _BV(TWEA)); // stop I2C clock so it does not interfere with KIM Uno use of that pin!    
#endif

// end of OLED display -----------------------------------------

  Serial.begin (9600);
  Serial.println ("Wait");
  setupUno();
  reset();
  Serial.print(F(VERSION " Free Memory=")); // just a little check, to avoid running out of RAM!
  Serial.println(freeRam());
}


// OLED display -----------------------------------------
uint8_t l1=0, l2=1; // used within loop to keep track of which part of display is updated
// end of OLED display -----------------------------------------

// main loop

void loop()
{

  if (noserial==0 && Serial.available())  // if serial input, process that
  {
   curkey=Serial.read();
   if (curkey==SERIAL_ESCAPE) noserial=1;   // one way ticket
   else exec1802(curkey);
   curkey=0;
  }
  if (tick%DISPLAY_DIVISOR==0) 
      scanKeys();   // scan the keyboard

  exec1802(curkey);   // process even if 0
  curkey=0;         // clear out keyboard

  // Update display only so often
  if (tick%DISPLAY_DIVISOR==0) 
  {
#ifdef OLED
    // OLED display -----------------------------------------
    // enable twi module, acks, and twi interrupt:
    TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA); // switch on I2C...
    oled.paintscreen(l1,l2);  // paint 1/4th of the display
    l1=l1+2; if (l1==8) l1=0;
    l2=l2+2; if (l2==9) l2=1;
    // disable twi module, acks, and twi interrupt
    TWCR &= ~(_BV(TWEN) | _BV(TWIE) | _BV(TWEA)); // switch off I2C...
    // end of OLED display -----------------------------------------
#endif
    driveLEDs();
  }
  tick++;
}



// check for out of RAM
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}


// =================================================================================================
// KIM Uno Board functions are bolted on from here
// =================================================================================================

void setupUno() {
  int i;
  // --------- initialse for scanning keyboard matrix -----------------
  // set columns to input with pullups
  for (i=0;i<8;i++)
  {  pinMode(aCols[i], INPUT_PULLUP);           // set pin to input
  }
  // set rows to output, and set them High to be in Neutral position
  for (i=0;i<3;i++)
  {  pinMode(aRows[i], OUTPUT);           // set pin to output
     digitalWrite(aRows[i], HIGH);       // set to high
  }
  // --------- clear display buffer ------------------------------------
  for (i=0;i<3;i++)
  { threeHex[i][0]=i+1; threeHex[i][1]=i+5; }
  Serial.println(F("Ready"));
}


// set 16-bit address into LEDs
void setaddress(uint16_t a)
{
  threeHex[0][0]=a>>12;
  threeHex[0][1]=(a&0xF00)>>8;
  threeHex[1][0]=(a&0xF0)>>4;
  threeHex[1][1]=a&0xF;
}

// set 8-bit data byte into leds
void setdata(uint8_t d)
{
  threeHex[2][0]=d>>4;
  threeHex[2][1]=d&0xF;
}

// Keep leds lit up
void driveLEDs()
{
  int led, col, ledNo, currentBit, bitOn;
  int byt,i;

  // 1. initialse for driving the 6 (now 8) 7segment LEDs
  // ledSelect pins drive common anode for [all segments] in [one of 6 LEDs]
  for (led=0;led<7;led++)
  { pinMode(ledSelect[led], OUTPUT);  // set led pins to output
    digitalWrite(ledSelect[led], LOW); // LOW = not lit
  }
  // 2. switch column pins to output mode
  // column pins are the cathode for the LED segments
  // lame code to cycle through the 3 bytes of 2 digits each = 6 leds
  for (byt=0;byt<3;byt++)
    for (i=0;i<2;i++)
    {
      ledNo=byt*2+i;
      for (col=0;col<8;col++)
      {  pinMode(aCols[col], OUTPUT);           // set pin to output
         //currentBit = (1<<(6-col));             // isolate the current bit in loop
         currentBit = (1<<(7-col));             // isolate the current bit in loop
         bitOn = (currentBit&dig[threeHex[byt][i]])==0;
         digitalWrite(aCols[col], bitOn);       // set the bit
      }
      // set decimal point or no
      pinMode(aCols[0],OUTPUT);
      digitalWrite(aCols[0],dp[ledNo]==0);

      digitalWrite(ledSelect[ledNo], HIGH); // Light this LED

//      ---old version:
//      delay(2);
//      ---new version, to not waste 2ms every time we light up one single 7-segment LED block...
      //unsigned long time1, time2;
      uint8_t is;
      //time1 = micros();
      for (is=0;is<80;is++)
        exec1802(0);   // do some 1802 emulation whilst the LED is lit up... we should take about 2ms here before going on...
      //time2 = micros();
      //Serial.println(time2-time1);
//    -------------

      digitalWrite(ledSelect[ledNo], LOW); // unLight this LED
    }
} // end of function





// convert scan codes into ASCII
uint8_t parseChar(uint8_t n) //  parse keycode to return its ASCII code
{
  uint8_t c;

  // KIM-I keys
  switch (n-1) {              //KIM Uno keyscan codes to ASCII codes used by emulator
    case	7	: c = 	'0' ;  break;	//        note: these are n-1 numbers!
    case	6	: c = 	'1';  break;	//
    case	5	: c = 	'2';  break;	//
    case	4	: c = 	'3';  break;	//
    case	3	: c = 	'4';  break;	//
    case	2	: c = 	'5';  break;	//
    case	1	: c = 	'6';  break;	//
    case	0	: c = 	KEY_ST;  break;	// ST

    case	15	: c = 	'7' ;  break;	//
    case	14	: c = 	'8';  break;	//
    case	13	: c = 	'9';  break;	//
    case	12	: c = 	'A';  break;	//
    case	11	: c =   'B';  break;	//
    case	10	: c =   'C';  break;	//
    case	9	: c =   'D';  break;	//
    case	8	: c =   KEY_RS;  break;	// RS

    case	23	: c =  'E';  break;     //
    case	22	: c =  'F';  break;     //
    case	21	: c = 	KEY_AD;   break;     // AD
    case	20	: c = 	KEY_DA;   break;     // DA
    case	19	: c = 	'+'; break;     // +
    case	18	: c = 	KEY_GO;   break;	// GO
    case	17	: c =   KEY_PC;  break;	// PC
    case	16	: c =   KEY_SST;  break; // 	SST toggle
  }
  return c;
}



uint8_t xkeyPressed()    // just see if there's any keypress waiting
{ return (curkey==0?0:1); }



// Scan keyboard (modfied a bit)
void scanKeys()
{
  int led,row,col, noKeysScanned;
  static int keyCode = -1, prevKey = 0;
  static unsigned long timeFirstPressed = 0;

  // 0. disable driving the 7segment LEDs -----------------
  for (led=0;led<8;led++)
  { pinMode(ledSelect[led], INPUT);  // set led pins to input. Maybe INPUT_PULLUP?
  }
  // 1. initialise: set columns to input with pullups
  for (col=0;col<8;col++)
  {  pinMode(aCols[col], INPUT_PULLUP);           // set pin to input
  }
  // 2. perform scanning
  noKeysScanned=0;

  for (row=0; row<3; row++)
  { digitalWrite(aRows[row], LOW);       // activate this row
    for (col=0;col<8;col++)
    { 
      
//#ifdef OLED
      // noise from SDA (which doubles as col0) needs bit of time to settle down...
      // otherwise you get incidental cases of a col0 keypress not being read, and thus a 
      // repeat of that keypress when it gets read in the next sample.
      if (col==0) delayMicroseconds(200);
//#endif
      
      if (digitalRead(aCols[col])==LOW)  // key is pressed
      { keyCode = col+row*8+1;
        if (keyCode==20 && !ef4term) ef4=1; else ef4=0;   // Set EF4 as long as + held down
        if (keyCode!=prevKey)
        {   //Serial.println();
            //Serial.print(" col: ");  Serial.print(col, DEC);
            //Serial.print(" row: ");  Serial.print(row, DEC);
            //Serial.print(" prevKey: ");  Serial.print(prevKey, DEC);
            //Serial.print(" KeyCode: ");  Serial.println(keyCode, DEC);
           prevKey = keyCode;
           curkey = parseChar(keyCode);
            //Serial.print(" curkey: ");  Serial.print(curkey, DEC);
           timeFirstPressed=millis();  //
        }
        else // if pressed for >1sec, it's a ModeShift key
        {
          if ((millis()-timeFirstPressed)>1000) // more than 1000 ms
          {
            if (keyCode==21) curkey='<';  // do not use curkey in if here since it gets reset
            if (keyCode==22) curkey='>';
            if (keyCode==17) curkey=KEY_SST;  // repeat SST key for single step
              timeFirstPressed=millis(); // because otherwise you toggle right back!

          }
        }
      }
      else
        noKeysScanned++;  // another row in which no keys were pressed
    }
    digitalWrite(aRows[row], HIGH);       // de-activate this row
  }

  if (noKeysScanned==24)
    {   // no keys detected in any row, 3 rows * 8 columns = 24. used to be 28.
    prevKey=0;        // allows you to enter same key twice
    ef4=0;
Serial.print(".");
    }
} // end of function
