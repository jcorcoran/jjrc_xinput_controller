//Teensy based RC controller for FRC using XBOX Controller Driver (XINPUT) 
//Supports buttons, analog sticks, and rumble.
//Originally implemented using the JJRC Q35-01 transmitter which retails for around $15
//  HT1621 LCD driver chip.
//  Tested with Teensy 3.5 & Teensy LC
//Author: james@team2168.org

//SETUP INSTRUCTIONS:
// - Install Arduino IDE
// - Install TeensyDuino
// - Install MSF-XINPUT into libraries foder. Copy files into hardware directory.
// - Select yout Teensy board from Tools > Board in Arduino IDE
// - Select Tools > Usb Type > XInput

#include <Bounce.h>
#include <EEPROM.h>
#include <xinput.h>

#include "src/ht1621_LCD/ht1621_LCD.h"
#include "src/fSevSeg/fSevSeg.h"

//DISABLED ANALOG INPUTS
#define LEFT_STICK_DISABLED false
#define RIGHT_STICK_DISABLED true
#define TRIGGER_DISABLED true

//Pinouts chosend to try to keep compatible with TeensyLC implementation
//DIGITAL INPUT PINS
#define AUX1_PIN 0      // Pin 0, Auxiliary discrete input 1
#define AUX2_PIN 1      // Pin 1, Auxiliary discrete input 2
#define AUX3_PIN 2      // Pin 2, Auxiliary discrete input 3
#define AUX4_PIN 3      // Pin 3, Auxiliary discrete input 4

//DIGITAL OUTPUT PINS
#define LEDPIN 13       // Pin 13

//LCD INTERFACE
#define LCD_CSPIN   10  // Pin 10
#define LCD_WRPIN   11  // Pin 11
#define LCD_DATAPIN 12  // Pin 12

//SERIAL DEBUG - Reserved
//RX3 PIN 7             // Pin 7
//TX3 PIN 8             // Pin 8
#define HWSERIAL Serial3

//ANALOG INPUT PINS
#define AN1PIN 0        // Pin 14, Wheel (turning) 
#define AN2PIN 1        // Pin 15, Trigger (acceleration)
#define AN3PIN 2        // Pin 16, Buttons
                        //         3.32V (0x1FFC) - No buttons pressed
                        //         2.57V (0x18C7) - Right menu key
                        //         2.29V (0x1610) - Left menu key
                        //         1.88V (0x121E) - "Forward fine tuning" pressed
                        //         1.27V (0x0C43) - "Left fine tuning" pressed
                        //         0.44V (0x042F) - "Right fine tuning" pressed
                        //         0.00V (0x0000) - "Backward fine tuning" pressed
#define AN4PIN 3        // Pin 17, Aux. analog input 1
#define AN5PIN 4        // Pin 18, Aux. analog input 2
#define AN6PIN 5        // Pin 19, Aux. analog input 3
#define AN7PIN 6        // Pin 20, Aux. analog input 4
                        
//ANALOG OUTPUT PINS
#define VIBE1PIN 22      // Pin 22 (A8), 'Heavy' weight vibrator motor
#define VIBE2PIN 23      // Pin 23 (A9), 'Light' weight vibrator motor

ht1621_LCD lcd;

#define ANALOG_RES 13     // Resolution of the analog reads (bits)

#define BUTTON_TOL 300   // Allowable error in bits (Assuming 13bit precision ADC) from the measured button voltages.
                         // Worst case emperical separation between voltages was about 700 bits.
                         // Tolerance should be less than half worst case separation. Will be used for +/- tol about setpoints.
#define BTN_NONE_PRESSED       0x1FFC  // 3.32V (0x1FFC) - No buttons pressed
#define BTN_RIGHT_MENU_PRESSED 0x18C7  // 2.57V (0x18C7) - Right menu key
#define BTN_LEFT_MENU_PRESSED  0x1610  // 2.29V (0x1610) - Left menu key
#define BTN_FWD_TUNE_PRESSED   0x121E  // 1.88V (0x121E) - "Forward fine tuning" pressed
#define BTN_LEFT_TUNE_PRESSED  0x0C43  // 1.27V (0x0C43) - "Left fine tuning" pressed
#define BTN_RIGHT_TUNE_PRESSED 0x042F  // 0.44V (0x042F) - "Right fine tuning" pressed
#define BTN_BACK_TUNE_PRESSED  0x0000  // 0.00V (0x0000) - "Backward fine tuning" pressed

enum BUTTON_T {
  RIGHT_MENU,
  LEFT_MENU,
  FWD_TUNE,
  LEFT_TUNE,
  RIGHT_TUNE,
  BACK_TUNE,
  NONE
};

BUTTON_T button_pressed = NONE;
#define CAL_BUTTON RIGHT_MENU      //Button to press to enter/exit the calibration menu
#define CAL_EXIT_BUTTON LEFT_MENU  //Button to hold to exit cal without saving changes

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
XINPUT controller(NO_LED);
int last_led_pattern = LED_ENABLED;

struct CAL_DATA_T {
  long x_min; //wheel
  long x_zero;
  long x_max;
  long y_min; //trigger
  long y_zero;
  long y_max;
  long cksum; //XOR of previous fields 
} cal_data;

boolean cal_valid = false;

#define MILLIDEBOUNCE 20  //Button debounce time in milliseconds
Bounce aux1 = Bounce(AUX1_PIN, MILLIDEBOUNCE);
Bounce aux2 = Bounce(AUX2_PIN, MILLIDEBOUNCE);
Bounce aux3 = Bounce(AUX3_PIN, MILLIDEBOUNCE);
Bounce aux4 = Bounce(AUX4_PIN, MILLIDEBOUNCE);

int wheelValue = 0;
int triggerValue = 0;
int buttonValue = 0;
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

void setup() {
  BUTTON_T b;

  pinMode(LEDPIN, OUTPUT);
  pinMode(AUX1_PIN, INPUT_PULLUP);
  pinMode(AUX2_PIN, INPUT_PULLUP);
  pinMode(AUX3_PIN, INPUT_PULLUP);
  pinMode(AUX4_PIN, INPUT_PULLUP);

  HWSERIAL.begin(115200);

  HWSERIAL.println("");
  HWSERIAL.println("");
  HWSERIAL.println("FRC2168 - XINPUT Controller - github.com/jcorcoran/jjrc_xinput_controller");

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
  delay(500);

  //TODO: Add analog stick calibration. Store to eeprom
  //      Store Min, Zero, Max, Checksum for each axis.
  //      Hold button(s) on startup to enter cal mode.
  //      Hold same buttons for duration to leave cal mode.
  //      Vibrate for confirmation of entering/exiting mode.

  b = read_buttons();
  //Must continue to hold button for 5s
  for(int i=0; i<1000 && b == CAL_BUTTON; i++){
    b = read_buttons();
    delay(5);
  }
  if(b == CAL_BUTTON) {
    calibrate(); //Enter Calibration Mode
  }

  LCDSegsOff();
  
  //Load in CAL data from EEPROM
  cal_valid = read_cal();

  setBorders(true);
  lcd.setSeg(Y_PERCENT);
  lcd.setSeg(X_PERCENT);
}

void loop() {
  int abs_throttle = 0;

  //Read pin values
  aux1.update();
  aux2.update();
  aux3.update();
  aux4.update();
  wheelValue = avgAnalogRead(AN1PIN);
  triggerValue = avgAnalogRead(AN2PIN);

  //If we have valid cal data, adjust the raw analog inputs
  if(cal_valid) {
    wheelValue = cal_scale_axis(wheel_axis, wheelValue);
    triggerValue = cal_scale_axis(throttle_axis, triggerValue);
  }

  //Update button states
  button_pressed = read_buttons();
  controller.buttonUpdate(BUTTON_A, button_pressed == FWD_TUNE);
  controller.buttonUpdate(BUTTON_B, button_pressed == LEFT_TUNE);
  controller.buttonUpdate(BUTTON_X, button_pressed == RIGHT_TUNE);
  controller.buttonUpdate(BUTTON_Y, button_pressed == BACK_TUNE);
  controller.buttonUpdate(BUTTON_BACK, button_pressed == LEFT_MENU);
  controller.buttonUpdate(BUTTON_START, button_pressed == RIGHT_MENU);
  controller.buttonUpdate(BUTTON_LB, !aux1.read());
  controller.buttonUpdate(BUTTON_RB, !aux2.read());
  controller.buttonUpdate(BUTTON_L3, !aux3.read());
  controller.buttonUpdate(BUTTON_R3, !aux4.read());

  //Update analog sticks
  if(!LEFT_STICK_DISABLED) {
    controller.stickUpdate(STICK_LEFT, xinput_scale_sticks(wheelValue),
      xinput_scale_sticks(triggerValue));
  }
  if(!RIGHT_STICK_DISABLED) {
    controller.stickUpdate(STICK_RIGHT, xinput_scale_sticks(avgAnalogRead(AN4PIN)),
      xinput_scale_sticks(avgAnalogRead(AN5PIN)));
  }
  if(!TRIGGER_DISABLED) {
    controller.triggerUpdate(xinput_scale_trigger(avgAnalogRead(AN6PIN)),
      xinput_scale_trigger(avgAnalogRead(AN7PIN)));
  }

  //Update rumbles
  analogWrite(VIBE1PIN, controller.rumbleValues[0]);
  analogWrite(VIBE2PIN, controller.rumbleValues[1]);

  //TODO - Flash lights along with Rumble 2


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
  volt_segs.DisplayString("");
  //volt_segs.DisplayIntHex(count);

  //Dump LCD data out to the screen
  lcd.update();
  delay(35);
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

/**
 * Checks if any of the buttons are pressed.
 * Note all buttons wire into a single analog input channel. If more than one button is pressed at a time,
 * the one with the least resistance to ground wins (electrical constraint).
 */
BUTTON_T read_buttons() {
  int value = avgAnalogRead(AN3PIN);
  BUTTON_T retval = NONE;

  //Walk down the voltages to see if anything was pressed.
  if(value >= (BTN_NONE_PRESSED - BUTTON_TOL)) {
    retval = NONE;
  } else if(value <= (BTN_RIGHT_MENU_PRESSED + BUTTON_TOL)
      && value >= (BTN_RIGHT_MENU_PRESSED - BUTTON_TOL)) {
    retval = RIGHT_MENU;
  } else if(value <= (BTN_LEFT_MENU_PRESSED + BUTTON_TOL)
      && value >= (BTN_LEFT_MENU_PRESSED - BUTTON_TOL)) {
    retval = LEFT_MENU;
  } else if(value <= (BTN_FWD_TUNE_PRESSED + BUTTON_TOL)
      && value >= (BTN_FWD_TUNE_PRESSED - BUTTON_TOL)) {
    retval = FWD_TUNE;
  } else if(value <= (BTN_LEFT_TUNE_PRESSED + BUTTON_TOL)
      && value >= (BTN_LEFT_TUNE_PRESSED - BUTTON_TOL)) {
    retval = LEFT_TUNE;
  } else if(value <= (BTN_RIGHT_TUNE_PRESSED + BUTTON_TOL)
      && value >= (BTN_RIGHT_TUNE_PRESSED - BUTTON_TOL)) {
    retval = RIGHT_TUNE;
  } else if(value <= (BTN_BACK_TUNE_PRESSED + BUTTON_TOL)) {
    retval = BACK_TUNE;
  }
  
  return retval;
}

void calibrate() {
  boolean calFinished = false;
  boolean calExited = false;
  BUTTON_T b = CAL_BUTTON;
  CAL_DATA_T cal;

  cal.x_min = pow(2,ANALOG_RES);
  cal.x_zero = 0;
  cal.x_max = 0;
  cal.y_min = pow(2,ANALOG_RES);
  cal.y_zero = 0;
  cal.y_max = 0;
  cal.cksum = 0;

  HWSERIAL.println("Entering Calibration Mode");

  y_segs.DisplayString("CA");
  x_segs.DisplayString("CA");
  volt_segs.DisplayString("CA");
  setBorders(true);
  lcd.update();
  
  //Wait until the calibration button is released before procedding.
  while(b == CAL_BUTTON){
    b = read_buttons();
    delay(10);
  }
  
  while(!calFinished && !calExited) {
    wheelValue = avgAnalogRead(AN1PIN);
    triggerValue = avgAnalogRead(AN2PIN);

    y_avg = iir(y_avg_last, triggerValue);
    x_avg = iir(x_avg_last, wheelValue);
    y_avg_last = y_avg;
    x_avg_last = x_avg;

    y_segs.DisplayIntHex(map(y_avg, 0, pow(2,ANALOG_RES), 0x00, 0xFF));
    x_segs.DisplayIntHex(map(x_avg, 0, pow(2,ANALOG_RES), 0x00, 0xFF));

    updateGauge(wheel_ind, wheelValue, 0, pow(2,ANALOG_RES));
    updateGauge(throttle_ind, triggerValue, 0, pow(2,ANALOG_RES));

    //Update new min/max values
    cal.x_min = min(cal.x_min, x_avg); 
    cal.x_max = max(cal.x_max, x_avg);
    cal.y_min = min(cal.y_min, y_avg);
    cal.y_max = max(cal.y_max, y_avg);

    b = read_buttons();
    //Check if calibration is complete (CAL button held for 5s)
    for(int i=0; (b == CAL_BUTTON) && !calFinished; i++) {
      if(i > 100) {
        calFinished=true;
      }
      b = read_buttons();
      delay(50);
    }

    //Check if cal is being exited (don't write changes to EEPROM (CAL_EXIT button held for 100ms)
    for(int i=0; (b == CAL_EXIT_BUTTON) && !calExited; i++) {
      if(i > 10) {
        HWSERIAL.println("Exiting Calibration Mode");
        calExited=true;
      }
      b = read_buttons();
      delay(10);
    }

    if(calFinished) {
      //Store zeros
      cal.x_zero = x_avg;
      cal.y_zero = y_avg;

      //Calculate checksum
      cal.cksum = cal.x_min ^ cal.x_zero ^ cal.x_max ^ cal.y_min ^ cal.y_zero ^ cal.y_max;
      HWSERIAL.println("Calibration sequence finished.");
      print_cal(cal);
      
      //Write to EEPROM
      store_cal(cal);
    }

    lcd.update();
    delay(10);
  }
}

/**
 * Reads the stored calibration data from EEPROM
 * Returns true if data is valid, false otherwise.
 */
boolean read_cal() {
  CAL_DATA_T cal;
  boolean retval = false;

  eeprom_read_block((void*)&cal, (void*)0, sizeof(cal));
  print_cal(cal);

  //Check checksum
  if(cal.cksum == (cal.x_min ^ cal.x_zero ^ cal.x_max ^ cal.y_min ^ cal.y_zero ^ cal.y_max)) {
    HWSERIAL.println("Checksum good.");
    cal_data = cal;
    retval = true;
  } else {
    HWSERIAL.println("Checksum bad! :(");
  }

  return retval;
}

void store_cal(CAL_DATA_T cal) {
  HWSERIAL.println("Writing cal data");
  eeprom_update_block((void*)&cal, (void*)0, sizeof(cal));
  delay(100);
}

void print_cal(CAL_DATA_T cal) {
  HWSERIAL.println("Calibration data:");
  HWSERIAL.print("  x_min:");
  HWSERIAL.println(cal.x_min, HEX);
  HWSERIAL.print("  x_zero:");
  HWSERIAL.println(cal.x_zero, HEX);
  HWSERIAL.print("  x_max:");
  HWSERIAL.println(cal.x_max, HEX);
  HWSERIAL.print("  y_min:");
  HWSERIAL.println(cal.y_min, HEX);
  HWSERIAL.print("  y_zero:");
  HWSERIAL.println(cal.y_zero, HEX);
  HWSERIAL.print("  y_max:");
  HWSERIAL.println(cal.y_max, HEX);
  HWSERIAL.print("  cksum:");
  HWSERIAL.println(cal.cksum, HEX);
}

/**
 * Scale the analog inputs to the range used by joystick function.
 *
 */
int xinput_scale_sticks(int val) {
  int _out_max = 32767;
  int _out_min = -32768;
  int _in_min = 0;
  int _in_max = pow(2,ANALOG_RES);
  int _in_zero = (_in_max - _in_min)/2;
  int ret = 0;

  if(val >= _in_zero) { //Scale the high side
    ret = map(val, _in_zero, _in_max, 0, _out_max);
  } else {  //Scale the low side
    ret = map(val, _in_min, _in_zero, _out_min, 0);
  }
  
  return ret;
}

/**
 * Scale the analog inputs to the range used by joystick trigger function.
 * 
 * Note each axis only operates over half the range (0 - 3V)
 *   left trigger is the low side of the range (0 - 1.5V)
 *   right trigger is the high side of the range (1.5 - 3V)
 */
int xinput_scale_trigger(int val) {
  int _out_max = 0xFF;
  int _out_min = 0;
  int _in_min = 0;
  int _in_max = pow(2,ANALOG_RES);
  int _in_zero = (_in_max - _in_min)/2;
  int ret = 0;

  if(val >= _in_zero) { //Scale the high side
    ret = map(val, _in_zero, _in_max, 0, _out_max);
  } else {  //Scale the low side
    ret = map(val, _in_min, _in_zero, _out_min, 0);
  }

  return ret;
}

/**
 * Scale the analog inputs to the range used by display functions.
 */
int cal_scale_axis(analog_axis axis, int val) {
  int _out_min = 0;
  int _out_max = pow(2,ANALOG_RES);
  int _out_zero = (_out_max - _out_min)/2;
  int _in_min = _out_min;
  int _in_max = _out_max;
  int _in_zero = _out_zero;
  int ret = 0;

  if(cal_valid) {
    switch (axis) {
      case wheel_axis: //X
        _in_max = cal_data.x_max;
        _in_min = cal_data.x_min;
        _in_zero = cal_data.x_zero;
        break;
      case throttle_axis: //Y
        _in_max = cal_data.y_max;
        _in_min = cal_data.y_min;
        _in_zero = cal_data.y_zero;
        break;
    }
  }

  //Ensure inputs are in range
  if(val < _in_min) {
    val = _in_min;
  } else if(val > _in_max) {
    val = _in_max;
  }

  if(val >= _in_zero) { //Scale the high side
    ret = map(val, _in_zero, _in_max, _out_zero, _out_max);
  } else {  //Scale the low side
    ret = map(val, _in_min, _in_zero, _out_min, _out_zero);
  }

  return ret;
}

int avgAnalogRead(int channel) {
  #define SAMPLES 5
  int average = 0;

  for(int i=0; i < SAMPLES; i++) {
    average += analogRead(channel);
  }

  return average / SAMPLES;
}
