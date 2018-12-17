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
private:
	int _cs;
	int _wr;
	int _dat;
	int _backlight;
	
	char _lcd_data[LCD_DATA_LEN];
};
#endif
