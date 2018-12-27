/*********************************************************************
 * This is a library for HT1621 based lcd drivers.
 * Datasheet: https://www.seeedstudio.com/document/HT1621.pdf
 * 
 * Original source based on the A6seglcd library  develloped by anxzhu: 
 * 	https://github.com/anxzhu/segment-lcd-with-ht1621
 * 
 ********************************************************************/

#include <Arduino.h>
#include "ht1621_LCD.h"
#include "stdio.h"

ht1621_LCD::ht1621_LCD() {
}

void ht1621_LCD::setup(int cs, int wr, int dat, int backlight) {
	_cs=cs;
	_wr=wr;
	_dat=dat;
	_backlight=backlight;

	pinMode(_cs, OUTPUT);
	digitalWrite(_cs, HIGH);
	pinMode(_wr, OUTPUT);
	digitalWrite(_wr, HIGH);
	pinMode(_dat, OUTPUT);
	digitalWrite(_dat, HIGH);
	if (_backlight > 0) {
		pinMode(_backlight, OUTPUT);
		//digitalWrite(_backlight, HIGH);
	}

	//initialize the LCD data buffer
	setAll(0x00);

	delay(100);
}
void ht1621_LCD::setup(int cs, int wr, int dat) {
	setup(cs, wr, dat, -1);
}
void ht1621_LCD::wrDATA(unsigned char data, unsigned char cnt) {
	unsigned char i;
	for (i = 0; i < cnt; i++) {
		digitalWrite(_wr, HIGH);
		digitalWrite(_wr, HIGH);
		digitalWrite(_wr, HIGH);
		if (data & 0x80) {
			digitalWrite(_dat, HIGH);
			digitalWrite(_dat, HIGH);
			digitalWrite(_dat, HIGH);
			digitalWrite(_dat, HIGH);
			digitalWrite(_dat, HIGH);
			digitalWrite(_dat, HIGH);
		} else {
			digitalWrite(_dat, LOW);
			digitalWrite(_dat, LOW);
			digitalWrite(_dat, LOW);
			digitalWrite(_dat, LOW);
			digitalWrite(_dat, LOW);
			digitalWrite(_dat, LOW);
		}
		digitalWrite(_wr, LOW);
		digitalWrite(_wr, LOW);
		digitalWrite(_wr, LOW);
		digitalWrite(_wr, LOW);
		digitalWrite(_wr, LOW);
		digitalWrite(_wr, LOW);
		digitalWrite(_wr, LOW);
		digitalWrite(_wr, LOW);
		digitalWrite(_wr, LOW);
		data <<= 1;
	}
	digitalWrite(_wr, HIGH);
	digitalWrite(_wr, HIGH);
	digitalWrite(_wr, HIGH);
	digitalWrite(_wr, HIGH);
	digitalWrite(_wr, HIGH);
	digitalWrite(_wr, HIGH);
	digitalWrite(_wr, HIGH);
	digitalWrite(_wr, HIGH);
	digitalWrite(_wr, HIGH);
	digitalWrite(_wr, HIGH);
	digitalWrite(_wr, HIGH);
	digitalWrite(_wr, HIGH);
}
void ht1621_LCD::wrclrdata(unsigned char addr, unsigned char sdata)
{
	addr <<= 2;
	digitalWrite(_cs, LOW);
	digitalWrite(_cs, LOW);
	digitalWrite(_cs, LOW);
	digitalWrite(_cs, LOW);
	digitalWrite(_cs, LOW);
	digitalWrite(_cs, LOW);
	digitalWrite(_cs, LOW);
	wrDATA(0xa0, 3);
	wrDATA(addr, 6);
	wrDATA(sdata, 4);
	digitalWrite(_cs, HIGH);
	digitalWrite(_cs, HIGH);
	digitalWrite(_cs, HIGH);
	digitalWrite(_cs, HIGH);
	digitalWrite(_cs, HIGH);
	digitalWrite(_cs, HIGH);
}

void ht1621_LCD::lcdon() {
	wrCMD(LCDON);
}

void ht1621_LCD::lcdoff() {
	wrCMD(LCDOFF);
}

void ht1621_LCD::wrone(unsigned char addr, unsigned char sdata) {
	addr <<= 2;
	digitalWrite(_cs, LOW);
	wrDATA(0xa0, 3);
	wrDATA(addr, 6);
	wrDATA(sdata, 4);
	digitalWrite(_cs, HIGH);
}
void ht1621_LCD::backlighton() {
	if(_backlight > 0) {
		digitalWrite(_backlight, HIGH);
		delay(1);
	}
}
void ht1621_LCD::backlightoff() {
	if(_backlight > 0) {
		digitalWrite(_backlight, LOW);
		delay(1);
	}
}
void ht1621_LCD::wrCMD(unsigned char CMD) {  //100
	digitalWrite(_cs, LOW);
	wrDATA(0x80, 4);
	wrDATA(CMD, 8);
	digitalWrite(_cs, HIGH);
}
void ht1621_LCD::conf() {
	wrCMD(RC256);
	wrCMD(BIAS);
	wrCMD(SYSDIS);
	wrCMD(WDTDIS1);
	wrCMD(SYSEN);
	wrCMD(LCDON);
	
}
void ht1621_LCD::display(unsigned char addr, unsigned char sdata){
	wrone(addr,sdata);
}

/**
 * Set the entire contents of the local LCD memory buffer to val.
 */
void ht1621_LCD::setAll(char val) {
	for(int i=0; i<LCD_DATA_LEN; i++) {
		_lcd_data[i] = val;
	}
}

/**
 * Write out entire contents of local LCD memory buffer to the display.
 */
void ht1621_LCD::update() {
	for(int i=0; i < LCD_DATA_LEN; i++) {
		wrclrdata(i, _lcd_data[i]);
	}
}

/**
 * Set the data value of a specific address in the local LCD memory buffer.
 * Call refresh to dump the buffer to the LCD.
 */ 
void ht1621_LCD::setByte(int address, char val) {
	if(address < LCD_DATA_LEN) {
		_lcd_data[address] = val;
	}
}

char ht1621_LCD::getByte(int address) {
	char ret = 0x00;
	if(address < LCD_DATA_LEN) {
		ret = _lcd_data[address];
	}
	return ret;
}

/**
 * Masks-in set bits into the LCD local memory buffer. Any zeroed bit positions
 *   in val will stay unmodified in the LCD local memory buffer.
 */
void ht1621_LCD::setBits(int address, char val) {
	char foo = 0x00;
	if(address < LCD_DATA_LEN) {
		foo = getByte(address) | val;
		setByte(address, foo);
	}
}

/**
 * Mask-out (clear) the non-zero bits passed in 'val'. Any zeroed bit posiitons
 *   in val will stay unmodified in the LCD local memory buffer.
 */
void ht1621_LCD::clearBits(int address, char val) {
	if(address < LCD_DATA_LEN) {
		setByte(address, getByte(address) & (~val)); 
	}
}


/**
 * Helper function to turn on a specified LCD segment.
 */
void ht1621_LCD::setSeg(SEG s) {
  setBits(s.addr, s.data_pos);
}

/**
 * Helper function to turn off a specified LCD segment.
 */
void ht1621_LCD::clearSeg(SEG s) {
  clearBits(s.addr, s.data_pos);
}