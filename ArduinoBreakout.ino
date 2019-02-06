//
// ArduinoBreakout - Arduino Project for Weeks 10 - 14.
// Created by Fraser Barker (1600196) on 25/11/2016.
//

//OLED Screen Includes
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//7-seg Includes
#include <InvertedTM1638.h>
#include <TM1638.h>
#include <TM1638QYF.h>
#include <TM1640.h>
#include <TM16XX.h>
#include <TM16XXFonts.h>

//Rotary Encoder Includes
#include <Interrupt.h>
#include <TimerListener.h>
#include <RifIntEncoder.h>

//Servo Includes
#include <Servo.h>

//7seg Defines
#define TM1638_STB 5  //attaches the 7seg STB pin to pin 5 on the Arduino
#define TM1638_CLK 6  //attaches the 7seg CLK pin to pin 6 on the Arduino
#define TM1638_DIO 7  //attaches the 7seg DIO pin to pin 7 on the Arduino
TM1638 module(TM1638_DIO, TM1638_CLK, TM1638_STB);

//OLED Screen Defines
#define OLED_MOSI 11  //attaches the OLED SDA pin to pin 11 on the Arduino
#define OLED_CLK 13 //attaches the OLED SCK pin to pin 13 on the Arduino
#define OLED_DC 9 //attaches the OLED DC pin to pin 9 on the Arduino
#define OLED_CS 12  
#define OLED_RESET 10 //attaches the OLED RES pin to pin 10 on the Arduino
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

//Rotary Encoder Defines
#define THROTTLE_ENCODERA_PIN 2 //attaches the Rotary Encoder to pin 2 on the Arduino
#define THROTTLE_ENCODERB_PIN 3 //attaches the Rotary Encoder to pin 3 on the Arduino
#define THROTTLEMIN 1 //defining the minimum rotation for the Rotary Encoder. Prevents the platform going off the left side of the screen.
#define THROTTLEMAX 107 //defining the maximum rotation for the Rotary Encoder. Prevents the platform going off the right side of the screen.
RifIntEncoder knob; //create RifIntEncoder object to control a Rotary Encoder.

//Servo Defines
Servo myservo;
#define pinServo 8 //sets which pin the servo is set to on the Arduino.

//Variables
int pos = 0;  //variable to store the servo position.
int ballX = 5;  
int ballY = 5;  
float ballSpeed = 60;  
int score = 0; 
int diffLED = 1;  //variable to show difficulty level, this is output on the 7seg LEDs.
int ledIntensity = 1;
bool moveXPos = true; //check if the ball is moving in positive X direction.
bool moveYPos = true; //check if the ball is moving in positive Y direction.
bool pause = true;  

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC);  //show initial boot screen on OLED.
  display.display();  //display boot screen on OLED.
  delay(1000);  
  display.clearDisplay();
  display.setTextSize(1); 
  display.setTextColor(WHITE);  
  Interrupt::initialiseStaticISRVectorTable() ; 
  knob.init(THROTTLE_ENCODERA_PIN, THROTTLE_ENCODERB_PIN);  //initialises the Rotary Encoder to pins 2 & 3 on the Arduino.
  knob.setMax(THROTTLEMAX); //sets minimum rotation value for Rotary Encoder.
  knob.setMin(THROTTLEMIN); //sets maximum rotation value for Rotary Encoder.

  reset();
  knob.setPos(54);  //set the platform in the centre of the screen.
  myservo.attach(pinServo);  //attaches the servo to pin 8 on the Arduino

  myservo.write(pos); //set the servo to position 0.
  display.clearDisplay(); //clear the OLED.
  module.clearDisplay();  //clear the 7seg.
  module.setupDisplay(true, ledIntensity); //(true) - set LEDs on/off. (ledIntensity) - LED intensity.
  module.setLEDs(diffLED);  //show difficulty level 1 on the 7seg LEDs.

  
}

void loop(){
  changeDiff(); 
  pauseGame();  
  platformMove(); 
  ball(); 
  delay(ballSpeed); //delays the screen output by the ballSpeed variable, thus increasing/decreasing the difficulty of the game.
  module.setDisplayToDecNumber(score, 0 ,false);  //output score on 7-seg.
}

void ball(){  //collate all properties of the ball
  drawBall(); //draws the ball on the OLED.
  if (!pause){
    moveBall(); //makes the ball move and prevents the ball moving if the game is paused.
  }
  ballCollision();  //check if the ball hits any of the walls or the players platform.
  reset();  //resets the balls position if button 1 on the 7seg is pressed.
}


void platformMove() { // constantly re-draw the platform on the OLED screen at the knob's value/position.
  byte x1 = knob.getPos();
  byte y1 = 48;
  byte x2 = x1 + 20;
  byte y2 = y1;
  byte x3 = x2;
  byte y3 = y2 + 5;
  byte x4 = x3 - 20;
  byte y4 = y3;
  display.clearDisplay();
  display.drawLine(x1, y1, x2, y2, WHITE);
  display.drawLine(x2, y2, x3, y3, WHITE);
  display.drawLine(x3, y3, x4, y4, WHITE);
  display.drawLine(x4, y4, x1, y1, WHITE);
  display.display();
}

void drawBall(){  // draw the ball on the OLED screen at ballX,ballY 
  display.drawCircle(ballX, ballY, 3, WHITE);
  display.display();  // display new elements on the screen
}

void moveBall(){
  //diagonal start (right and down)
  if (moveXPos){
    ballX++;
  } else {
    ballX--;
  }
  if (moveYPos){
    ballY++;
  } else {
    ballY--;
  }  
}

void ballCollision(){ // check if the ball hits either of the walls or the platform.
  if (ballX >= 122){  
    moveXPos = false;
  }
  if (ballY == 43 && ballX > knob.getPos() && ballX < (knob.getPos() + 20)){  
    moveYPos = false; 
    score++;  //add a point to the players score as they successfully bounced the ball off the platform.
    if (score % 2 == 0 && pos < 180)
    {
      pos += 45;
      myservo.write(pos);
      delay(15);
      if (pos == 180) 
      {
        pos = -45;  //set pos equal to -45, upon entering the loop above again, the servo should rotate back to 0 degrees.
      }
    }
  }
  if (ballX == 1){  
    moveXPos = true;
  }
  if (ballY == 1){  
    moveYPos = true;  
  }
}

void reset(){ //reset the game if the player presses button 1 on the 7seg.
  if (module.getButtons() == 1) { //if the first button on the 7seg is pressed.
    ballX = 5;  
    ballY = 5;  
    ballSpeed = 60; //reset the value of ballSpeed to 60, putting it back to difficulty one.
    score = 0;  
    pos = 0;  
    myservo.write(pos); //rotate the servo to the value of pos(0).
    pause = true; 
    ledIntensity = 1;
    diffLED = 1;
    module.setLEDs(diffLED);  //output the value of diffLED on the 7seg LEDs.
    delay(300); 
  }
}

void pauseGame(){ 
  if (module.getButtons() == 2){  
    pause = !pause; 
    delay(300); 
  }
}

void changeDiff(){  //change the difficulty of the game via altering the value of the ballSpeed variable
  if (module.getButtons() == 4){  
    diffLED += diffLED;
    ledIntensity++;
    ballSpeed -= 7.5; 
    if (diffLED > 128 && ballSpeed < 1) 
    {                                   
      diffLED = 128;
      ledIntensity = 8;  
      ballSpeed = 0;  
    }
    module.setLEDs(diffLED); 
    delay(300); 
  }
  else if (module.getButtons() == 8){ 
    diffLED = diffLED / 2;
    ledIntensity--;  
    ballSpeed += 7.5; 
    if (diffLED < 1 && ballSpeed > 60)  
    {                                   
      diffLED = 1;
      ledIntensity = 1;  
      ballSpeed = 60; 
    }
    module.setLEDs(diffLED);  
    delay(300); 
  }
}



