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

void setup() {
//  Serial3.begin(115200);
  
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
  delay(2000);
  LCDSegsOff();

  //TODO: Add analog stick calibration. Store to eeprom
  //      Store Min, Zero, Max, Checksum for each axis.
  //      Hold button(s) on startup to enter cal mode.
  //      Hold same buttons for duration to leave cal mode.
  //      Vibrate for confirmation of entering/exiting mode.
}

/**
 * Scale the analog inputs to the range used by joystick function.
 */
int scaleStick(int val) {
  return map(val, 0, pow(2,ANALOG_RES), -32768, 32767);
}

void loop() {
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
  
  //Update LCD
  //updateLCD();

  controller.LEDUpdate();     //Update the LEDs
  controller.sendXinput();    //Send data
  controller.receiveXinput(); //Receive data

  walkLCDSegments(0x31, 2000);
}

void LCDSegsOff() {
  //Write all zeros to addresses 0x00 through 0x31
  for(int i = 0; i < 0X31; i++) {
    lcd.wrclrdata(i, 0X00);
  }
}

void LCDSegsOn() {
  //Write all ones to addresses 0x00 through 0x31
  for(int i = 0; i < 0X31; i++) {
    lcd.wrclrdata(i, 0xFF);
  }
}

void walkLCDSegments(unsigned char addr, int delay_ms) {
  for(int i=0; i < addr; i++) {
    lcd.wrclrdata(i,0xff);
    delay(delay_ms);
    lcd.wrclrdata(i,0x00);
  }
}
