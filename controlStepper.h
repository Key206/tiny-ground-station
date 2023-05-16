#include "predictSat.h"

#define STEP_ONE_TURN               200
#define SPEED_ROTATE                400
#define STEP_AZ                     33
#define DIR_AZ                      26
#define STEP_EL                     32
#define DIR_EL                      25
#define STEPPER_AZ                  1
#define STEPPER_EL                  0
#define MOTOR_INTERFACE_TYPE        1
#define MIN_DEGREE_ROTATE           1.8
#define CLOCKWISE_THRESHOLD         300
#define PWM1_CHANNEL                0
#define PWM2_CHANNEL                1
#define PWM_RES                     10
#define PWM_FREQ                    200
#define TIMER_INTERVAL_US           1000000

void setupPWM();
void IRAM_ATTR stopPWM1();
void IRAM_ATTR stopPWM2();
void rotateStepper(uint8_t typeMotor, double Angle);
void rotateInTrackingMode(Sgp4& satInfo, bool resetFlag);
bool rotateInStandardMode(Sgp4& satInfo, unsigned long t_max);
