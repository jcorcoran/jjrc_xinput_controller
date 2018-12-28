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
#include "src/fSevSeg/fSevSeg.h"

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

enum analog_indicator {
  wheel_ind,        //horizontal bar (numeric and gauge)
  throttle_ind,     //vertical bar (numeric and gauge)
  volts_ind,        //No gauge, just numeric
  speedometer_ind,  //No numerics, just gague
  radio_ind         //No numberics, just gauge
};

enum analog_axis {
  wheel_axis,
  throttle_axis
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

DIGIT y_ones = {Y_ONES_A, Y_ONES_B, Y_ONES_C, Y_ONES_D, Y_ONES_E, Y_ONES_F, Y_ONES_G};
DIGIT y_tens = {Y_TENS_A, Y_TENS_B, Y_TENS_C, Y_TENS_D, Y_TENS_E, Y_TENS_F, Y_TENS_G};
DIGIT y_hunds = {NUL_SEG, Y_HUNDS, Y_HUNDS, NUL_SEG, NUL_SEG, NUL_SEG, NUL_SEG};

DIGIT x_ones = {X_ONES_A, X_ONES_B, X_ONES_C, X_ONES_D, X_ONES_E, X_ONES_F, X_ONES_G};
DIGIT x_tens = {X_TENS_A, X_TENS_B, X_TENS_C, X_TENS_D, X_TENS_E, X_TENS_F, X_TENS_G};
DIGIT x_hunds = {NUL_SEG, X_HUNDS, X_HUNDS, NUL_SEG, NUL_SEG, NUL_SEG, NUL_SEG};

DIGIT v_ones = {VOLT_ONES_A, VOLT_ONES_B, VOLT_ONES_C, VOLT_ONES_D, VOLT_ONES_E, VOLT_ONES_F, VOLT_ONES_G};
DIGIT v_tenths = {VOLT_TENT_A, VOLT_TENT_B, VOLT_TENT_C, VOLT_TENT_D, VOLT_TENT_E, VOLT_TENT_F, VOLT_TENT_G};

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
float y_avg = pow(2,ANALOG_RES)/2.0;
float y_avg_last = pow(2,ANALOG_RES)/2.0;
float x_avg = pow(2,ANALOG_RES)/2.0;
float x_avg_last = pow(2,ANALOG_RES)/2.0;

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

fSevSeg y_segs, x_segs, volt_segs;

void calibrate() {
  boolean calFinished = false;

  long x_min = pow(2,ANALOG_RES);
  long x_max = 0x0;
  long y_min = pow(2,ANALOG_RES);
  long y_max = 0x0;

  //Serial5.println("Entering Calibration Mode");

  y_segs.DisplayString("CA");
  x_segs.DisplayString("CA");
  volt_segs.DisplayString("CA");
  setBorders(true);
  lcd.update();
  
  //Wait until the calibration button is released before procedding.
  while(!b1.read()){
    b1.update();
    delay(5);
  }
  
  while(!calFinished) {
    wheelValue = analogRead(AN1PIN);
    triggerValue = analogRead(AN2PIN);

    y_avg = iir(y_avg_last, triggerValue);
    x_avg = iir(x_avg_last, wheelValue);
    y_avg_last = y_avg;
    x_avg_last = x_avg;

    y_segs.DisplayIntHex(map(y_avg, 0, pow(2,ANALOG_RES), 0x00, 0xFF));
    x_segs.DisplayIntHex(map(x_avg, 0, pow(2,ANALOG_RES), 0x00, 0xFF));

    updateGauge(wheel_ind, wheelValue, 0, pow(2,ANALOG_RES));
    updateGauge(throttle_ind, triggerValue, 0, pow(2,ANALOG_RES));

    //Update new min/max values
    x_min = min(x_min, x_avg); 
    x_max = max(x_max, x_avg);
    y_min = min(y_min, y_avg);
    y_max = max(y_max, y_avg);

    b1.update();
    //Check if calibration is complete (button held for 1s)
    for(int i=0; !b1.read() && !calFinished; i++) {
      if(i > 100) {
        calFinished=true;
      }
      b1.update();
      delay(10);
    }

    if(calFinished) {
      //Serial5.println("Calibration sequence finished.");
      //Write to EEPROM
    }

    lcd.update();
    delay(10);
  }
}

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
  y_segs.setup(&lcd, y_hunds, y_tens, y_ones, false);
  x_segs.setup(&lcd, x_hunds, x_tens, x_ones, false);
  volt_segs.setup(&lcd, v_ones, v_tenths, false);
  
  LCDSegsOff();
  LCDSegsOn();
  y_segs.DisplayString("21");
  x_segs.DisplayString("68");
  lcd.update();
  delay(750);
  LCDSegsOff();

  //TODO: Add analog stick calibration. Store to eeprom
  //      Store Min, Zero, Max, Checksum for each axis.
  //      Hold button(s) on startup to enter cal mode.
  //      Hold same buttons for duration to leave cal mode.
  //      Vibrate for confirmation of entering/exiting mode.
  
  b1.update();
  if(!b1.read()) {
    //Must continue to hold button for 0.5s
    for(int i=0; i<100 && !b1.read(); i++){
      b1.update();
      delay(5);
    }
    if(!b1.read()) {
      calibrate(); //Enter Calibration Mode
    }
  }

  setBorders(true);
  lcd.setSeg(Y_PERCENT);
  lcd.setSeg(X_PERCENT);
}

/**
 * Scale the analog inputs to the range used by joystick function.
 */
int scaleStick(analog_axis axis, int val) {
  int _max = 32767;
  int _min = -32768;
  int _zero = 0;

  //TODO: use the calibrated min/max/zero for each axis
  switch (axis) {
    case wheel_axis:
      break;
    case throttle_axis:
      break;
  }
  
  return map(val, 0, pow(2,ANALOG_RES), _min, _max);
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
  controller.stickUpdate(STICK_LEFT, scaleStick(wheel_axis, wheelValue),
    scaleStick(throttle_axis, triggerValue));

  //Update rumbles
  analogWrite(VIBE1PIN, controller.rumbleValues[0]);
  analogWrite(VIBE2PIN, controller.rumbleValues[1]);

  controller.LEDUpdate();     //Update the LEDs
  controller.sendXinput();    //Send data
  controller.receiveXinput(); //Receive data

  //Update screen graphics
  updateGauge(wheel_ind, wheelValue, 0, pow(2,ANALOG_RES));
  updateGauge(throttle_ind, triggerValue, 0, pow(2,ANALOG_RES));
  abs_throttle = abs(map(triggerValue, 0, pow(2,ANALOG_RES), -100, 100));
  updateGauge(speedometer_ind, abs_throttle, 0, 100);
  //updateGauge(radio_ind, count, 0, 10);

  //Update 7-segment sections
  y_avg = iir(y_avg_last, triggerValue);
  x_avg = iir(x_avg_last, wheelValue);
  y_avg_last = y_avg;
  x_avg_last = x_avg;

  y_segs.DisplayInt(map(y_avg, 0, pow(2,ANALOG_RES), -100, 100));
  x_segs.DisplayInt(map(x_avg, 0, pow(2,ANALOG_RES), -100, 100));

  //volt_segs.DisplayIntHex(count);

  //Dump LCD data out to the screen
  lcd.update();

  delay(40);
}

void LCDSegsOff() {
  //Write all zeros to LCD
  lcd.setAll(0x00);
  lcd.update();
}

void LCDSegsOn() {
  //Write all ones to LCD
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
 * Turn on/off border segments.
 * on - true = on, fales = off
 */
void setBorders(boolean on) {
  if(on) {
    lcd.setSeg(Y_BAR_BORDER);
    lcd.setSeg(X_BAR_BORDER);
    lcd.setSeg(SPEED_BORDER);
  } else {
    lcd.clearSeg(Y_BAR_BORDER);
    lcd.clearSeg(X_BAR_BORDER);
    lcd.clearSeg(SPEED_BORDER);
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
    case wheel_ind:
      gauge = x_axis;
      break;
    case throttle_ind:
      gauge = y_axis;
      break;
    case volts_ind:
      gauge = volts_axis;
      break;
    case speedometer_ind:
      gauge = speed_axis;
      break;
    case radio_ind:
      gauge = radio_axis;
      break;
  }

  index = map(val, min, max, 0, gauge.gauge_len - 1);
  if(gauge.gauge_len != 0) {
    //Walk through the segments and turn one on, turn others off
    for(int i=0; i < gauge.gauge_len; i++) {
      if(index == i) {
        lcd.setSeg(gauge.segs[i]);
      } else {
        lcd.clearSeg(gauge.segs[i]);
      }
    }
  }
}

float iir(float old_val, float new_val) {
  float a = 0.70;
  return ((old_val * a) + (new_val * (1.0 - a)));
}

long max(long a, long b) {
  if(a > b) {
    return a;
  } else {
    return b;
  }
}

long min(long a, long b) {
  if(a < b) {
    return a;
  } else {
    return b;
  }
}

