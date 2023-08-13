/* Configuration stuff for 1802UNO */
// Configuration

// Define to 1 for simulation monitor built-in
#define MONITOR 1
#define BIOS 1  // partial BIOS implementation (experimental)
// remove this to cut out OLED code -- if you don't have the OLED you must not compile this
// or the machine will hang
// - NOTE: if you have an OLED connected, you MUST define OLED. If you don't, you MUST undefine it.   !!!!!!!!!!!!!!!!!!!!!! READ THIS !!!!!!!!!!!!!!!!!!!!
//#define OLED



#if BIOS==1
#if MONITOR==0
#error Must enable MONITOR when BIOS enabled
#endif
#endif



// Note: MAXMEM is a mask, so don't make it something like 0x500
// Since the Arduino Pro Mini has 2K of RAM--and we use a good bit of that
// this will probably never be more than 0x3FF and certainly could not be
// more than 0x7FF (which would mean on RAM for the rest of the program!)
// So in reality, probably 0x3FF or less unless you port to a CPU
// with more RAM
#define MAXMEM 0x3FF  // maximum memory address; important: only 1K of EEPROM to store stuff in and not much RAM anyway
#define LED_PORT 4     // port for DATA LED display
#define SW_PORT 4      // Front panel switch port
#define CTL_PORT 7    // Control port
#define A0_PORT 2     // LSD address display output port
#define A1_PORT 3     // MSD address display output port
#ifdef OLED
#define PIXIEPORT 1
#define SER_INP 6
#define SER_OUT 6
#else
#define SER_INP 1     // UART input port
#define SER_OUT 1     // UART output port
#endif

// How many cycles between display refreshes?
// Higher makes the simulation faster, but the screen more blinky
#define DISPLAY_DIVISOR 32  // number of ticks between display refresh
#define NICE_VALUE 40 // number of times to execute instructions while lighting LEDs (0 to disable)
