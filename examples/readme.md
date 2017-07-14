Send these files over the serial port (9600 8 1 N) then reset and go.

Note: To use the terminal as an ASCII input device you need to send |
one time and then it will stay in force until you reboot the device
(not an 1802 reset).

You can also turn this on programtically (and off, too) using the control
port. However, note that once you turn the terminal to this mode, you
can't upload over the serial port (until you power off/on).

* QBlink.txt - Blink the Q LED fast

* blink.hex - Same thing in Intel hex format (use X command to load)

* QBlinker.txt - Blink the LED slower

* Echo.txt - Once you turn the serial terminal to ASCII (|) you can press a key to see its hex value on the LEDs. Note: this assumes you have not remapped the ports (port 1 is the terminal and port 4 is the LEDs).

* Hello.txt - Say Hello!

* dice.txt - From QuestData. Press Go to roll dice and stop to see the value. NOTE: this works because P=0 and X=0 at reboot! So 64 is an immediate output command when X==P==0 since OUT reads M[R[X]] and does R[X]++. How cool!

* bcd.txt - My first program published in QuestData! You do your decimal data entry (00-99) and press GO to see the result. Now you can enter a new value and press enter. The logic, by the way, is 2*(M/4+M/16)+L. Note that this is the same as 2*(5*M/16)+L or 10*M/16+L. Where M=the top digit unshifted and L is the bottom digit. So for number 92, M=90 and L=2 (hex). Remember 16 is 10 hex so in hex you get 0A*90/10+2.

* bcd.hex - Same as above in Intel hex file format.

* bcd1.txt - Same as above but uses IDL to stop the processor while waiting for input. Press run to continue.
