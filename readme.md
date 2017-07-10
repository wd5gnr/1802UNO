1802 UNO
===
Starting with Oscar's KIM-UNO code, I changed out the 6502 for an 1802.
See: <http://obsolescence.wixsite.com/obsolescence/kim-uno-summary-c1uuh> for more details.


Configuration
===
You have 1K of RAM and no interrupts.

Keyboard
===
The keyboard is mapped like this:

* Go - Run
* ST - Stop running or stop load mode
* RS - Reset
* AD - Copy extended data register to load address
* Plus - EF4 (Input key for program or load mode enter)
* DA - While running, EF1, while idle, single step
* PC - Protect memory toggle so that load mode displays data
* SST - Enter load mode
* 0-F - Build up hex number. Accumulates 16-bits although you can only see the lower 8. For load mode, the lower 8 is used. For AD all 16-bits are used.
* SST - Hold down for one second to save RAM to EEPROM
* Plus - Hold down for one second when not running and not memory protected to read RAM from EEPROM

Serial Port
===
On a terminal:

* ST=^N  
* RS=^R 
* AD=^A 
* DA=^D 
* GO=^G 
* PC=^P 
* SST=[
* SST (1 sec) <
* Plus (1 sec) >

That means that like KIM UNO, you don't need the hardware to run it (well, you do need the Arduino).

In addition, you can write to the terminal from 1802 code at port 1. If you
want to read from the terminal, you can enter a | character. Once you do,
the terminal will not act as a front panel anymore.

The 1802 code can control that mode by writing a 1 to port 7 to disable the
serial front panel. A zero will reenable it.

Here's a simple terminal program:

0000: 69 32 00 64 30 00

Remember to turn the terminal front panel off with | before you try this.

LEDs
===

The far left decimal point is the Q LED.
The next is lit for load state.
The next is list for run state.
The decimal point between the two digits of the data display indicate memory protect.

Known Problems
===
The display of memory needs a little work. Select memory protect and then go to load mode, you'll see the byte at the current address. Pressing + will advance the address and show you the previous byte (so you see the first byte twice). THis will get fixed, but for now, you can just get used to it

There is no telling how many instruction miscodings I've made.


Future Plans
===
Would be nice to have a way to read/write files to the serial terminal, if present.

Would like to have an I/O port (7) to control the address display use and a way to set the address display.

Hackaday
===
Yes, there will be a Hackaday post about this.

Building
===
I used Platform.io to build. You may need to adjust the .ini file to suit.
