#include <Arduino.h>

/**
 * Buttons
 * */
#include "ezButton.h"
#define runButton 5
ezButton button(runButton);

/**
 * STEPPER
 * */
#include "A4988.h"
#define MOTOR_STEPS 200
#define RPM 30
#define MICROSTEPS 8
#define DIR 8
#define STEP 9
A4988 stepper(MOTOR_STEPS, DIR, STEP);


/**
 * PUSH MOTOR
 * */
#include <L298N.h>
const unsigned int IN1 = 2;
const unsigned int IN2 = 3;
unsigned short theSpeed = 255;
L298N motor(IN1, IN2);

/**
 * Bed Allign
 * 
 * */
#define BED_ALIGN_PIN A5


/**
 * Other variables
 */
const int FB_DOUBLE_PRESS_TIME = 1000;
const int FB_HOLD_PRESS_TIME = 2000;
unsigned int fb_lastPressTime = 0;
bool fb_waitForDoubleClick = false;

int brake = 0;
void setup() {
  Serial.begin(9600);

  //pinMode(BED_ALIGN_PIN,INPUT_PULLUP);

  pinMode(LED_BUILTIN, OUTPUT);

  //stepper
  stepper.begin(RPM, MICROSTEPS);  

  //motor
  motor.setSpeed(theSpeed);
}

void motorCallback(){

}


void fn1(){
  Serial.println("single click");
   Serial.println("The button is pressed");
      motor.forward();
      delay(1000);
      motor.backward();
      delay(1000);
      motor.stop();
      stepper.rotate(360);
      delay(1000);
}

void fn2(){
  Serial.println("double click");
  if(motor.getDirection() == motor.FORWARD){
    motor.backward();
  }else{
    motor.forward();
  }
  

}

void buttonRutine(void (*singleClick)(), void (*doubleClick)()){
    button.loop();


    if(button.isPressed()){
      
      Serial.println(fb_waitForDoubleClick);

      if(fb_waitForDoubleClick == false){
        fb_waitForDoubleClick = true;
      }else if(fb_waitForDoubleClick == true){
        if(millis() < (fb_lastPressTime+FB_DOUBLE_PRESS_TIME)){
          
          doubleClick();
          //Serial.println("Double click");
          fb_waitForDoubleClick = false;
        }
      }
      fb_lastPressTime = millis();
    }

    if(fb_waitForDoubleClick == true){
        if(millis() > (fb_lastPressTime+FB_DOUBLE_PRESS_TIME)){
          
          //Serial.println("Single click");
          singleClick();
          
          fb_waitForDoubleClick = false;
          fb_lastPressTime = millis();
        }
    }

  }


void loop(){
  buttonRutine(fn1,fn2);


  
  
  /*if(button.isPressed()){
    if(millis() < (pressedTime+500)){
      Serial.println("double click");
    }
    else if(millis() > (pressedTime+500)){
        Serial.println("single click");
    }
    pressedTime = millis();
    Serial.println(pressedTime);
  }*/

  
  //Serial.println(pressedTime);


   
    /*if(button.isPressed()){
      Serial.println("The button is pressed");
      motor.forward();
      delay(1000);
      motor.backward();
      delay(1000);
      motor.stop();
      stepper.rotate(360);
      delay(1000);
    }

    if(button.isReleased()){
      Serial.println("The button is released");
    }*/

}

