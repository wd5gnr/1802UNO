/*
Hi lo game 

From the original documentation:
ELF HI LO Game Instructions:

The purpose of this game is to guess a number selected by the computer between 1 and 100 in as few tries as possible.

Reset and Run the ELF. All the LED's will be blinking. Press I and the computer will select a secret number, then the 

Q LED will come on, indicating that your input is needed. Enter a number using the toggle switches then press I. 

At that point, one of 3 things can happen:

1- Your number is higher than the secret number, and so the HIGH nibble on the LED display (i.e. the leftmost 4 LED's) will blink

2- Your number is lower than the secret number and the LOW nibble on the LED display (i.e. the rightmost 4 LED's) will blink

3- Your guessed the correct number and it will appear on the LED display and blink

Press I again and the Q LED will come on and you will be able to enter another guess as before. If you guessed the correct number,

then the number of tries will be displayed and blinked and the game will be over.

Hope you like it.

Walid Maalouli

Patched for ROM by Al Williams


*/

const uint8_t PROGMEM rom[]=
  {
    0x90,	
    0x30,
    0x75,
    0XC4,	// filler for patch
    0XB4,	
    0XB5,	
    0XB6,	
    0xC4,  // filler for patch
    0XC4,  // filler for patch again	
    0XF8,	
    0XFF,   // STOR1 now at 03FF,	
    0XA1,	
    0x38,	
    0XC4,
    0XF8,
    0x64,	
    0XA2,	
    0XF8,	
    0XFF,	
    0XA4,	
    0XF8,
    0x57,	
    0XA6,	
    0XD6,	
    0x37,	
    0x20,	
    0x22,	
    0x82,	
    0X3A,	
    0x14,	
    0x30,	
    0X0E,	
    0X7B,
    0x82,	
    0x51,	
    0XE2,
    0XF8,	
    0xFE,	// STOR2 now at 03FE
    0XA2,	
    0x38,	
    0XC4,
    0x37,
    0x29,
    0X3F,
    0X2B,	
    0X6C,	
    0x37,
    0X2E,	
    0x13,	
    0X7A,	
    0XE1,	
    0XF5,	
    0x32,	
    0x47,	
    0X3B,	
    0X3B,	
    0XF8,	
    0X0F,	
    0XC8,	
    0XF8,
    0XF0,	
    0XA4,	
    0XF8,
    0x57,	
    0XA6,	
    0XD6,	
    0X3F,	
    0X3E,	
    0X7B,	
    0x30,	
    0x23,	
    0x02,	
    0XA4,	
    0XF8,
    0x57,	
    0XA6,	
    0XD6,	
    0X3F,	
    0x49,	
    0x83,	
    0XA4,	
    0XF8,
    0x57,	
    0XA6,	
    0XD6,	
    0x30,	
    0x51,	
    0XF8,
    0XAF,	
    0XA5,	
    0XF8,	
    0xFD,	// stor3 now at 03FD
    0XA7,	
    0x84,	
    0x57,	
    0x38,	
    0XC4,
    0XE7,	
    0x64,	
    0x27,	
    0x25,
    0x85,	
    0X3A,	
    0x64,	
    0XF8,	
    0x00,	
    0x57,	
    0x64,	
    0x27,	
    0XF8,	
    0XAF,	
    0XA5,	
    0x25,
    0x85,	
    0X3A,	
    0x70,	
    0XD0,
    // patch @ 0x75 for ROM
    0xF8, 0x00,
    0xB1,
    0xB2,
    0xB3,
    0xA3,
    0xB7,
    0x90,
    0x30,0x03  // return
  };



