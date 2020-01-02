/*This file contains the functions that control the temperature, humidity and the return functionality*/
#include <DHT.h>
#include <Servo.h>
#include <Interval.h>
#include "regulation.h"

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
Interval returnInterval(3000); //Delay between two reversals
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
  Serial.println("OK !\n");
}

void initializeFan(){
  Serial.print("> Initializing the fan..........");
  pinMode(fan, OUTPUT);
  Serial.println("OK !\n");
}

void initializeServo(){
  Serial.print("> Initializing the servomotor..........");
  myServo.attach(servo);
  myServo.write(0);
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
      digitalWrite(thermistor, LOW);}
    else {
      //Else, values are saved
      //Check if we need to activate the heating
      if (saveTemperature > finalTemperature + thresholdTemperature)  {
        //We deactivate the heating
        Serial.println("> Temperature too high (>" + String(finalTemperature) + ") ! Deactivate the heating...\n");
        digitalWrite(thermistor, LOW);
      }
      if (saveTemperature < finalTemperature - thresholdTemperature)  {
       //We activate the heating
       Serial.println("> Temperature too low (<" + String(finalTemperature) + ") ! Activate the heating...\n");
       digitalWrite(thermistor, HIGH);
     }
     //Check if we need to activate the fan
     if(useFan){
        if (saveHumidity > finalHumidity - thresholdHumidity)  {
         //We activate the fan
         Serial.println("> Humidity too high (>" + String(finalHumidity) + ") ! Activate the fan...\n");
         digitalWrite(fan, HIGH);
       }
       else{
         //We deactivate the fan
         Serial.println("> Humidity correct (<" + String(finalHumidity) + ") ! Deactivate the fan...\n");
         digitalWrite(fan, LOW);
       }
     }
    }
  }
  digitalWrite(LED_BUILTIN, LOW);
}

//Return the eggs with the servomotor
void eggsReturn(){
  //Return eggs with the servomotor
  myServo.attach(servo);
  if(returnInterval.isElapsed()){
    digitalWrite(LED_BUILTIN, HIGH);
    //Retournement phase 1
    if(returnPhase){
     myServo.write(angle); 
     returnPhase = false;
    }
    else{
      myServo.write(0);
      returnPhase = true;
    }
    myServo.detach();
  }
  digitalWrite(LED_BUILTIN, LOW);
}
