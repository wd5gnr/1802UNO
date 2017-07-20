1802 UNO v20
===
Starting with Oscar's KIM-UNO code, I changed out the 6502 for an 1802.
See: <http://obsolescence.wixsite.com/obsolescence/kim-uno-summary-c1uuh> for more details.


Configuration
===
You have 1K of RAM at 0000 and no interrupts.
There is a small ROM at 8000
To set the ROM you must reflash the Arduino
The various commands to save and read memory only operate on RAM
You can "LOad" through the ROM but it won't change the contents

You can load ETOPS in rom (see http://www.elf-emulation.com/software/rctops.html and below)
or the IDIOT/4 monitor in rom (see below)
By default a HiLo game is in ROM (see below)

To run it put C0 80 00 at location 0 to jump to it. Note it uses RAM at 03FF.
On power up (but not reset) the first 3 bytes of RAM initialize to C0 80 00.

The file 1802rom.h only includes another file (1802idiot.h, 1802hilo.h, or 1802etops.h) so it is reasonably easy to flip different ROM images around.

Keyboard
===
The keyboard is mapped like this:

* Go - Run
* ST - Stop running or stop load mode
* RS - Reset
* AD - Copy extended data register to load address
* \+ - EF4 (Input key for program or load mode enter)
* DA - While idle, enter load mode
* PC - Protect memory toggle so that load mode displays data
* SST - Single step
* 0-F - Build up hex number. Accumulates 16-bits although you can only see the lower 8. For load mode, the lower 8 is used. For AD all 16-bits are used.
* SST - Hold down for one second to save RAM to EEPROM
* AD - Hold down for one second when not running and not memory protected to read RAM from EEPROM


Serial Port
===
On a terminal (9600 8 N 1) you can use normal keys like 0-9 A-F a-f and these additional keys:

* ST=S
* RS=R
* AD=Equal sign (=)
* DA=L
* GO=G
* PC=P
* SST=/
* DA (1 sec) <
* AD (1 sec) >

Note: + does not act as Enter from the terminal; use $ to toggle EF4 instead.

That means that like KIM UNO, you don't need the hardware to run it (well, you do need the Arduino).

Other Serial Commands
===
* \| - Go into serial terminal mode (until power cycle)
* \; - Toggle trace mode (warning: makes execution slow). Prints address, opcode, and D on each instruction execution
* \* - Dump registers and state
* \! - Dump address and data displays to terminal
* ? - Dump 1K of RAM in 1802UNO Format (see below)
* $ - Set EF4 on/off (overrides HW keyboard)
* @ - Load RAM in 1802UNO Format (see below and examples directory; also see binto1802.c)
* X - Load RAM from Intel hex file
* Y - Write 1K RAM to Intel hex file (hint, you can delete all the zero lines and keep the last EOF line using a text editor)
* \\ - Enter monitor mode. This is a meta-monitor running in the host Arduino. See section below for more details

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

You can also read and write Intel hex files with X and Y commands. Note that Y writes out all 1K of RAM. The last line is an EOF record and you can delete any lines you don't care about. So if you dump a simple program that takes, say, 30 bytes, you can keep the first two lines and the last line and delete the rest using a text editor.


LEDs
===

The far left decimal point is the Q LED.
The next is lit for load state.
The next is lit for run state.
The next is list when EF4 is active.
The decimal point between the two digits of the data display indicate memory protect.

Known Problems
===
* There is no telling how many instruction miscodings I've made. Of course, that's not technically a "known" problem.


Future Plans
===
* All done for now.

Hackaday
===
Yes, there will be a Hackaday post about this.

Port Summary
===
* Port 1 - Serial port
* Port 2 - LSD of address display (if enabled)
* Port 3 - MSD of address display (if enabled)
* Port 4 - Switch/Data LED
* Port 7 - Control port. Set bit 0 to disable serial front panel. Set bit 1 to put address displays under program control (see port 2,3).


Building
===
I used Platform.io to build. You may need to adjust the .ini file to suit. See http://platformio.org.

To build and upload try:

    pio run --target upload

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
work with the simulator. I may try to port or recreate parts of Riley's BIOS although it will require being "ROMed" as the simulator only has 1K of RAM.

Of course, if you have the source to a program you can change it to use the serial ports (very easy to read and write serial compared to bit-banging).

Available ROMS
===
* 1802hilo.h - A hilo game (the default)
* 1802etops.h - A keyboard/LED-based monitor
* 1802idiot.h - A monitor that uses the serial port

Using IDIOT/4
===
IDIOT/4 is a monitor by Lee Hart. I hacked it up to use the virutalized serial port instead of bit banging. You need a terminal at 9600 8 N and 1 and you'll want local echo on.

You have to switch the terminal to raw mode (| command) before you can issue commands to the monitor. That change stays in force until you reboot the simulator (not press the reset button). Then you can no longer use the front panel commands.

So the general work flow would be:

G |

You'll get a sign on message and a prompt. The basic commands are dump memory:

?M 8000 20

Dump registers (shows them in memory; you need a decoder ring to know which is which)

?R

Load memory 
!M 300 20 40 10

You can use a semicolon to keep going after a line (e.g., !M 300 20 40 10;)
(NOTE: That seems to not work; perhaps a bug in the emulator or in my port
of the monitor code)

Run a program at 100.

$P 100

There are more things you can do. See http://www.retrotechnology.com/memship/mship_idiot.html for a good write up.

NOTE: IDIOT scans backwards for memory. The simulator maps the 1K block to every 1K block from 0000-7FFF. So it will find the RAM at 7C00 which is fine.

Sample ETOPS Session
===
From terminal enter:

    LC0$$80$$00$$R
    02G
    01$
    $00$
    $
    DE$
    $AD$
    $BE$
    $EF$
    $
    R
    01G
    01$
    $00$
    $
    $
    $
    $
    $
    $
    $
    $
    $R

Note that if you send the $ characters too fast, you may get such a fast press of EF4 that the 1802 software running will miss.

The first line loads the jump to the ETOPS monitor.

The 02G tells ETOPS to load memory. The address is set to 0100.

Then the four bytes DE AD BE EF are set.

A reset and then an ETOP view command (01G) follows. After entering the
same address, you should see the bytes entered appear again.

Of course, you can do all this from the hardware keyboard, as well:

    DA C 0 + 8 0 + 0 0 + RS
    0 2 GO
    0 1 + 0 0 +
    D E + A D + B E + E F +
    RS
    0 1 GO
    0 1 + 0 0 +
    \+ + + + RS

Playing HiLo
===
See https://groups.yahoo.com/neo/groups/cosmacelf/files/HI%20LO/

The original instructions have to be modified a bit. Be sure and hold the
\+ key down long enough for the Q LED to light when guessing. The blink blocks
so it is easy to press it and release before the 1802 "looks" at it.

In addition, the LEDs don't "bink" because they are 7 segment. What you will see is for a high you will see 00/F0 for a low you will see 00/0F and when the nubmer is correct you will see it blink between 00 and the number.

Once the game is over and it shows you the # of guesses you made, you'll have to press RESET and run for a new game.

Note the code required a little patching to move the data out of ROM.

Here's the original "read me" text for the game:

>ELF HI LO Game Instructions:
>
>The purpose of this game is to guess a number selected by the computer between 1 and 100 in as few tries as possible.
>
>Reset and Run the ELF. All the LED's will be blinking. Press I and the computer will select a secret number, then the Q LED will come on, indicating that your input is needed. Enter a number using the toggle switches then press I. 
>
>At that point, one of 3 things can happen:
>
>1.- Your number is higher than the secret number, and so the HIGH nibble on the LED display (i.e. the leftmost 4 LED's) will blink
>
>2.- Your number is lower than the secret number and the LOW nibble on the LED display (i.e. the rightmost 4 LED's) will blink
>
>3.- Your guessed the correct number and it will appear on the LED display and blink
>
>Press I again and the Q LED will come on and you will be able to enter another guess as before. If you guessed the correct number, then the number of tries will be displayed and blinked and the game will be over. 
>
>Hope you like it.
>
>Walid Maalouli

Built In Monitor
===
Enter Monitor mode with the \ key while in front panel mode. You do NOT have to switch the keyboard using the | character. Note the keyboard and display will be dead while the monitor is active.

Upper/lower case does not matter. Note that backspace sort of works, except on multi-line M commands. Even then, it works, just with a twist (see the M command for more). You can use Escape to abort a line. Esc will also abort a long memory dump.

You can #define MONITOR 0 in main.h if you want to disable it.


Commands 
---
Note: lower case letters represent numbers

* B - List all breakpoints
* B n - List breakpoint n (0-F)
* B n - - (that's a dash after the number) Disable breakpoint n
* B n @aaaa - Break at address a, breakpoint n
* B n Pp - Break when P becomes equal to p
* B n Iii - Break when current instruction is ii
* C - Continue execution (assuming 1802 is running; same as Q)
* G aaaa - Goto address
* G aaaa p - Set P to p and goto address
* I n - Input from port n
* M aaaa - Display 256 bytes from address aaaa
* M aaaa nnn - Display nnn bytes from address aaaa
* M aaaa=nn nn nn; - Set bytes starting at aaaa (multiple lines allowed with semicolon only on last line; see notes below)
* N - Execute next instruction
* O n bb - Output byte bb to port n
* Q - Quit. Will resume execution if 1802 is running.
* R - Display all registers
* R n - Display register N (note 10 and above are special registers, see below.
* R n=vvvv - Set register n to value vvvv
* X - Exit. This will not resume execution.
* . - Dot command. Sends next characters to the front panel simulation. That is, .$ toggles EF4. .41$ enters the number 41 on the keypad and presses (but does not release) EF4
* ? - Very basic help message

Registers
---
In addition to working with registers 0-F, you can access other registers using numbers larger than 0F.

    R0:8042	R1:00FF
    R2:00FE	R3:0002
    R4:80F0	R5:800B
    R6:8064	R7:00FD
    R8:0000	R9:0000
    RA:0000	RB:0000
    RC:0000	RD:0000
    RE:0000	RF:0000
    (10) X:7	(11) P:6
    (12) D:0B	(13) DF:0
    (14) Q:0	(15) T:0

Note that R12 is the accumulator "D" not register D. You can see the list by using the R command.

Note, too, that the parens mess up the GitHub markdown formatting, but escaping them doesn't work either (actually, it fixes the formatting but the \\ characaters appear in the text).

Examples
---
    R2=AA - Set register 2 to AA


    M 400 - Display 100 hex bytes at 400
    M 400 10 - Display 10 hex bytes at 400
    M 400=20 30 40; - Set data starting at 400 (end with semicolon)

    G 400 - Goto 400
    G 400 3 - Goto 400 with P=3

    BF I7A - Set breakpoint when instruction 7A executes 

    I 2 - Show input from N=2
    O 1 41 - Write 41 to output N=2 (this will echo on terminal)

Notes about the M command
---
You can enter new bytes on one line:
    M 400=7A 7b 30 00;

Note that there is a limit on line size (currently 31 characters).

However, if you start a new line (you get a colon prompt), you will not be able to backspace past the current byte:

    M 400=
    : 20 30 40
    : 50 60 70;

Backing up while entering 30 can only delete the 30 and not the 20. Also, instead of backing up you can just keep going 
as in:

    :M 400=
    : 200 300 400;

All 3 bytes will then be zero.

About the dot command
---
The dot command is pretty handy. For example:

.44!

Will set the data input (port 4) to 44 and then display the address and data
LEDs on the terminal.

.; is also useful (set/reset trace mode)

For example, try this:

.5A
I 4

You'll see that the input reads 5A, as set.

For fun while playing HiLo, try this (from front panel mode):

    \
    O 7 2
    o 2 14
    o 3 07
    c

Now look at your 1802 UNO upside down!


Hard Breakpoint
---
Code a 68 (an illegal 1802 instruction) to force a jump to the built-in
monitor.
