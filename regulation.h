#include <Servo.h>

#define thermistor 4
#define fan 12

extern bool useFan;
extern bool useTempHum;
extern bool useReturnServo;
extern float saveTemperature;
extern float saveHumidity;
extern Servo myServo;

void initializeTempHum();
void initializeFan();
void initializeServo();
void controlTemperature();
void eggsReturn();
