Send these files over the serial port (9600 8 1 N) then reset and go.

Note: To use the terminal as an ASCII input device you need to send |
one time and then it will stay in force until you reboot the device
(not an 1802 reset).

You can also turn this on programtically (and off, too) using the control
port. However, note that once you turn the terminal to this mode, you
can't upload over the serial port (until you power off/on).

QBlink.txt - Blink the Q LED fast

QBlinker.txt - Blink the LED slower

Echo.txt - Once you turn the serial terminal to ASCII (|) you can press a key to see its hex value on the LEDs. Note: this assumes you have not remapped the ports (port 1 is the terminal and port 4 is the LEDs).

Hello.txt - Say Hello!
