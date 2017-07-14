1802 UNO v9
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
* DA - While idle, enter load mode
* PC - Protect memory toggle so that load mode displays data
* SST - Single step
* 0-F - Build up hex number. Accumulates 16-bits although you can only see the lower 8. For load mode, the lower 8 is used. For AD all 16-bits are used.
* SST - Hold down for one second to save RAM to EEPROM
* AD - Hold down for one second when not running and not memory protected to read RAM from EEPROM


Serial Port
===
On a terminal (9600 8 N 1) you can use normal keys like 0-9 A-F a-f + and these additional keys:

* ST=S
* RS=R
* AD=Equal sign
* DA=L
* GO=G
* PC=P
* SST=/
* DA (1 sec) <
* AD (1 sec) >

That means that like KIM UNO, you don't need the hardware to run it (well, you do need the Arduino).

Other Serial Commands
===
* pipe - Go into serial terminal mode (until power cycle)
* semicolon - Toggle trace mode (warning: makes execution slow). Prints address, opcode, and D on each instruction execution
* asterisk - Dump registers and state
* ? - Dump 1K of RAM in 1802UNO Format (see below)
* @ - Load RAM in 1802UNO Format (see below and examples directory; also see binto1802.c)
* X - Load RAM from Intel hex file

Using the Serial Port from an 1802 Program
===
In addition, you can write to the terminal from 1802 code at port 1. If you
want to read from the terminal, you can enter a | character. Once you do,
the terminal will not act as a front panel anymore.

The 1802 code can control that mode by writing a 1 to port 7 to disable the
serial front panel. A zero will reenable it.

There is a simple terminal program in the examples directory called Echo.txt. Remember to turn the terminal front panel off with | before you try this.

Loading and Saving RAM
===
With a serial port connected, you can send the ? command to get a dump of all 1K of memory.

You can also set memory by sending back the string returned by the ? command. You can also make your own string using the following format:

@address:byte byte byte byte .

Everything is in ASCII, so "@0000:7A 7B ." is 13 bytes. Note that you don't need all the digits (e.g., "@0:5 2 1 FF ." is OK. Also you must have a space betwween bytes and before the period which ends the transmission. The only characters that matter are the @ sign, the period, and the hex digits (upper or lower case). So you could just as well say "@0000=7a,7b&." if you wanted to.

LEDs
===

The far left decimal point is the Q LED.
The next is lit for load state.
The next is list for run state.
The decimal point between the two digits of the data display indicate memory protect.

Known Problems
===
* There is no telling how many instruction miscodings I've made. Of course, that's not technically a "known" problem.


Future Plans
===
* Would like to have an I/O port (7) to control the address display use and a way to set the address display.

Hackaday
===
Yes, there will be a Hackaday post about this.

Building
===
I used Platform.io to build. You may need to adjust the .ini file to suit.

Tool
===
The file binto1802 will convert a binary file into a loadable file at address 0. Of course, you can edit the file and change this address. You can compile this tool with:

     gcc -o binto1802 binto1802.c

If you have an Intel hex file or other format, you can convert it to binary with srec_cat.

A Note About Compatiblity
===
There is quite a bit of software out there for the 1802 that uses a terminal
such as monitors, Basic, Forth, etc. In many cases, these programs expect
to drive their own serial port via the Q and EF ports. This approach won't
work with the simulator. I may try to port or recreate parts of Riley's BIOS althoug it will require being "ROMed" as the simulator only has 1K of RAM.

Of course, if you have the source to a program you can change it to use the serial ports (very easy to read and write serial compared to bit-banging).



