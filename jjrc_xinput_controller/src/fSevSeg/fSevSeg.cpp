// Original source forked from: https://github.com/sparkfun/SevSeg/blob/master/src/SevSeg.h
//   Written by Dean Reading, 2012.  deanreading@hotmail.com
//
// @Author: james@team2168.org

#include "fSevSeg.h"

fSevSeg::fSevSeg() {
  //Initial values
	numberOfDigits = 0;
}

//
void fSevSeg::setup(ht1621_LCD* lcd, DIGIT dig1, DIGIT dig2, DIGIT dig3, boolean decimalPoint) {
  //Bring all the variables in from the caller
  digit1 = dig1;
  digit2 = dig2;
  digit3 = dig3;
	decPoint = decimalPoint;
	numberOfDigits = 3;
	_lcd = lcd;
}

void fSevSeg::setup(ht1621_LCD* lcd, DIGIT dig1, DIGIT dig2, boolean decimalPoint) {
	setup(lcd, dig1, dig2, NUL_DIGIT, decimalPoint);
	numberOfDigits = 2;
}

void fSevSeg::turnOffDigit(DIGIT digit) {
	//If the "b" segment doesn't have a valid bit position defined, assume this is
	//  a "NUL_DIGIT" and don't bother zeroing it out, otherwise, zero all SEGs.
	//if(digit.B.data_pos != NUL_SEG.data_pos){
		_lcd->clearSeg(digit.A);
		_lcd->clearSeg(digit.B);
		_lcd->clearSeg(digit.C);
		_lcd->clearSeg(digit.D);
		_lcd->clearSeg(digit.E);
		_lcd->clearSeg(digit.F);
		_lcd->clearSeg(digit.G);
	//}
}

void fSevSeg::turnOnDigit(DIGIT digit) {
	//If the "b" segment doesn't have a valid bit position defined, assume this is
	//  a "NUL_DIGIT" and don't bother zeroing it out, otherwise, zero all SEGs.
	//if(digit.B.data_pos != NUL_SEG.data_pos){
		_lcd->setSeg(digit.A);
		_lcd->setSeg(digit.B);
		_lcd->setSeg(digit.C);
		_lcd->setSeg(digit.D);
		_lcd->setSeg(digit.E);
		_lcd->setSeg(digit.F);
		_lcd->setSeg(digit.G);
	//}
}

//Refresh Display
/*******************************************************************************************/
void fSevSeg::DisplayString(String s) {
	DIGIT d;
	String val;

	//Start by tuning all segments off
	turnOffDigit(digit1);
	turnOffDigit(digit2);
	turnOffDigit(digit3);

	//Sanitize the passed string.
	//TODO: replace with a loop that prepends string with BLANKS if passed string length is shorter than segment.
	switch(s.length()) {
		case 1:
			if(numberOfDigits == 2) {
				val = String(" " + s);
			} else {
				val = String("  " + s);
			}
			break;
		case 2:
			if(numberOfDigits == 2) {
				val = s;
			} else {
				val = String(" " + s);
			}
			break;
		case 3:
			//TODO: Fix case for 2 digit length segment... 
			val = s;
			break;
		default:
			val = String("   ");
			break;
	}


	//Loop through the digits and display the appropriate character for each position.
	for(int digit = 0; digit < numberOfDigits; digit++) {
		switch(digit){
			case 0: //First character
				d = digit1;
			  break;
			case 1: //Second character
				d = digit2;
				break;
			case 2: //Thirdd character
				d = digit3;
				break;
			default: //shouldn'at get here
				d = NUL_DIGIT;
				break;
		}

		//Here we access the array of segments
		//This could be cleaned up a bit but it works
		//displayCharacter(toDisplay[digit]); //Now display this digit
		// displayArray (defined in SevSeg.h) decides which segments are turned on for each number or symbol
		unsigned char characterToDisplay = val.charAt(digit);

		const uint8_t chr = pgm_read_byte(&characterArray[characterToDisplay]);
		if (chr & (1<<6)) _lcd->setSeg(d.A);
		if (chr & (1<<5)) _lcd->setSeg(d.B);
		if (chr & (1<<4)) _lcd->setSeg(d.C);
		if (chr & (1<<3)) _lcd->setSeg(d.D);
		if (chr & (1<<2)) _lcd->setSeg(d.E);
		if (chr & (1<<1)) _lcd->setSeg(d.F);
		if (chr & (1<<0)) _lcd->setSeg(d.G);
	}
}

void fSevSeg::DisplayInt(int i) {
	DisplayString(String(i));
}

void fSevSeg::DisplayIntHex(int i) {
	DisplayString(String(i, HEX));
}