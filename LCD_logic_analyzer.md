Data captured probing the DATA/WR/CS lines between the stock microcontroller and the LCD.
Details on the LCD communication protocol can be found in the [HT1621 datasheet](https://www.seeedstudio.com/document/HT1621.pdf).

# Power on
The following commands are sent after power is applied to the LCD to configure/initialize the LCD.  

| ID  | COMMAND CODE | DESCRIPTION |
| --- | ------------ | ----------- |
| 100 | 0001-1000-0  | System clock source, on-chip RC oscillator |
| 100 | 0010-1001-0  | LCD 1/3 bias option. 4 Commons option |
| 100 | 0000-0000-0  | Turn off both system oscillator and LCD bias generator |
| 100 | 0000-0101-0  | Disable WDT time-out flag output |
| 100 | 0000-0001-0  | Turn on system oscillator |
| 100 | 0000-0011-0  | Turn on LCD bias generator |

Then a number of RAM writes occur, to set the contents of RAM & LCD to a known state.  
The format of the command code for ID 101 is 6 address bits followed by 4 data bits (A5A4A3A2A1A0-D0D1D2D3).

| ID  | COMMAND CODE | DESCRIPTION |
| --- | ------------ | ----------- |
| 101 | 000000-0000  | Turn off all the LCD segments |
| 101 | 000001-0000  | Turn off all the LCD segments |
| 101 | 000010-0000  | Turn off all the LCD segments |
| 101 | 000011-0000  | Turn off all the LCD segments |
| ... | ...          | Get the idea? |
| 101 | 011111-0000  | Turn off all the LCD segments |
| 101 | 000000-1111  | Turn on all the LCD segments |
| 101 | 000001-1111  | Turn on all the LCD segments |
| 101 | 000010-1111  | Turn on all the LCD segments |
| 101 | 000011-1111  | Turn on all the LCD segments |
| ... | ...          | Get the idea? |
| 101 | 011111-1111  | Turn on all the LCD segments |

On power-up, the LCD does start with all segments turned on for a few seconds, so this looks to be consistent with what the data capture shows.


# Normal operation
It looks like the following bulk writes are sent to the LCD roughly every ~50.5 mS.  
Both wheel and trigger are at 50% (center) positions. 6VDC supplied.  
All non-listed addresses were zero ("0000").  

| ID  | COMMAND CODE | DESCRIPTION |
| --- | ------------ | ----------- |
| 101 | 000000-0000  | |
| ... | ...  | |
| 101 | 001010-0110  | |
| 101 | 001011-1011  | |
| 101 | 001100-1101  | |
| 101 | 001101-1111  | |
| 101 | 001110-0001  | |
| 101 | 001111-1000  | |
| 101 | 010000-1000  | |
| 101 | 010001-0110  | |
| 101 | 010010-1011  | |
| 101 | 010011-0101  | |
| 101 | 010100-1111  | |
| 101 | 010101-0010  | |
| 101 | 010110-1101  | |
| 101 | 010111-1111  | |
| 101 | 011000-1111  | |
| 101 | 011001-1111  | |
| 101 | 011010-1001  | |
| 101 | 011011-0000  | |
| 101 | 011100-0111  | |
| 101 | 011101-1111  | |
| 101 | 011110-0000  | |
| 101 | 011111-1000  | |



Once I have the LCD code running on the Teensy, I'll be able to walk the memory addresses and correlate them to LCD segments.
