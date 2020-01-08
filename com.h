#include <LiquidCrystal_I2C.h>
extern bool useDoorSensor;
extern bool useBluetooth;
extern unsigned int menuActive;
extern LiquidCrystal_I2C lcd;

void initializeIR();
void initializeLCD();
void initializeBT();
void initializeDoor();
void interceptCommands();
void refreshDisplay(int menu);
void refreshMenuIncubation();
void checkDoor();
void updateApplication();
void doorStatus(bool stat);
void emergencyStop();
