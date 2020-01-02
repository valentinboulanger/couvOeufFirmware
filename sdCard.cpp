/*This file contains the functions that control incubation time and the backup on the SD card*/
#include <SD.h>
#include <SPI.h>
#include <Interval.h>
#include "sdCard.h"

/*USER SETTINGS*/
bool useSD = 1;
bool useIncubation = 1;
#define SD_PIN 49 //The pin where the SD Module is attached. Default wiring : CS on D4 - SCK on D52 - MOSI on D51 - MISO on D50
Interval incubationInterval(1000); //Delay between two updates of incubation time
Interval backupInterval(3000); //Delay between each backup of the incubation time to the SD Card - Must be less than 32400

/*PROGRAM SETTINGS*/
String path = "file.txt";
int inc[10];
long temp[10];
bool eggPresent = false;
File file;

void initializeSD(){
 Serial.print("> Initializing the SD Module.........."); 
  for(int i = 0; i <= 9; i++){
    inc[i] = -1;
  }
  if(!SD.begin(SD_PIN)) {
    Serial.println("ERROR !\n");
    return;
  }
  Serial.println("OK !\n");
  file = SD.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("> Data file not found. Creation of a new data file...\n"); 
    createFile();
  }
  file.close();
  fetchNullLocations();
  for(int i = 0; i <= 9; i++){
    if(inc[i] != -1) eggPresent = true;
  }
}

//Increment the incubation time
void controlIncubation(){
  //Increment the incubation time every incubation interval (if the current egg is present)
  if(incubationInterval.isElapsed()){
    digitalWrite(LED_BUILTIN, HIGH);
    for(int i = 0; i <= 9; i++){
      if(inc[i] != -1) inc[i] += (incubationInterval.value()/1000);
    }
  }
  digitalWrite(LED_BUILTIN, LOW);
}

//Debugging function - Display the data saved in the cache and in the SD Card 
void displayData() {
  Serial.println("DATA IN CACHE");
  for(int i = 0; i <= 9; i++){
    Serial.println("Egg " + String(i+1) + " : " + inc[i]);
  }
  Serial.println("DATA SAVED");
  String buffer;
  int counter = 0;
  file = SD.open(path, FILE_READ);
  while (file.available()) {
    buffer = file.readStringUntil('\n');
    Serial.println("Egg " + String(counter+1) + " : " + String(buffer));
    counter++;
  }
  Serial.println("");
  file.close();
}

//Generate the data file
void createFile(){
  file = SD.open(path, FILE_WRITE);
  for(int i = 0; i <= 9; i++){
    file.println("-1");
  }
  file.close();
}

//Add an egg at a location
void addEgg(int number){
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("> Adding an egg at the location " + String(number) + "\n");
  inc[number-1] = 0;
  backup();
  digitalWrite(LED_BUILTIN, LOW);
}

//Delete an egg at a location
void deleteEgg(int number){
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("> Deleting an egg at the location " + String(number) + "\n");
  inc[number-1] = -1;
  backup();
  digitalWrite(LED_BUILTIN, LOW);
}

//Check empty eggs locations
void fetchNullLocations(){
  int counter = 0;
  file = SD.open(path, FILE_READ);
  while (file.available()) {
    String tmp = file.readStringUntil('\n');
    if(tmp.toInt() == -1) inc[counter] = -1;
    else inc[counter] = 0;
    counter++;
  }
  file.close();
}

//Backup incubation time to the SD Card
void backup(){
  if(backupInterval.isElapsed()){
    fetchData();
    saveData();
  }
}

//Fetch data saved in the SD Card
void fetchData(){
  //Import data in a buffer array
  digitalWrite(LED_BUILTIN, HIGH);
  String buffer;
  file = SD.open(path, FILE_READ);
  int counter = 0;
  while (file.available()) {
    buffer = file.readStringUntil('\n');
    temp[counter] = buffer.toInt(); 
    counter++;
  }
  file.close();
  //Update the incubation time
  eggPresent = false;
  for(int i = 0; i <= 9; i++){
    if(inc[i] == -1 || inc[i] == 0 || temp[i] == -1) temp[i] = long(inc[i]);
    else{ temp[i] += long(inc[i]); inc[i] = 0; }
    //Verify if there is at least an egg
    if(temp[i] != -1) eggPresent = true;
  }
  digitalWrite(LED_BUILTIN, LOW);
}

//Save data in the cache to the SD Card
void saveData(){
  //Save data to the SD card
  digitalWrite(LED_BUILTIN, HIGH);
  SD.remove(path);
  file = SD.open(path, FILE_WRITE);
  for(int i = 0; i <= 9; i++){
    file.println(temp[i]);
  }
  file.close();
  Serial.println("> Automatic backup performed\n");
  digitalWrite(LED_BUILTIN, LOW);
}
