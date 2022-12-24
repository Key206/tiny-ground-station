#include <Arduino.h> 
#include <AccelStepper.h>

#define STEP_ONE_TURN               200
#define SPEED_ROTATE                100
#define STEP_AZ                     27
#define DIR_AZ                      13
#define STEP_EL                     32
#define DIR_EL                      33
//#define STEP_MEASURE                26
//#define DIR_MEASURE                 25
#define MOTOR_INTERFACE_TYPE        1

void backRotateStepper(AccelStepper& mystepper, double Angle);
void rotateStepper(AccelStepper& mystepper,double Angle);
