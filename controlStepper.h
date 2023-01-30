#include <Arduino.h> 
#include <AccelStepper.h>
#include "predictSat.h"

#define STEP_ONE_TURN               200
#define SPEED_ROTATE                200
#define STEP_AZ                     27
#define DIR_AZ                      13
#define STEP_EL                     32
#define DIR_EL                      33
#define MOTOR_INTERFACE_TYPE        1

void backRotateStepper(AccelStepper& mystepper, double Angle);
void rotateStepper(AccelStepper& mystepper,double Angle);
void rotateInTrackingMode(AccelStepper& stepperAz, AccelStepper& stepperEl, Sgp4& satInfo);
