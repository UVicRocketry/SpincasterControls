/*
  UVic Rocketry Proulsion Team - Spincaster Controls

  This software will allow users to set the amount of time
  the spincaster will rotate for and automatically shut it off
  in the event of the control system fails.

  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 4
 * LCD D5 pin to digital pin 3
 * LCD D6 pin to digital pin 2
 * LCD D7 pin to digital pin 5
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * LCD LED+ to 5V
 * LCD LED- to ground
 * 10K pot resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 * Rotary SW to digital pin 10
 * Rotary DT to digital pin 9
 * Rotary CLK to digital pin 8

 Joshua Piuti 27/9/2021
*/

// include the library code:
#include <LiquidCrystal.h>

#define CLK 8
#define DT 9
#define SW 10
#define RL 7

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 4, d5 = 3, d6 = 2, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int counter = 1;
int currentStateCLK;
int lastStateCLK;
String currentDir ="";
unsigned long lastButtonPress = 0;
int minRem = 0;
int secRem = 0;
char timeBuf[7] = "       ";
char minBuf[3] = "   ";

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print startup screen to LCD
  lcd.print("Click to Start");
  lcd.setCursor(9, 1);
  lcd.print("Time:01");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Set encoder pins as inputs
  pinMode(CLK,INPUT);
  pinMode(DT,INPUT);
  pinMode(SW, INPUT_PULLUP);

  // Set relay pin as output
  pinMode(RL, OUTPUT);
  digitalWrite(RL, LOW);

  // Setup Serial Monitor
  Serial.begin(9600);

  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);
}//setup

void loop() {
  // Read the current state of CLK
  currentStateCLK = digitalRead(CLK);

  // Turns relay off for safety
  digitalWrite(RL, LOW);

  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){

    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DT) != currentStateCLK) {
      if(counter > 0){
        counter --;
       currentDir ="CCW";
      }//if
    } else {
      // Encoder is rotating CW so increment, max is 99
      if(counter < 99){
        counter ++;
        currentDir ="CW";
      }//if
    }//if

    // displays the selected number of minutes to run
    lcd.setCursor(14, 1);
    sprintf(minBuf, "%02i", counter);
    lcd.print(minBuf);
  }//if

  // Remember last CLK state
  lastStateCLK = currentStateCLK;

  // Read the button state
  int btnState = digitalRead(SW);

  //If we detect LOW signal, button is pressed
  if (btnState == LOW && counter > 0) {
    //if 50ms have passed since last LOW pulse, it means that the
    //button has been pressed, released and pressed again
    if (millis() - lastButtonPress > 50) {
      lastButtonPress = millis();
      // Read the button state
      btnState = digitalRead(SW);

      // Sets up timer in minutes and seconds
      minRem = counter - 1;
      secRem = 59;

      // Sets screen to countdown
      lcd.clear();
      lcd.print("Caster Running");
      lcd.setCursor(5, 1);
      lcd.print("TimeR:");
      // Prints current time remaining to LCD
      sprintf(timeBuf, "%02i:%02i", minRem, secRem);
      lcd.setCursor(11, 1);
      lcd.print(timeBuf);

      // Turns relay on
      digitalWrite(RL, HIGH);

      // Counts down until time elapsed
      while(true){
        delay(1000);
        
        if(secRem <= 0){
          if(minRem <= 0){
            break;
          }//if
          minRem--;
          secRem = 59;
        } else {
          secRem--;
        }//if else

        // Prints current time remaining to LCD
        sprintf(timeBuf, "%02i:%02i", minRem, secRem);
        lcd.setCursor(11, 1);
        lcd.print(timeBuf);
      }//while
    }//if

    // Turns off relay when countdown done
    digitalWrite(RL, LOW);

    // Prints screen and prompts user to push rotary encoder button to get back to start screen
    lcd.clear();
    lcd.print("Casting Finished");
    lcd.setCursor(0, 1);
    lcd.print("Press But to RST");
    lastButtonPress = millis();
    btnState = digitalRead(SW);

    // Waits until button is pressed to go back to start screen
    while(btnState == HIGH){
      // Read the button state
      btnState = digitalRead(SW);
    };//do while

    // Logs last button press
    lastButtonPress = millis();

    // Print startup screen to LCD
    lcd.clear();
    lcd.print("Click to Start");
    lcd.setCursor(9, 1);
    lcd.print("Time:01");
    lcd.setCursor(14, 1);
    sprintf(minBuf, "%02i", counter);
    lcd.print(minBuf);
  }//if

  // Put in a slight delay to help debounce the reading
  delay(1);
}//loop
