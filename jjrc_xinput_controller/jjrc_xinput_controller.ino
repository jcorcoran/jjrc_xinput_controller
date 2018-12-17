//Teensy based RC controller for FRC using XBOX Controller Driver (XINPUT) 
//Supports buttons, analog sticks, and rumble.
//Originally implemented using the JJRC Q35-01 transmitter which retails for around $15
//  HT1621 LCD driver chip.
//  Teensy 3.5
//Author: james@team2168.org

//SETUP INSTRUCTIONS:
// - Install Arduino IDE
// - Install TeensyDuino
// - Install MSF-XINPUT into libraries foder. Copy files into hardware directory.
// - Select yout Teensy board from Tools > Board in Arduino IDE
// - Select Tools > Usb Type > XInput

#include <Bounce.h>
#include <xinput.h> //Include the XINPUT library

#include "src/ht1621_LCD/ht1621_LCD.h"

//Pinouts chosend to try to keep compatible with TeensyLC implementation
//BUTTON INPUT PINS
#define B1PIN 0         //"Forward fine tuning"
#define B2PIN 1         //"Left fine tuning"
#define B3PIN 2         //"Right fine tuning"
#define B4PIN 3         //"Backward fine tuning"
#define B5PIN 4         //Left menu key
#define B6PIN 5         //Left menu key

//ANALOG INPUT PINS
#define AN1PIN 0        // Pin 14, Wheel (turning) 
#define AN2PIN 1        // Pin 15, Trigger (acceleration)

//ANALOG OUTPUT PINS
#define VIBE1PIN 8      // Pin 22, 'Heavy' weight vibrator motor
#define VIBE2PIN 9      // Pin 23, 'Light' weight vibrator motor

//DIGITAL OUTPUT PINS
#define LEDPIN 13       //

//LCD INTERFACE
#define LCD_CSPIN   10    //
#define LCD_WRPIN   11    // 
#define LCD_DATAPIN 12    //

ht1621_LCD lcd;

#define ANALOG_RES 13   // Resolution of the analog reads (bits)
#define MILLIDEBOUNCE 20  //Debounce time in milliseconds

struct SEG {
  char addr;     //address this LCD segment resides within
  char data_pos; //bit position in data field for the LCD segment 
};

const SEG NUL_SEG = {0x0, 0x0};

enum analog_indicator {
  wheel,        //hosizontal bar (numeric and gauge)
  throttle,     //vertical bar (numeric and gauge)
  volts,        //No gauge, just numeric
  speedometer,  //No numerics, just gague
  radio         //No numberics, just gauge
};

struct ANALOG_T {
  int val = 0; //Current value (e.g. 0 - 100%)
  int min_val = 0;
  int max_val = 0;
  SEG border = NUL_SEG;
  int gauge_len = 0;
  SEG segs[10];
};

//Seven segment number positions are represented
// as follows:
//        -A-     -A-
//  B    F   B   F   B
//        -G-     -G-
//  C    E   C   E   C
//        -D-     -D-
// Hund.  Tens    Ones

struct SEVEN_SEG {
  SEG A;
  SEG B;
  SEG C;
  SEG D;
  SEG E;
  SEG F;
  SEG G;
};

//LCD Pixel map
SEG Y_HUNDS = {0x0A, 0x80}; //Y Hundreds "1" (B and C)
SEG Y_TENS_A = {0x0B, 0x80}; //Y Tens A
SEG Y_TENS_B = {0x0B, 0x40}; //Y Tens B
SEG Y_TENS_C = {0x0B, 0x20}; //Y Tens C
SEG Y_TENS_D = {0x0B, 0x10}; //Y Tens D
SEG Y_TENS_E = {0x0A, 0x10}; //Y Tens E
SEG Y_TENS_F = {0x0A, 0x40}; //Y Tens F
SEG Y_TENS_G = {0x0A, 0x20}; //Y Tens G
SEG Y_ONES_A = {0x0C, 0x80}; //Y Ones A
SEG Y_ONES_B = {0x0C, 0x40}; //Y Ones B
SEG Y_ONES_C = {0x0C, 0x20}; //Y Ones C
SEG Y_ONES_D = {0x0D, 0x10}; //Y Ones D
SEG Y_ONES_E = {0x0C, 0x10}; //Y Ones E
SEG Y_ONES_F = {0x0C, 0x40}; //Y Ones F
SEG Y_ONES_G = {0x0C, 0x20}; //Y Ones G
SEG Y_PERCENT = {0x0C, 0x80}; //Y Percent Symbol

SEVEN_SEG y_ones = {Y_ONES_A, Y_ONES_B, Y_ONES_C, Y_ONES_D, Y_ONES_E, Y_ONES_F, Y_ONES_G};
SEVEN_SEG y_tens = {Y_TENS_A, Y_TENS_B, Y_TENS_C, Y_TENS_D, Y_TENS_E, Y_TENS_F, Y_TENS_G};

SEG Y_BAR_0 = {0x0E, 0x20}; //Y Bar graph pos 0 (lowest)
SEG Y_BAR_1 = {0x0E, 0x40}; //Y Bar graph pos 1
SEG Y_BAR_2 = {0x0E, 0x80}; //Y Bar graph pos 2
SEG Y_BAR_3 = {0x0F, 0x80}; //Y Bar graph pos 3
SEG Y_BAR_4 = {0x0F, 0x40}; //Y Bar graph pos 4
SEG Y_BAR_5 = {0x0F, 0x20}; //Y Bar graph pos 5
SEG Y_BAR_6 = {0x0F, 0x10}; //Y Bar graph pos 6 (highest)
SEG Y_BAR_BORDER = {0x0E, 0x10}; //Y Border around bar graph

SEG X_BAR_0 = {0x10, 0x10}; //X Bar graph pos 0 (left most)
SEG X_BAR_1 = {0x10, 0x20}; //X Bar graph pos 1
SEG X_BAR_2 = {0x10, 0x40}; //X Bar graph pos 2
SEG X_BAR_3 = {0x10, 0x80};//X Bar graph pos 3
SEG X_BAR_4 = {0x1F, 0x40}; //X Bar graph pos 4
SEG X_BAR_5 = {0x1F, 0x20}; //X Bar graph pos 5
SEG X_BAR_6 = {0x1F, 0x10}; //X Bar graph pos 6 (right most)
SEG X_BAR_BORDER = {0x1F, 0x80}; //X Border around bar graph

// {0x11, 0x80}, //X Hundreds "1" (B and C)
// {0x11, 0x10}, //X Tens E
// {0x11, 0x20}, //X Tens G
// {0x11, 0x40}, //X Tens F
// {0x12, 0x10}, //X Tens D
// {0x12, 0x20}, //X Tens C
// {0x12, 0x40}, //X Tens B
// {0x12, 0x80}, //X Tens A
// {0x13, 0x10}, //X Ones E
// {0x13, 0x20}, //X Ones G
// {0x13, 0x40}, //X Ones F
// {0x14, 0x10}, //X Ones D
// {0x14, 0x20}, //X Ones C
// {0x14, 0x40}, //X Ones B
// {0x14, 0x80}, //X Ones A

// {0x15, 0x10}, //Video Icon
// {0x15, 0x20}, //X Percent Symbol
// {0x15, 0x40}, //Photo Icon

// {0x19, 0x10}, //Voltage Ones A
// {0x18, 0x10}, //Voltage Ones B
// {0x18, 0x40}, //Voltage Ones C
// {0x19, 0x80}, //Voltage Ones D
// {0x19, 0x40}, //Voltage Ones E
// {0x19, 0x20}, //Voltage Ones F
// {0x18, 0x20}, //Voltage Ones G
// {0x18, 0x80}, //Voltage Decimal Point
// {0x17, 0x10}, //Voltage Tenths A
// {0x16, 0x10}, //Voltage Tenths B
// {0x16, 0x40}, //Voltage Tenths C
// {0x17, 0x80}, //Voltage Tenths D
// {0x17, 0x40}, //Voltage Tenths E
// {0x17, 0x20}, //Voltage Tenths F
// {0x16, 0x20}, //Voltage Tenths G
// {0x16, 0x80}, //Voltage "V" label

SEG SPEED_0 = {0x1B, 0x10}; //Speedometer pos 0 (left most)
SEG SPEED_1 = {0x1B, 0x20}; //Speedometer pos 1
SEG SPEED_2 = {0x1B, 0x40}; //Speedometer pos 2
SEG SPEED_3 = {0x1B, 0x80}; //Speedometer pos 3
SEG SPEED_4 = {0x1E, 0x80}; //Speedometer pos 4
SEG SPEED_5 = {0x1E, 0x40}; //Speedometer pos 5
SEG SPEED_6 = {0x1E, 0x20}; //Speedometer pos 6
SEG SPEED_7 = {0x1E, 0x10}; //Speedometer pos 7
SEG SPEED_8 = {0x1A, 0x20}; //Speedometer pos 8
SEG SPEED_9 = {0x1A, 0x40}; //Speedometer pos 9 (right most)
SEG SPEED_BORDER = {0x1A, 0x10}; //Speedometer labels
SEG SPEED_KMH = {0x1A, 0x80}; //Speedometer KM/H label

SEG RADIO_0 = {0x1d, 0x40}; //Signal meter pos 0 (left most)
SEG RADIO_1 = {0x1d, 0x20}; //Signal meter pos 1
SEG RADIO_2 = {0x1d, 0x10}; //Signal meter pos 2
SEG RADIO_3 = {0x1c, 0x10}; //Signal meter pos 3
SEG RADIO_4 = {0x1c, 0x20}; //Signal meter pos 4 (right most)
SEG RADIO_MODE1 = {0x1c, 0x40}; //"Mode1" Indicator
SEG RADIO_ANT = {0x1d, 0x80}; //Signal meter Antenna Symbol

//Initiate the class and setup the LED pin
XINPUT controller(LED_ENABLED, LEDPIN);

Bounce b1 = Bounce(B1PIN, MILLIDEBOUNCE);
Bounce b2 = Bounce(B2PIN, MILLIDEBOUNCE);
Bounce b3 = Bounce(B3PIN, MILLIDEBOUNCE);
Bounce b4 = Bounce(B4PIN, MILLIDEBOUNCE);
Bounce b5 = Bounce(B5PIN, MILLIDEBOUNCE);
Bounce b6 = Bounce(B6PIN, MILLIDEBOUNCE);

int wheelValue = 0;
int triggerValue = 0;

ANALOG_T y_axis =     {0,    //Initial val
                       -100, //Min
                       100,  //Max
                       Y_BAR_BORDER,
                       7,    //Number of segments
                       {Y_BAR_0, Y_BAR_1, Y_BAR_2, Y_BAR_3, Y_BAR_4,
                        Y_BAR_5, Y_BAR_6, NUL_SEG, NUL_SEG, NUL_SEG}};
ANALOG_T x_axis =     {0,    //Initial val
                       -100, //Min
                       100,  //Max
                       X_BAR_BORDER,
                       7,    //Number of segments
                       {X_BAR_0, X_BAR_1, X_BAR_2, X_BAR_3, X_BAR_4,
                        X_BAR_5, X_BAR_6, NUL_SEG, NUL_SEG, NUL_SEG}};
ANALOG_T volts_axis = {0,    //Initial val
                       0,    //Min
                       99,   //Max
                       NUL_SEG,
                       0,    //Number of segments
                       {NUL_SEG, NUL_SEG, NUL_SEG, NUL_SEG, NUL_SEG,
                        NUL_SEG, NUL_SEG, NUL_SEG, NUL_SEG, NUL_SEG}};
ANALOG_T speed_axis = {0,    //Initial val
                       0,    //Min
                       100,  //Max
                       SPEED_BORDER,
                       10,   //Number of segments
                       {SPEED_0, SPEED_1, SPEED_2, SPEED_3, SPEED_4,
                        SPEED_5, SPEED_6, SPEED_7, SPEED_8, SPEED_9}};
ANALOG_T radio_axis = {0,   //Initial val
                       0,   //Min
                       100, //Max
                       RADIO_ANT,
                       5,   //Number of segments
                       {RADIO_0, RADIO_1, RADIO_2, RADIO_3, RADIO_4,
                        NUL_SEG, NUL_SEG, NUL_SEG, NUL_SEG, NUL_SEG}};

void setup() {
  //Serial5.begin(115200);
  
  //Set pin modes
  pinMode(B1PIN, INPUT_PULLUP);
  pinMode(B2PIN, INPUT_PULLUP);
  pinMode(B3PIN, INPUT_PULLUP);
  pinMode(B4PIN, INPUT_PULLUP);
  pinMode(B5PIN, INPUT_PULLUP);
  pinMode(B6PIN, INPUT_PULLUP);

  //Increase resolution of analog inputs.
  analogReadResolution(ANALOG_RES);

  lcd.setup(LCD_CSPIN, LCD_WRPIN, LCD_DATAPIN);
  lcd.conf();
  LCDSegsOff();
  LCDSegsOn();
  delay(1000);
  LCDSegsOff();

  //TODO: Add analog stick calibration. Store to eeprom
  //      Store Min, Zero, Max, Checksum for each axis.
  //      Hold button(s) on startup to enter cal mode.
  //      Hold same buttons for duration to leave cal mode.
  //      Vibrate for confirmation of entering/exiting mode.
  
  setBorders(true);
}

/**
 * Scale the analog inputs to the range used by joystick function.
 */
int scaleStick(int val) {
  return map(val, 0, pow(2,ANALOG_RES), -32768, 32767);
}

void loop() {
  int abs_throttle = 0;
  
  //Read pin values
  b1.update();
  b2.update();
  b3.update();
  b4.update();
  b5.update();
  b6.update();
  wheelValue = analogRead(AN1PIN);
  triggerValue = analogRead(AN2PIN);

  //Update buttons
  controller.buttonUpdate(BUTTON_A, !b1.read());
  controller.buttonUpdate(BUTTON_B, !b2.read());
  controller.buttonUpdate(BUTTON_X, !b3.read());
  controller.buttonUpdate(BUTTON_Y, !b4.read());
  controller.buttonUpdate(BUTTON_BACK, !b5.read());
  controller.buttonUpdate(BUTTON_START, !b6.read());

  //Update analog sticks
  controller.stickUpdate(STICK_LEFT, scaleStick(wheelValue), scaleStick(triggerValue));

  //Update rumbles
  analogWrite(VIBE1PIN, controller.rumbleValues[0]);
  analogWrite(VIBE2PIN, controller.rumbleValues[1]);

  controller.LEDUpdate();     //Update the LEDs
  controller.sendXinput();    //Send data
  controller.receiveXinput(); //Receive data

  //Update screen graphics
  updateGauge(wheel, wheelValue, 0, pow(2,ANALOG_RES));
  updateGauge(throttle, triggerValue, 0, pow(2,ANALOG_RES));
  abs_throttle = abs(map(triggerValue, 0, pow(2,ANALOG_RES), -100, 100));
  updateGauge(speedometer, abs_throttle, 0, 100);
  //updateGauge(radio, count, 0, 10);

  //Dump LCD data out to the screen
  lcd.update();
}

void LCDSegsOff() {
  //Write all zeros to addresses 0x00 through 0x31
  lcd.setAll(0x00);
  lcd.update();
}

void LCDSegsOn() {
  //Write all ones to addresses 0x00 through 0x31
  lcd.setAll(0xFF);
  lcd.update();
}

void walkLCDSegments(unsigned char addr, int delay_ms) {
  for(int i=0x9; i < addr; i++) {
    lcd.setByte(i, 0xf0);
    lcd.update();
    delay(delay_ms);
  }
  for(int i=0x9; i < addr; i++) {
    lcd.setByte(i, 0x00);
    lcd.update();
    delay(delay_ms);
  }
}

/**
 * Helper function to turn on a specified LCD segment.
 */
void setSeg(SEG s) {
  lcd.setBits(s.addr, s.data_pos);
}

/**
 * Helper function to turn off a specified LCD segment.
 */
void clearSeg(SEG s) {
  lcd.clearBits(s.addr, s.data_pos);
}

/**
 * Turn on/off border segments.
 * on - true = on, fales = off
 */
void setBorders(boolean on) {
  if(on) {
    setSeg(Y_BAR_BORDER);
    setSeg(X_BAR_BORDER);
    setSeg(SPEED_BORDER);
  } else {
    clearSeg(Y_BAR_BORDER);
    clearSeg(X_BAR_BORDER);
    clearSeg(SPEED_BORDER);
  }
}

/**
 * Update the specified gauge with the provided value.
 * 
 * gaugeID - the axis to be updated
 * val - the number to be diplayed
 * min - smallest possible value for val
 * max - largest possible value for val
 */
void updateGauge(analog_indicator gaugeID, int val, int min, int max) {
  ANALOG_T gauge;
  int index;
    
  switch (gaugeID) {
    case wheel:
      gauge = x_axis;
      break;
    case throttle:
      gauge = y_axis;
      break;
    case volts:
      gauge = volts_axis;
      break;
    case speedometer:
      gauge = speed_axis;
      break;
    case radio:
      gauge = radio_axis;
      break;
  }

  index = map(val, min, max, 0, gauge.gauge_len - 1);
//  Serial5.print("i=");
//  Serial5.print(val);
//  Serial5.print(", map=");
//  Serial5.println(index);
  if(gauge.gauge_len != 0) {
    //Walk through the segments and turn one on, turn others off
    for(int i=0; i < gauge.gauge_len; i++) {
//      Serial5.print("  ");
      if(index == i) {
//        Serial5.print("setting  bit:");
        setSeg(gauge.segs[i]);
      } else {
//        Serial5.print("clearing bit:");
        clearSeg(gauge.segs[i]);
      }
//      Serial5.print(i);
//      Serial5.print(" [");
//      Serial5.print(gauge.segs[i].addr,HEX);
//      Serial5.print(",");
//      Serial5.print(gauge.segs[i].data_pos,HEX);
//      Serial5.println("]");
    }
  }
}
