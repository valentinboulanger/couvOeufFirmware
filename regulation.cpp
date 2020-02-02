/*This file contains the functions that control the temperature, humidity and the return functionality*/
#include <DHT.h>
#include <Servo.h>
#include <Interval.h>
#include "regulation.h"
#include "com.h"
#include "sdCard.h"

/*USER SETTINGS*/
bool useFan = 1;
bool useTempHum = 1;
bool useReturnServo = 1;
#define DHTPIN 3  //The pin where the temperature/humidity sensor is attached. Default wiring : data on D3
#define DHTTYPE DHT11 //The type of the temperature/humidity sensor
#define thermistor 4 //The pin where the thermistor relay is attached. Default wiring : relay on D4
#define finalTemperature 37.5 //The final temperature to reach
#define finalHumidity 100.0 //The final humidity to reach
#define thresholdTemperature 1.0 //Threshold for activating/deactivating the temperature
#define thresholdHumidity 5.0 //Threshold for activating/deactivating the temperature
#define fan 12 //The pin where the fan is attached. Default wiring : relay on D12
#define servo 11  //The pin where the servomotor is attached
#define angle 45 //Angle needed to return the eggs
Interval returnInterval(10000); //Delay between two reversals
Interval measureInterval(5000); //Delay between two measurements

/*PROGRAM SETTINGS*/
DHT dht(DHTPIN, DHTTYPE);
float saveTemperature = 0;
float saveHumidity = 0;
bool fanPhase = false;
Servo myServo;
bool returnPhase = false;

void initializeTempHum(){
  Serial.print("> Initializing temperature & humidity sensor..........");
  dht.begin();
  pinMode(thermistor, OUTPUT);
  digitalWrite(thermistor, HIGH);
  Serial.println("OK !\n");
}

void initializeFan(){
  Serial.print("> Initializing the fan..........");
  pinMode(fan, OUTPUT);
  digitalWrite(fan, HIGH);
  Serial.println("OK !\n");
}

void initializeServo(){
  Serial.print("> Initializing the servomotor..........");
  myServo.attach(servo);
  myServo.write(0);
  myServo.detach();
  Serial.println("OK !\n");
}

//Control temperature in the incubator
void controlTemperature(){
  //Control the internal temperature & humidity
  if(measureInterval.isElapsed()){
    digitalWrite(LED_BUILTIN, HIGH);
    saveTemperature = dht.readTemperature();
    saveHumidity = dht.readHumidity();
    //If the returned values are not numbers, there was an error
    if(isnan(saveTemperature) || isnan(saveHumidity)) {
      Serial.println("> An error has occured while measuring temperature and humidity !\n");
      digitalWrite(thermistor, HIGH);
      if(useFan) digitalWrite(fan, HIGH);
    }
    else {
      //Else, values are saved
      if(menuActive == 2) refreshDisplay(2); //Update the device infos page if active
      //Check if we need to activate the heating
      if(eggPresent){
        if (saveTemperature > finalTemperature + thresholdTemperature)  {
        //We deactivate the heating and the fan
        Serial.println("> Temperature too high (>" + String(finalTemperature) + ") ! Deactivate the heating...\n");
        digitalWrite(thermistor, HIGH);
        if(useFan) digitalWrite(fan, HIGH);
        }
        if (saveTemperature < finalTemperature - thresholdTemperature)  {
         //We activate the heating and the fan
         Serial.println("> Temperature too low (<" + String(finalTemperature) + ") ! Activate the heating...\n");
         digitalWrite(thermistor, LOW);
         if(useFan) digitalWrite(fan, LOW);
       }
      }
    }
  }
  digitalWrite(LED_BUILTIN, LOW);
}

//Return the eggs with the servomotor
void eggsReturn(){
  //Return eggs with the servomotor
  if(returnInterval.isElapsed()){
    digitalWrite(LED_BUILTIN, HIGH);
    //Retournement phase 1
    if(returnPhase){
     myServo.attach(servo);
     myServo.write(angle); 
     myServo.detach();
     returnPhase = false;
    }
    else{
      myServo.attach(servo);
      myServo.write(0);
      myServo.detach();
      returnPhase = true;
    }
  }
  digitalWrite(LED_BUILTIN, LOW);
}
