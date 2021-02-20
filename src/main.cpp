/**
 * Standard libraries
 */ 
#include <Arduino.h>
#include <EEPROM.h>

/**
 * Buttons and endstops
 */
#include "ezButton.h"
#define BUTTON_RUN 2
#define BUTTON_UP 3
#define BUTTON_DOWN 4
#define ENDSTOP_CART 5
#define ENDSTOP_BED 6
ezButton buttonRun(BUTTON_RUN);
ezButton buttonUp(BUTTON_UP);
ezButton buttonDown(BUTTON_DOWN);

ezButton endstopBed(ENDSTOP_BED);
ezButton endstopCart(ENDSTOP_CART);


/**
 * Steppers
 */
#include "A4988.h"
#define STEPPER_CART_STEPS 200
#define STEPPER_CART_RPM 90
#define STEPPER_CART_MICRO 8
#define STEPPER_CART_DIR 7
#define STEPPER_CART_STEP 8
A4988 stepperCart(STEPPER_CART_STEPS,STEPPER_CART_DIR,STEPPER_CART_STEP);

#define STEPPER_BED_STEPS 200
#define STEPPER_BED_RPM 40
#define STEPPER_BED_MICRO 8
#define STEPPER_BED_DIR 9
#define STEPPER_BED_STEP 10
A4988 stepperBed(STEPPER_BED_STEPS,STEPPER_BED_DIR,STEPPER_BED_STEP);

/**
 * DC motors
 */
#include "L298N.h"
#define MOTOR_ENABLE 11
#define MOTOR_IN1 13
#define MOTOR_IN2 12
#define MOTOR_SPEED 255
#define MOTOR_SPEED_EXTRUDE 55
L298N motorPush(MOTOR_ENABLE,MOTOR_IN1,MOTOR_IN2);

/**
 * Other variables and functions
 */
void buttonRefresh(){
  buttonRun.loop();
  buttonUp.loop();
  buttonDown.loop();
  endstopBed.loop();
  endstopCart.loop();
}

void homeCart(){
  // home the cart
  buttonRefresh();
  stepperCart.move(250);
  delay(10);
  stepperCart.startMove(-10000); //some steps to reach the endstop
  Serial.println("Stepper cart -> going to home");
  while(!endstopCart.isPressed()){
      buttonRefresh();
      stepperCart.nextAction();
  }
  stepperCart.startBrake();
  Serial.println("Stepper cart -> at home possition");
}

void homeBed(){
  // home the bed
  buttonRefresh();
  stepperBed.startMove(3600); //some steps to reach the endstop
  Serial.println("Stepper bed -> going to home");
  while(!endstopBed.isPressed()){
      buttonRefresh();
      stepperBed.nextAction();
  }
  stepperBed.startBrake();
  Serial.println("Stepper bed -> at home possition");
}

/**
 * EEPROM
 */
unsigned int MEMORY_RUN_COUNT_ADDR = 10;
void writeIntIntoEEPROM(int address, int number)
{ 
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}

int readIntFromEEPROM(int address)
{
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}

void increaseRunCounter(){
  int runcount = readIntFromEEPROM(MEMORY_RUN_COUNT_ADDR);
  writeIntIntoEEPROM(MEMORY_RUN_COUNT_ADDR, runcount+1);
}

void printRunCount(){
  Serial.print("COUNTER - RunCount: ");
  Serial.println(readIntFromEEPROM(MEMORY_RUN_COUNT_ADDR));
}

void counterResetRutine(){
  buttonRefresh();
  if(buttonDown.isPressed() && buttonUp.isPressed()){
    Serial.println("COUNTER - Counter reset required !");
    writeIntIntoEEPROM(MEMORY_RUN_COUNT_ADDR,0);
    printRunCount();
    Serial.println("COUNTER - Waiting to button release");
    while(buttonDown.isPressed() && buttonUp.isPressed()){
      buttonRefresh();
      delay(5);
    }
    Serial.println("COUNTER - Button released, continue.");
  }
}

/**
 * Setup
 */
void setup(){
  Serial.begin(9600); //prepare serial
  Serial.println("---   FireCaulkGun   ---");
  printRunCount();

  //check if is required to reset counter
  counterResetRutine();

  //prepare steppers
  stepperBed.begin(STEPPER_BED_RPM,STEPPER_BED_MICRO);
  stepperCart.begin(STEPPER_CART_RPM,STEPPER_CART_MICRO);

  ///prepare motor
  analogWrite(MOTOR_ENABLE,255);
  motorPush.setSpeed(MOTOR_SPEED);


  //SETUP RUTINE
  stepperCart.move(250);
  //homeCart();
  homeBed();
}


void loop(){
  buttonRefresh(); //refresh buttons

  //check buttons
  if(buttonUp.isPressed()){
    Serial.println("MOTOR - UP");
    while(!buttonUp.isReleased()){
          //Serial.println("MOTOR - UP");
          buttonRefresh();
          motorPush.backward();
    }
  }
  if(buttonDown.isPressed()){
    Serial.println("MOTOR - DOWN");
    while(!buttonDown.isReleased()){
          //Serial.println("MOTOR - DOWN");
          buttonRefresh();
          motorPush.forward();
    }
  }
  motorPush.stop();

  //run button
  if(buttonRun.isPressed()){
    Serial.println("RUN - start");
    homeCart();
    stepperCart.move(5325); //cart to back possition
    motorPush.setSpeed(MOTOR_SPEED_EXTRUDE);
    motorPush.forward(); //start extrude
    stepperBed.rotate(360*2);
    motorPush.stop();
    stepperBed.rotate(360*1);
    motorPush.setSpeed(MOTOR_SPEED);
    motorPush.backward();
    delay(100);
    motorPush.stop();
    Serial.println("RUN - done, going to home");
    increaseRunCounter();
    printRunCount();
    homeCart();
    delay(200);
  }
}