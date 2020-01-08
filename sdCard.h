#include <SD.h>
#include <SPI.h>
#include "Arduino.h"

#define SD_PIN 49

extern bool useSD;
extern int inc[10];
extern long temp[10];
extern bool eggPresent;
extern bool useIncubation;
extern File file;

void initializeSD();
void controlIncubation();
void displayData();
void createFile();
void addEgg(int number);
void deleteEgg(int number);
void fetchNullLocations();
void backup();
void fetchData();
void saveData();
