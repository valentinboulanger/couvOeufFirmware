/*
# # # # # COUV'OEUF    PROJECT # # # # #
# # # V.DELOBEL  &  V.BOULANGER # # #
v.2.4.0 # # # # # # # # # # # # # # # # # # # #
*/
#include "com.h"
#include "regulation.h"
#include "sdCard.h"

//Variables of the program - Change them at your own risk !
#define INFO "# # # # # COUV'OEUF Project # # # #\n#     V.DELOBEL & V.BOULANGER     #\nv2.4.0  # # # # # # # # # # # # # #\n\n"

void setup() {
  //Initialisation
  Serial.begin(9600);
  Serial.println(INFO);
  Serial.println("> START-UP VERIFICATION PROCESS\n");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  //Temperature & Humidity
  if(useTempHum) initializeTempHum();

  //Ventilation
  if(useFan) initializeFan();

  //Servomotor
  if(useReturnServo) initializeServo();

  //IR communication and LCD
  initializeIR();
  initializeLCD();
  
  //Bluetooth module
  if(useBluetooth) initializeBT();

  //Limit sensor
  if(useDoorSensor) initializeDoor();

  //SD Module
  if(useSD) initializeSD();
}

void loop() {
  interceptCommands();
  if(useDoorSensor) checkDoor();
  if(useTempHum) controlTemperature();
  if(eggPresent){
    if(useReturnServo) eggsReturn();
    if(useIncubation) controlIncubation();
    if(useSD) backup();
    if(useBluetooth) updateApplication();
  }
}
