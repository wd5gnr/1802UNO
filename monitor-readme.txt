Enter Monitor mode with the \ key while in front panel mode. You do NOT have to switch the keyboard using the | character. Note the keyboard and display will be dead while the monitor is active.

Here's the basic help:


Commands:

Note that backspace sort of works, kind of.

R - Display all registers
R2 - Display register 2
R2=AA - Set register 2 to AA

Note: X=10, P=11, D=12, DF=13, Q=14, and T=15 -- display all registers to see that list

M 400 - Display 100 hex bytes at 400
M 400 10 - Display 10 hex bytes at 400
M 400=20 30 40; - Set data starting at 400 (end with semicolon)

G 400 - Goto 400
G 400 3 - Goto 400 with P=3

B - List all enabled breakpoints
B0 - - Disable Breakpoint 0 (that is a dash as in B0 -)
BF @200 - Set breakpoint F to address 200
BF P3 - Set breakpoint when P=3 
BF I7A - Set breakpoint when instruction 7A executes 

Note would be possible to do data write (or even read) breakpoints
Would also be possible to do rnages of addresses, if desired

N - Execute next instruction


I 2 - Show input from N=2
O 2 10 - Write 10 to output N=2

Note: The keypad and display are dead while the monitor is in control

X - Exit to front panel mode (not running)

C - Exit and continue running (if already running)
Q - Same as C

Commands:

R - Display all registers
R2 - Display register 2
R2=AA - Set register 2 to AA

Note: X=10, P=11, D=12, DF=13, Q=14, and T=15 -- display all registers to see that list

M 400 - Display 100 hex bytes at 400
M 400 10 - Display 10 hex bytes at 400
M 400=20 30 40; - Set data starting at 400 (end with semicolon)

Note that you can use the first line with full backspace:
M 400=20 30 40;

But if you start a new line (you get a colon prompt), you will not be able to backspace past the current byte:

M 400=
: 20 30 40
: 50 60 70;

Backing up while entering 30 can only delete the 30 and not the 20. Also, instead of backing up you can just keep going 
as in:

:M 400=
: 200 300 400;

All 3 bytes will then be zero.

G 400 - Goto 400
G 400 3 - Goto 400 with P=3

B - List all enabled breakpoints
B0 - - Disable Breakpoint 0 (that is a dash as in B0 -)
BF @200 - Set breakpoint F to address 200
BF P3 - Set breakpoint when P=3 
BF I7A - Set breakpoint when instruction 7A executes 

Note would be possible to do data write (or even read) breakpoints
Would also be possible to do ranges of addresses, if desired

N - Execute next instruction


I 2 - Show input from N=2
O 2 10 - Write 10 to output N=2


X - Exit to front panel mode (not running)

C - Exit and continue running (if already running)
Q - Same as C

.+ - Send next character(s) to the front panel interpreter (no spaces)

The last command is pretty handy. For example:

.44!

Will set the data input (port 64) to 44 and then display the address and data
LEDs on the terminal.

.; is also useful (set/reset trace mode)

For example, try this:

.5A
I 4

You'll see that the input reads 5A, as set.


Todo:

Need to range check certain things.

Note you still can't write to ROM.

Note that MP is still respected.

Maybe to do:
Address range breakpoints
Counted breakpoints
Data read/write breakpoints (ranged)
Rom patch system
Control commands to manipulate port 7
Control command to change display refresh interval
