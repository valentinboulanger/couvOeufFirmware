/*This file contains the functions that control the bluetooth module, the IR receiver and the door sensor*/
#include <IRremote.h>
#include <SoftwareSerial.h>
#include <Interval.h>
#include "com.h"
#include "regulation.h"
#include "sdCard.h"

/*USER SETTINGS*/
bool useBluetooth = 1;
bool useDoorSensor = 1;
#define RX_BLUETOOTH 11 //The pin where the RX pin of the bluetooth module is attached. Default wiring : RX on D11
#define TX_BLUETOOTH 10 //The pin where the TX pin of the bluetooth module is attached. Default wiring : TX on D10
#define modeLed 12 //The LED which indicates that the mode key is active
#define doorSensor 7 //The pin where the sensor limit of the door is attached. Default wiring : status on D7
#define receiver 5 //The pin where the IR receiver is attached. Default wiring : data on D5
Interval resetModesInterval(3000); //Delay to press a target egg on the remote
Interval updateInterval(5000); //Delay between each refresh on the Couv'Oeuf application

/*PROGRAM SETTINGS*/
SoftwareSerial bt (TX_BLUETOOTH, RX_BLUETOOTH);
IRrecv ir(receiver);
decode_results resultsIr;
bool additionMode = 0;
bool deletionMode = 0;
bool door = false;

void initializeIR(){
  //Initialize the IR receiver
  Serial.print("> Initializing the infrared communication..........");
  ir.enableIRIn();
  ir.blink13(true);
  Serial.println("OK !\n");
  //Initialize the mode LED
  pinMode(modeLed, OUTPUT);
  digitalWrite(modeLed, LOW);
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
    if(result == 0xFF629D) emergencyStop();
    else if(result == 0xFF22DD) Serial.println("> Display page 1\n");
    else if(result == 0xFF02FD) Serial.println("> Display page 2\n");
    else if(result == 0xFFC23D) Serial.println("> Display page 3\n");
    else if(result == 0xFFA857) backup();
    else if(result == 0xFF4AB5 && additionMode == true) { addEgg(1); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF4AB5 && deletionMode == true) { deleteEgg(1); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF6897 && additionMode == true) { addEgg(2); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF6897 && deletionMode == true) { deleteEgg(2); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF9867 && additionMode == true) { addEgg(3); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF9867 && deletionMode == true) { deleteEgg(3); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFFB04F && additionMode == true) { addEgg(4); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFFB04F && deletionMode == true) { deleteEgg(4); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF30CF && additionMode == true) { addEgg(5); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF30CF && deletionMode == true) { deleteEgg(5); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF18E7 && additionMode == true) { addEgg(6); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF18E7 && deletionMode == true) { deleteEgg(6); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF7A85 && additionMode == true) { addEgg(7); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF7A85 && deletionMode == true) { deleteEgg(7); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF10EF && additionMode == true) { addEgg(8); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF10EF && deletionMode == true) { deleteEgg(8); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF38CF && additionMode == true) { addEgg(9); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF38CF && deletionMode == true) { deleteEgg(9); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF5AA5 && additionMode == true) { addEgg(10); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF5AA5 && deletionMode == true) { deleteEgg(10); additionMode = false; deletionMode = false; digitalWrite(modeLed, LOW); }
    else if(result == 0xFF42BD) { deletionMode = false; additionMode = true; digitalWrite(modeLed, HIGH); }
    else if(result == 0xFF52AD) { additionMode = false; deletionMode = true; digitalWrite(modeLed, HIGH); }
    else if(result == 0xFF45FA) displayData();
    ir.resume();
  }
  digitalWrite(LED_BUILTIN, LOW);
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
  if(stat) { door = true; Serial.println("> WARNING : The door is now open ! Close the door to maintain internal temperature !\n"); bt.write('o'); }
  else { door = false; Serial.println("> The door is now closed !\n"); bt.write('f'); }
  digitalWrite(LED_BUILTIN, LOW);
}

//Reset the add and remove key on the remote
void resetModes(){
  if(resetModesInterval.isElapsed()){
    digitalWrite(LED_BUILTIN, HIGH);
    additionMode = false;
    deletionMode = false;    
    digitalWrite(modeLed, LOW);
  }
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
  Serial.println("> The incubator has been stopped");
}
