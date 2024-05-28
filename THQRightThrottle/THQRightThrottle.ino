// Program to setup an Arduino Pro Micro to provide additional 
// controls (ministick, hats, buttons and switches) to a
// VKB Throttle Quadrant (THQ) module, making use of the 3D
// printed left and right throttle bodies that have been designed 
// by Marco (@Marco_596178) at Printerbale.com which can be found 
// here - https://www.printables.com/model/405514-vkb-thq-hotas-throttle
//
// This sketch is for the right throttle controls 
//
// Darren Eckert
// 2023-06-12
// Version 0.2

#include <Joystick.h>
#define DEBUGTHQ TRUE


const int HID_ID = 0x26;              // Unique HID Device ID for this arduino
const int uBtns  = 9;                 // Number of buttons on the device
const int hBtns  = 4;                 // Hat switches Up, Right, Down, Left
const int tDelay = 50;                // Time delay for loop() in ms, may need adjusting
int i;                                // Generic counter

// Define digital pin to be used for each  button
int sHat[hBtns] = {0, 3, 4, 2};       // 5-Way Hat U/R/D/L
int sBtn[uBtns] = {1,                 // 5-Way Hat Push switch,   HID button 0
                   15,                // TDC Push switch,         HID button 1
                   6, 7,              // 2-Way Speedbrake switch, HID buttons 2 and 3
                   8, 9,              // 2-Way Comms switch,      HID buttons 4 and 5
                   10, 14, 16};       // 3-Way Misc switch,       HID buttons 6, 7 and 8


// Define Analog pins for each axis
const int xTDC_Axis  = A0;            // Analog pin for TDC X-Axis
const int yTDC_Axis  = A1;            // Analog pin for TDC Y-Axis
  

// Default states for axes and buttons
int xTDC = 0;
int yTDC = 0;

bool hVal_Changed = false;
int sBtn_CurrState, hBtn_CurrState;
int sBtn_LastState[uBtns] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int hBtn_LastState[hBtns] = {0, 0, 0, 0};


// Joystick (HID ID, Type, Buttons, Hats, X-Aix, Y-Axis, Z-Axis, Rx-Axis, Ry-Axis, Rz-Axis, Rudder, Throttle, Accelerator, Brake, Steering);
Joystick_ THQ(HID_ID, JOYSTICK_TYPE_JOYSTICK, uBtns, 1, true, true, false, false, false, false, false, false, false, false, false);
const bool initAutoSendState = true;


void setup() {
  // Initialise digital pin modes and enable pull-up resistors
  for (i=0; i<uBtns; i++) {
    pinMode(sBtn[i], INPUT);
    digitalWrite (sBtn[i], HIGH);
  }

  for (i=0; i<hBtns; i++) {
    pinMode(sHat[i], INPUT);
    digitalWrite (sHat[i], HIGH);
  }

  THQ.begin(initAutoSendState);

  #ifdef DEBUGTHQ
    Serial.begin(9600);
  #endif
}

void loop() {
  // Read axes and send to HID
  // X-Axis
  xTDC = analogRead(xTDC_Axis);
  xTDC = map(xTDC, 0, 1023, 0, 255);   // scale resolution to reduce jittering, need to test if necessary
  THQ.setXAxis(xTDC);

  // Y-Axis
  yTDC = analogRead(yTDC_Axis);
  yTDC = map(yTDC, 0, 1023, 0, 255);  // scale resolution to reduce jittering, need to test if necessary
  THQ.setYAxis(yTDC);

  // Check for change in state of each hat direction and if it has changed send new state to HID
  for (i=0; i<hBtns; i++)  {
    hBtn_CurrState = !digitalRead(sHat[i]);
    if (hBtn_CurrState != hBtn_LastState[i]) {
      hVal_Changed = true;
      hBtn_LastState[i] = hBtn_CurrState;
    }
  }

  // Something has changed since last check
  if (hVal_Changed) {
    // No switch is currently active, release hat switch
    if ((hBtn_LastState[0] == 0) && 
        (hBtn_LastState[1] == 0) &&
        (hBtn_LastState[2] == 0) &&
        (hBtn_LastState[3] == 0)) {
          THQ.setHatSwitch(0, -1);
        }

    // Up Switch pressed
    if (hBtn_LastState[0] == 1) {
      THQ.setHatSwitch(0, 0);
      #ifdef DEBUGTHQ 
        Serial.println("POV Up pressed!"); 
      #endif
    }
    // Right Switch pressed
    if (hBtn_LastState[1] == 1) {
      THQ.setHatSwitch(0, 90);
    }
    // Down Switch pressed
    if (hBtn_LastState[2] == 1) {
      THQ.setHatSwitch(0, 180);
    }
    // Left Switch pressed
    if (hBtn_LastState[3] == 1) {
      THQ.setHatSwitch(0, 270);
    }

  }

  // Check for change in state of each button and if it has changed send new state to HID
  for (i=0; i<uBtns; i++) {
    sBtn_CurrState = !digitalRead(sBtn[i]);
    if (sBtn_CurrState != sBtn_LastState[i]) {
      THQ.setButton(i, sBtn_CurrState);
      sBtn_LastState[i] = sBtn_CurrState;  
      #ifdef DEBUGTHQ
        if (sBtn_CurrState == HIGH) {
          Serial.print("Button ");
          Serial.print(i);
          Serial.println(" pressed");
        }
        if (sBtn_CurrState == LOW) {
          Serial.print("Button ");
          Serial.print(i);
          Serial.println(" released");
        }
      #endif
    }
  }

  delay (tDelay);
}
