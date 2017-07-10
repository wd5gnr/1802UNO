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

That means that like KIM UNO, you don't need the hardware to run it (well, you do need the Arduino).

LEDs
===

The far left decimal point is the Q LED.
The next is lit for load state.
The next is list for run state.
The decimal point between the two digits of the data display indicate memory protect.

Hackaday
===
Yes, there will be a Hackaday post about this.

