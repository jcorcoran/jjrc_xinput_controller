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

//Seven segment number positions are represented
// as follows:
//        -A-     -A-
//  B    F   B   F   B
//        -G-     -G-
//  C    E   C   E   C
//        -D-     -D-
// Hund.  Tens    Ones

//LCD Pixel map
//0xADDR, 0xDATA, Description
//{{0x0A, 0x10}, //Y Tens E
// {0x0A, 0x20}, //Y Tens G
// {0x0A, 0x40}, //Y Tens F
// {0x0A, 0x80}, //Y Hundreds "1" (B and C)
// {0x0B, 0x10}, //Y Tens D
// {0x0B, 0x20}, //Y Tens C
// {0x0B, 0x40}, //Y Tens B
// {0x0B, 0x80}, //Y Tens A
// {0x0C, 0x10}, //Y Ones E
// {0x0C, 0x20}, //Y Ones G
// {0x0C, 0x40}, //Y Ones F
// {0x0C, 0x80}, //Y Percent Symbol
// {0x0D, 0x10}, //Y Ones D
// {0x0C, 0x20}, //Y Ones C
// {0x0C, 0x40}, //Y Ones B
// {0x0C, 0x80}, //Y Ones A

// {0x0E, 0x20}, //Y Bar graph pos 0 (lowest)
// {0x0E, 0x40}, //Y Bar graph pos 1
// {0x0E, 0x80}, //Y Bar graph pos 2
// {0x0F, 0x80}, //Y Bar graph pos 3
// {0x0F, 0x40}, //Y Bar graph pos 4
// {0x0F, 0x20}, //Y Bar graph pos 5
// {0x0F, 0x10}, //Y Bar graph pos 6 (highest)
// {0x0E, 0x10}, //Y Border around bar graph

// {0x10, 0x10}, //X Bar graph pos 0 (left most)
// {0x10, 0x20}, //X Bar graph pos 1
// {0x10, 0x40}, //X Bar graph pos 2
// {0x10, 0x80}, //X Bar graph pos 3
// {0x1F, 0x40}, //X Bar graph pos 4
// {0x1F, 0x20}, //X Bar graph pos 5
// {0x1F, 0x10}, //X Bar graph pos 6 (right most)
// {0x1F, 0x80}, //X Border around bar graph

// {0x11, 0x10}, //X Tens E
// {0x11, 0x20}, //X Tens G
// {0x11, 0x40}, //X Tens F
// {0x11, 0x80}, //X Hundreds "1" (B and C)
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

// {0x16, 0x10}, //Voltage Tenths B
// {0x16, 0x20}, //Voltage Tenths G
// {0x16, 0x40}, //Voltage Tenths C
// {0x16, 0x80}, //Voltage "V" label
// {0x17, 0x10}, //Voltage Tenths A
// {0x17, 0x20}, //Voltage Tenths F
// {0x17, 0x40}, //Voltage Tenths E
// {0x17, 0x80}, //Voltage Tenths D
// {0x18, 0x10}, //Voltage Ones B
// {0x18, 0x20}, //Voltage Ones G
// {0x18, 0x40}, //Voltage Ones C
// {0x18, 0x80}, //Voltage Decimal Point
// {0x19, 0x10}, //Voltage Ones A
// {0x19, 0x20}, //Voltage Ones F
// {0x19, 0x40}, //Voltage Ones E
// {0x19, 0x80}, //Voltage Ones D

// {0x1B, 0x10}, //Speedometer pos 0 (left most)
// {0x1B, 0x20}, //Speedometer pos 1
// {0x1B, 0x40}, //Speedometer pos 2
// {0x1B, 0x80}, //Speedometer pos 3
// {0x1E, 0x80}, //Speedometer pos 4
// {0x1E, 0x40}, //Speedometer pos 5
// {0x1E, 0x20}, //Speedometer pos 6
// {0x1E, 0x10}, //Speedometer pos 7
// {0x1A, 0x20}, //Speedometer pos 8
// {0x1A, 0x40}, //Speedometer pos 9 (right most)
// {0x1A, 0x10}, //Speedometer labels
// {0x1A, 0x80}, //Speedometer KM/H label

// {0x1d, 0x40}, //Signal meter pos 0 (left most)
// {0x1d, 0x20}, //Signal meter pos 1
// {0x1d, 0x10}, //Signal meter pos 2
// {0x1c, 0x10}, //Signal meter pos 3
// {0x1c, 0x20}, //Signal meter pos 4 (right most)
// {0x1c, 0x40}, //"Mode1" Indicator
// {0x1d, 0x80}, //Signal meter Antenna Symbol




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

  walkLCDSegments(32, 200);
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
//  boolean last_button = false;
//  boolean current_button = false;
  
  for(int i=0; i < addr; i++) {
    for(int j=0x8; j < 0xff; j <<= 1) {
      Serial5.print("addr=");
      Serial5.print(i,HEX);
      Serial5.print(", data=");
      Serial5.println(j, HEX);
      
      lcd.wrclrdata(i,j);
      delay(delay_ms);

//      while(!(!last_button && current_button)) {
//        b1.update();
//        last_button = current_button;
//        current_button = !b1.read();
//      }
//      current_button = false;
//      last_button = false;
    }
    lcd.wrclrdata(i,0x00);
  }
}
