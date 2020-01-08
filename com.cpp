/*This file contains the functions that control the bluetooth module, the IR receiver and the door sensor*/
#include <IRremote.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Interval.h>
#include "com.h"
#include "regulation.h"
#include "sdCard.h"

/*USER SETTINGS*/
bool useBluetooth = 1;
bool useDoorSensor = 1;
#define RX_BLUETOOTH 11 //The pin where the RX pin of the bluetooth module is attached. Default wiring : RX on D11
#define TX_BLUETOOTH 10 //The pin where the TX pin of the bluetooth module is attached. Default wiring : TX on D10
#define doorSensor 7 //The pin where the sensor limit of the door is attached. Default wiring : status on D7
#define receiver 5 //The pin where the IR receiver is attached. Default wiring : data on D5
Interval updateInterval(5000); //Delay between each refresh on the Couv'Oeuf application

/*PROGRAM SETTINGS*/
SoftwareSerial bt (TX_BLUETOOTH, RX_BLUETOOTH);
LiquidCrystal_I2C lcd(0x27,16,2 );  // set the LCD address to 0x27 for a 16 chars and 2 line display
IRrecv ir(receiver);
decode_results resultsIr;
bool lcdPower = 0;
unsigned int menuActive = 0;
unsigned int selection = 1;
bool door = false;
byte upArrow[8]={ B00100, B01110, B11111, B01110, B01110, B01110, B01110, B01110 };
byte downArrow[8]={ B01110, B01110, B01110, B01110, B01110, B11111, B01110, B00100 };
byte egg[8]={ B00000, B00100, B01110, B10111, B11101, B11111, B11011, B01110 };

void initializeIR(){
  //Initialize the IR receiver
  Serial.print("> Initializing the infrared communication..........");
  ir.enableIRIn();
  ir.blink13(true);
  Serial.println("OK !\n");
}

void initializeLCD(){
  lcd.init();
  lcd.noDisplay();
  lcd.noBacklight();
  lcd.createChar(0, upArrow);
  lcd.createChar(1, downArrow);
  lcd.createChar(2, egg);
}

void initializeBT(){
  Serial.print("> Initializing the bluetooth module..........");
  bt.begin(57600);
  Serial.println("OK !\n");
  bt.println("r");
}

void initializeDoor(){
  Serial.print("> Initializing the limit sensor for the door..........");
  pinMode(doorSensor, INPUT_PULLUP);
  Serial.println("OK !\n");
}

void interceptCommands(){
  //Try to recover a command which is send via the bluetooth module, the serial monitor or the ir receiver
  String message;
  if(useBluetooth){
     while (bt.available()){
      message = bt.readString();
    }
  }
  while (Serial.available()){
      message = Serial.readString();
  }
  //Serial.println("Message intercepte : " + message);
  if(message[0] == 'a') addEgg(message[1]);
  else if(message[0] == 'd') deleteEgg(message[1]);
  if(ir.decode(&resultsIr)){
    long unsigned int result = resultsIr.value;
    digitalWrite(LED_BUILTIN, HIGH);
  
    if(result == 0xFF42BD) emergencyStop();
    else if(result == 0xFF45FA) displayData();
    else if(result == 0xFF52AD) backup();
    //Wake up the screen when a key is pressed
    else if(result && !lcdPower) { refreshDisplay(0); }
    //Switch off the screen when the 0 key is pressed
    else if(result == 0xFF4AB5 && lcdPower && menuActive == 0) { refreshDisplay(-1); }
    //Return to the home menu when the 0 key is pressed
    else if(result == 0xFF4AB5 && lcdPower && menuActive != 0) { refreshDisplay(0); }

    //OK button
    else if(result == 0xFF02FD && lcdPower && menuActive) { 
      if(menuActive == 1){
        menuActive = selection*10;
        lcd.clear();
        lcd.setCursor(0,0);
        String location = (selection <= 9) ? ("0" + String(selection)) : String(selection);
        lcd.print("Location " + location);
        lcd.setCursor(0,1);
        lcd.write(2);
        lcd.print(" Please wait...");
      }
      if(menuActive == 3) { addEgg(selection); refreshDisplay(0); }
      else if(menuActive == 4) { deleteEgg(selection); refreshDisplay(0); }
    }
    //Menu 1
    else if(result == 0xFF6897 && lcdPower && !menuActive) { selection = 1; refreshDisplay(1); }
    //Menu 2
    else if(result ==  0xFF9867 && lcdPower && !menuActive) { refreshDisplay(2); }
    //Menu 3
    else if(result ==  0xFFB04F && lcdPower && !menuActive) { selection = 1; refreshDisplay(3); }
    //Menu 4
    else if(result == 0xFF30CF && lcdPower && !menuActive) { selection = 1; refreshDisplay(4); }    
    //Up arrow
    else if(result == 0xFF629D && lcdPower) { if(selection >= 10) { selection = 10; } else { selection++; } refreshDisplay(menuActive); }
    //Down arrow
    else if(result == 0xFFA857 && lcdPower) { if(selection <= 1) { selection = 1; } else { selection--; } refreshDisplay(menuActive); }
    ir.resume();
  }
  digitalWrite(LED_BUILTIN, LOW);
}

void refreshDisplay(int menu){
  if(menu == -1){
    lcd.noDisplay();
    lcd.noBacklight();
    lcdPower = false;
  }
  else{
    lcd.clear();
    lcd.display();
    lcd.backlight();
    lcdPower = true;
  }
  if(menu == 0){
    lcd.setCursor(0,0);
    lcd.print("1|Eggs  2|Device");
    lcd.setCursor(0,1);
    lcd.print("3|+Egg    4|-Egg");
  }
  else if(menu == 2){
    lcd.setCursor(0,0);
    lcd.print("T:" + String(saveTemperature) + "C   DOOR");
    lcd.setCursor(0,1);
    if(door) lcd.print("H:" + String(saveHumidity) + "%   OPEN");
    else lcd.print("H:" + String(saveHumidity) + "%  CLOSED");
  }
  else if(menu == 1 || menu == 3 || menu == 4){
    lcd.setCursor(0,0);
    lcd.print("What's  location");
    lcd.setCursor(5,1);
    if(selection == 10){
      lcd.print("  10 ");
      lcd.write(1);
    }
    else if(selection == 1){
      lcd.write(0);
      lcd.print(" 01  ");
    }
    else{
      lcd.write(0);
      lcd.print(" 0" + String(selection) + " ");
      lcd.write(1);
    }
  }
  menuActive = menu;
}

void refreshMenuIncubation(){
  if(menuActive >= 10){
    lcd.clear();
    lcd.setCursor(0,0);
    String location = (selection <= 9) ? ("0" + String(selection)) : String(selection);
    if(inc[selection-1] == -1){
      lcd.print("Location " + location);
      lcd.setCursor(0,1);
      lcd.write(2);
      lcd.print(" EMPTY");
    }
    else{
      unsigned long int data = temp[selection-1] + inc[selection-1];
      unsigned int percent = (100*data)/1814400;
      if(percent >= 100) lcd.print("Location " + location + " " + String(percent) + "%");
      else if(percent >= 10) lcd.print("Location " + location + "  " + String(percent) + "%");
      else if(percent < 10) lcd.print("Location " + location + "   " + String(percent) + "%");
      lcd.setCursor(0,1);
      lcd.write(2);
      lcd.print(" " + String(data) + "s");
    }
  }
}

void checkDoor(){
  /*If the door sensor return 1 and the door was closed -> the door is now open 
  If the door sensor return 0 and the door was opened -> the door is now closed*/
  digitalWrite(LED_BUILTIN, HIGH);
  bool sensor = !digitalRead(doorSensor);
  if(sensor && !door) { door = true; doorStatus(true); }
  else if(!sensor && door) { door = false; doorStatus(false); }
  digitalWrite(LED_BUILTIN, LOW);
}

void updateApplication(){
  //update the application data
  if(updateInterval.isElapsed()){
    //Refresh serial monitor
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("- - - DATA REFRESHING - - -");
    Serial.println("> Internal temperature : " + String(saveTemperature) + " °C");
    Serial.println("> Internal humidity level : " + String(saveHumidity) + " °C");
    Serial.println("Location status : ");
    for(int i = 0; i <= 9; i++){ 
      if(inc[i] != -1) Serial.println("Location " + String(i+1) + " : OCCUPIED");
      else Serial.println("Location " + String(i+1) + " : FREE");
    }
    if(door) Serial.println("> Door status : OPEN");
    else Serial.println("> Door status : CLOSED\n");
    //Refresh application data
  }
  digitalWrite(LED_BUILTIN, LOW);
}

void doorStatus(bool stat){
  digitalWrite(LED_BUILTIN, HIGH);
  //Update the door status
  if(stat) { Serial.println("> WARNING : The door is now open ! Close the door to maintain internal temperature !\n"); bt.write('o');}
  else { Serial.println("> The door is now closed !\n"); bt.write('f'); }
  if(menuActive == 2) refreshDisplay(2); //Update the device infos page if active
  digitalWrite(LED_BUILTIN, LOW);
}

//Stop the incubator now
void emergencyStop(){
  Serial.println("> Stopping all functions...");
  if(useReturnServo) myServo.detach();
  if(useSD) file.close();
  if(useFan) digitalWrite(fan, LOW);
  if(useTempHum) digitalWrite(thermistor, LOW);
  useFan = 0;
  useSD = 0;
  useReturnServo = 0;
  useDoorSensor = 0;
  useTempHum = 0;
  useBluetooth = 0;
  useIncubation = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("  NOW  STOPPED");
  lcd.setCursor(0,1);
  lcd.print("Reset to restart");
  Serial.println("> The incubator has been stopped");
}
