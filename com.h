extern bool useDoorSensor;
extern bool useBluetooth;

void initializeIR();
void initializeBT();
void initializeDoor();
void interceptCommands();
void checkDoor();
void updateApplication();
void doorStatus(bool stat);
void resetModes();
void emergencyStop();
