/*********************************************************************
 * This is a library for HT1621 based lcd drivers.
 * Datasheet: https://www.seeedstudio.com/document/HT1621.pdf
 * 
 * Original source based on the A6seglcd library  develloped by anxzhu: 
 * 	https://github.com/anxzhu/segment-lcd-with-ht1621
 * 
 ********************************************************************/
#ifndef ht1621_LCD_
#define ht1621_LCD_

							// ID,  Command Code, Description
#define  BIAS     0x52		//0b100 0010-1001-0,  1/3 Bias, 4 commons
#define  SYSDIS   0X00		//0b100 0000-0000-0,  Turn off both system oscillator and LCD bias generator
#define  SYSEN    0X02		//0b100 0000-0001-0,  Turn on system oscillator
#define  LCDOFF   0X04		//0b100 0000-0010-0
#define  LCDON    0X06		//0b100 0000-0011-0
#define  XTAL     0x28		//0b100 0001-0100-0
#define  RC256    0X30		//0b100 0001-1000-0,  System clock source, on-chip RC oscillator
#define  TONEON   0X12		//0b100 0000-1001-0
#define  TONEOFF  0X10		//0b100 0000-1000-0
#define  WDTDIS1  0X0A		//0b100 0000-0101-0,  Disable WDT time-out flag output

#define LCD_DATA_LEN 32

struct SEG {
  char addr;     //address this LCD segment resides within
  char data_pos; //bit position in data field for the LCD segment 
};

const SEG NUL_SEG = {0x0, 0x0};

//Seven segment number positions are represented
// as follows:
//        -A-     -A-
//  B    F   B   F   B
//        -G-     -G-
//  C    E   C   E   C
//        -D-     -D-
// Hund.  Tens    Ones

const SEG Y_HUNDS = {0x0A, 0x80}; //Y Hundreds "1" (B and C)
const SEG Y_TENS_A = {0x0B, 0x80}; //Y Tens A
const SEG Y_TENS_B = {0x0B, 0x40}; //Y Tens B
const SEG Y_TENS_C = {0x0B, 0x20}; //Y Tens C
const SEG Y_TENS_D = {0x0B, 0x10}; //Y Tens D
const SEG Y_TENS_E = {0x0A, 0x10}; //Y Tens E
const SEG Y_TENS_F = {0x0A, 0x40}; //Y Tens F
const SEG Y_TENS_G = {0x0A, 0x20}; //Y Tens G
const SEG Y_ONES_A = {0x0D, 0x80}; //Y Ones A
const SEG Y_ONES_B = {0x0D, 0x40}; //Y Ones B
const SEG Y_ONES_C = {0x0D, 0x20}; //Y Ones C
const SEG Y_ONES_D = {0x0D, 0x10}; //Y Ones D
const SEG Y_ONES_E = {0x0C, 0x10}; //Y Ones E
const SEG Y_ONES_F = {0x0C, 0x40}; //Y Ones F
const SEG Y_ONES_G = {0x0C, 0x20}; //Y Ones G
const SEG Y_PERCENT = {0x0C, 0x80}; //Y Percent Symbol

const SEG Y_BAR_0 = {0x0E, 0x20}; //Y Bar graph pos 0 (lowest)
const SEG Y_BAR_1 = {0x0E, 0x40}; //Y Bar graph pos 1
const SEG Y_BAR_2 = {0x0E, 0x80}; //Y Bar graph pos 2
const SEG Y_BAR_3 = {0x0F, 0x80}; //Y Bar graph pos 3
const SEG Y_BAR_4 = {0x0F, 0x40}; //Y Bar graph pos 4
const SEG Y_BAR_5 = {0x0F, 0x20}; //Y Bar graph pos 5
const SEG Y_BAR_6 = {0x0F, 0x10}; //Y Bar graph pos 6 (highest)
const SEG Y_BAR_BORDER = {0x0E, 0x10}; //Y Border around bar graph

const SEG X_BAR_0 = {0x10, 0x10}; //X Bar graph pos 0 (left most)
const SEG X_BAR_1 = {0x10, 0x20}; //X Bar graph pos 1
const SEG X_BAR_2 = {0x10, 0x40}; //X Bar graph pos 2
const SEG X_BAR_3 = {0x10, 0x80};//X Bar graph pos 3
const SEG X_BAR_4 = {0x1F, 0x40}; //X Bar graph pos 4
const SEG X_BAR_5 = {0x1F, 0x20}; //X Bar graph pos 5
const SEG X_BAR_6 = {0x1F, 0x10}; //X Bar graph pos 6 (right most)
const SEG X_BAR_BORDER = {0x1F, 0x80}; //X Border around bar graph

const SEG X_HUNDS = {0x11, 0x80}; //X Hundreds "1" (B and C)
const SEG X_TENS_A = {0x12, 0x80}; //X Tens A
const SEG X_TENS_B = {0x12, 0x40}; //X Tens B
const SEG X_TENS_C = {0x12, 0x20}; //X Tens C
const SEG X_TENS_D = {0x12, 0x10}; //X Tens D
const SEG X_TENS_E = {0x11, 0x10}; //X Tens E
const SEG X_TENS_F = {0x11, 0x40}; //X Tens F
const SEG X_TENS_G = {0x11, 0x20}; //X Tens G
const SEG X_ONES_A = {0x14, 0x80}; //X Ones A
const SEG X_ONES_B = {0x14, 0x40}; //X Ones B
const SEG X_ONES_C = {0x14, 0x20}; //X Ones C
const SEG X_ONES_D = {0x14, 0x10}; //X Ones D
const SEG X_ONES_E = {0x13, 0x10}; //X Ones E
const SEG X_ONES_F = {0x13, 0x40}; //X Ones F
const SEG X_ONES_G = {0x13, 0x20}; //X Ones G
const SEG X_PERCENT = {0x15, 0x20}; //X Percent Symbol

const SEG VIDEO = {0x15, 0x10}; //Video Icon
const SEG CAMERA = {0x15, 0x40}; //Photo Icon

const SEG VOLT_ONES_A = {0x19, 0x10}; //Voltage Ones A
const SEG VOLT_ONES_B = {0x18, 0x10}; //Voltage Ones B
const SEG VOLT_ONES_C = {0x18, 0x40}; //Voltage Ones C
const SEG VOLT_ONES_D = {0x19, 0x80}; //Voltage Ones D
const SEG VOLT_ONES_E = {0x19, 0x40}; //Voltage Ones E
const SEG VOLT_ONES_F = {0x19, 0x20}; //Voltage Ones F
const SEG VOLT_ONES_G = {0x18, 0x20}; //Voltage Ones G
const SEG VOLT_DP = {0x18, 0x80}; //Voltage Decimal Point
const SEG VOLT_TENT_A = {0x17, 0x10}; //Voltage Tenths A
const SEG VOLT_TENT_B = {0x16, 0x10}; //Voltage Tenths B
const SEG VOLT_TENT_C = {0x16, 0x40}; //Voltage Tenths C
const SEG VOLT_TENT_D = {0x17, 0x80}; //Voltage Tenths D
const SEG VOLT_TENT_E = {0x17, 0x40}; //Voltage Tenths E
const SEG VOLT_TENT_F = {0x17, 0x20}; //Voltage Tenths F
const SEG VOLT_TENT_G = {0x16, 0x20}; //Voltage Tenths G
const SEG VOLT_LABEL = {0x16, 0x80}; //Voltage "V" label

const SEG SPEED_0 = {0x1B, 0x10}; //Speedometer pos 0 (left most)
const SEG SPEED_1 = {0x1B, 0x20}; //Speedometer pos 1
const SEG SPEED_2 = {0x1B, 0x40}; //Speedometer pos 2
const SEG SPEED_3 = {0x1B, 0x80}; //Speedometer pos 3
const SEG SPEED_4 = {0x1E, 0x80}; //Speedometer pos 4
const SEG SPEED_5 = {0x1E, 0x40}; //Speedometer pos 5
const SEG SPEED_6 = {0x1E, 0x20}; //Speedometer pos 6
const SEG SPEED_7 = {0x1E, 0x10}; //Speedometer pos 7
const SEG SPEED_8 = {0x1A, 0x20}; //Speedometer pos 8
const SEG SPEED_9 = {0x1A, 0x40}; //Speedometer pos 9 (right most)
const SEG SPEED_BORDER = {0x1A, 0x10}; //Speedometer labels
const SEG SPEED_KMH = {0x1A, 0x80}; //Speedometer KM/H label

const SEG RADIO_0 = {0x1d, 0x40}; //Signal meter pos 0 (left most)
const SEG RADIO_1 = {0x1d, 0x20}; //Signal meter pos 1
const SEG RADIO_2 = {0x1d, 0x10}; //Signal meter pos 2
const SEG RADIO_3 = {0x1c, 0x10}; //Signal meter pos 3
const SEG RADIO_4 = {0x1c, 0x20}; //Signal meter pos 4 (right most)
const SEG RADIO_MODE1 = {0x1c, 0x40}; //"Mode1" Indicator
const SEG RADIO_ANT = {0x1d, 0x80}; //Signal meter Antenna Symbol

class  ht1621_LCD
{
public:
	int cs;
	int wr;
	int dat;
	int backlight;
	ht1621_LCD();
	void setup(int cs, int wr, int dat, int backlight);
	void setup(int cs, int wr, int dat);
	void conf();
	void display(unsigned char addr, unsigned char sdata);//
	void backlighton();//
	void backlightoff();//
	void wrone(unsigned char addr, unsigned char sdata);
	void wrclrdata(unsigned char addr, unsigned char sdata);
	void wrDATA(unsigned char data, unsigned char cnt);
	void wrCMD(unsigned char CMD);
	void lcdon();
	void lcdoff();
	void setAll(char val);
	void update();
	void setByte(int address, char val);
	char getByte(int address);
	void setBits(int address, char val);
	void clearBits(int address, char val);
	void setSeg(SEG s);
	void clearSeg(SEG s);
private:
	int _cs;
	int _wr;
	int _dat;
	int _backlight;
	
	char _lcd_data[LCD_DATA_LEN];
};
#endif
