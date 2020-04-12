// ConstantSpeed.pde
// -*- mode: C++ -*-
//
// Shows how to run AccelStepper in the simplest,
// fixed speed mode with no accelerations
/// \author  Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2009 Mike McCauley
// $Id: ConstantSpeed.pde,v 1.1 2011/01/05 01:51:01 mikem Exp mikem $

#include <AccelStepper.h>
#include <Bounce2.h>
#include <EEPROM.h>
#include <TMCStepper.h>
int addr = 0;

// AccelStepper stepper; // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
#define dirPin 4  // 2=x, 3=y, 4=z
#define stepPin 7  // 5=x, 6=y, 7=z
#define stepsPerRevolution 200
#define motorInterfaceType 1
#define CS_PIN           10 // Chip select
#define R_SENSE 0.075f

TMC5160Stepper driver = TMC5160Stepper(CS_PIN, R_SENSE);

//AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);
AccelStepper stepper = AccelStepper(stepper.DRIVER, stepPin, dirPin);
int motorSpeed;

  Bounce btnIncrease = Bounce();
  Bounce btnDecrease = Bounce();
  Bounce btnReset = Bounce();
  
 int bImode=0;
 int bDmode=0;
 int bRmode=0;
void setup()
{    
    SPI.begin();
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    driver.begin();             // Initiate pins and registeries
    driver.rms_current(300);    // Set stepper current to 600mA. The command is the same as command TMC2130.setCurrent(600, 0.11, 0.5);
    driver.en_pwm_mode(1);      // Enable extremely quiet stepping
    //driver.pwm_autoscale(1);
    driver.microsteps(32);
    
  Serial.begin(9600);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(8, OUTPUT);  //enable
  pinMode(A0,OUTPUT);
  digitalWrite(A0, LOW);
  //pinMode(A1,INPUT_PULLUP);
  //pinMode(A2,INPUT_PULLUP);
  //pinMode(A3,INPUT_PULLUP);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  
  btnIncrease.attach(A1,INPUT_PULLUP);
  btnDecrease.attach(A2,INPUT_PULLUP);
  btnReset.attach(A3,INPUT_PULLUP);
  btnIncrease.interval(25);
  btnDecrease.interval(25);
  btnReset.interval(25);

  digitalWrite(8,LOW);
  //400 steps * 32 microsteps = 12,800 steps per rev
  //1 rpm = 12800 / min = 12800/60 steps per sec = 213.333333
  stepper.setAcceleration(750.0);
  
  //stepper.moveTo(500000.0);
   
  stepper.setMaxSpeed(10000);
  EEPROM.get(addr, motorSpeed);
  if(motorSpeed==0) 
  {motorSpeed = -250;}
  stepper.setSpeed(motorSpeed); 
  Serial.println("setup, motor speed=" + motorSpeed);
}

void loop()
{    
  checkButton(btnIncrease, bImode);
  checkButton(btnDecrease, bDmode);
  checkButton(btnReset, bRmode);

  if(bDmode==25)
  {
    motorSpeed--;
    Serial.print("Decreasing motorSpeed:");
    Serial.println(motorSpeed);
  }
  if(bImode==25)
  {
    motorSpeed++;
    Serial.print("Increasing motorSpeed:");
    Serial.println(motorSpeed);
  }
  if(bRmode==25)
  {
    motorSpeed=-250;
  }
  if(bRmode==20)
  {
    Serial.print("Saving motorSpeed:");
    Serial.println(motorSpeed);
    EEPROM.put(addr, motorSpeed);
  }
  if(bImode>35||bImode==20||bImode==25)
  {
    Serial.print("Increase:");
    Serial.println(bImode);
  }
  if(bDmode>35||bDmode==20||bDmode==25)
  {
    Serial.print("Decrease:");
    Serial.println(bDmode);
  }
  if(bRmode>35||bRmode==20||bRmode==25)
  {
    Serial.print("Reset:");
    Serial.println(bRmode);
  }
  //stepper.run();
  stepper.setSpeed(motorSpeed);
  //stepper.setSpeed(-10000);
  stepper.runSpeed();
}

int checkButton(Bounce& button, int& mode) 
{
  button.update();
  int doubleClickTime = 250;
  int longClickTime = 2000;
  switch( mode )
  {
    case 0://------------------------ button up and in restmode
      if ( button.read() == LOW )
      {                       
        mode = 10;
      }
      break;
    case 10://------------------------ button down, waiting
      // wait for button release or...
      // or wait for doubleClick timeout (button down too long for double click)
      // or wait for long click (duration longer than longClickTime)
      if (button.duration() > doubleClickTime)
      {
        mode = 15;
      }
      else if ( button.read() == HIGH )
      {
        mode = 30;
      }
      break;
    case 15://------------------------ button down, passed double click time
      // no more double click possible - now either long click or back to 0
      if (button.duration() > longClickTime)
      {
        mode = 20;
      }
      else if ( button.read() == HIGH )
      {                       
        mode = 25;
      }
      break;
    case 20://------------------------ button up, long click
      // wait for up, then reset
      mode = 21;
      break;
    case 21://------------------------ button up, single click
      if ( button.read() == HIGH )
      {
        mode = 0;
      }
      break;
    case 25://------------------------ button up, single click
      mode = 0;
      break;
    case 30://------------------------ button up, wait for double click
      // wait for second click of the double click
      // or a single click if we time out      
      if (button.duration() > doubleClickTime)
      {
        mode = 25; //single
      }
      if ( button.read() == LOW )
      {
        mode = 35; //initiating double
      }
      break;
    case 35://------------------------ button down, waiting
      //wait for up before dc timeout
      // or long click?
      if (button.duration() > longClickTime)
      {
        mode = 40;  //long dc
      }
      if ( button.read() == HIGH )
      {
        mode = 45;
      }
      break;
    case 40://------------------------ button up, long double click
      mode = 46;
      break;
    case 46://------------------------ button up, double click
            // wait for up, then reset
      if ( button.read() == HIGH )
      {
        mode = 0;
      }
      break;
    case 45://------------------------ button up, double click
      mode = 0;
      break;
  }//switch
  return mode;
}
