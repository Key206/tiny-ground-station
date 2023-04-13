//#include <AccelStepper.h>
#include <esp32-hal-timer.h>
#include "predictSat.h"

#define STEP_ONE_TURN               200
#define SPEED_ROTATE                400
#define STEP_AZ                     32
#define DIR_AZ                      33
#define STEP_EL                     27
#define DIR_EL                      13
#define MOTOR_INTERFACE_TYPE        1
#define MIN_DEGREE_ROTATE           1.8
#define CLOCKWISE_THRESHOLD         300
#define PWM1_CHANNEL                0
#define PWM1_RES                    10
#define PWM1_FREQ                   200
#define TIMER_INTERVAL_US           1000000

void setupPWM();
void IRAM_ATTR stopPWM();
void rotateStepper(double Angle);
/*
void backRotateStepper(AccelStepper& mystepper, double Angle);
void rotateStepper(AccelStepper& mystepper,double Angle);
void rotateInTrackingMode(AccelStepper& stepperAz, AccelStepper& stepperEl, Sgp4& satInfo, bool resetFlag);
bool rotateInBasicMode(AccelStepper& stepperAz, AccelStepper& stepperEl, Sgp4& satInfo, unsigned long t_now);
*/
