#include "controlStepper.h"

hw_timer_t *timer0 = NULL;
hw_timer_t *timer1 = NULL;

void IRAM_ATTR stopPWM1() {
  ledcWrite(PWM1_CHANNEL, 0);
  timerDetachInterrupt(timer0);
  timerEnd(timer0);
}
void IRAM_ATTR stopPWM2() {
  ledcWrite(PWM2_CHANNEL, 0);
  timerDetachInterrupt(timer1);
  timerEnd(timer1);
}
void setupPWM(){
  ledcAttachPin(STEP_AZ, PWM1_CHANNEL);
  ledcAttachPin(STEP_EL, PWM2_CHANNEL);
  ledcSetup(PWM1_CHANNEL, PWM_FREQ, PWM_RES);  
  ledcSetup(PWM2_CHANNEL, PWM_FREQ, PWM_RES);
}
void rotateStepper(uint8_t typeMotor, double Angle){
  if(typeMotor){ // typeMotor == 1 => AZ stepper
    (Angle > 0) ? digitalWrite(DIR_AZ, HIGH) : digitalWrite(DIR_AZ, LOW);
    timer0 = timerBegin(0, 80, true);
    timerAttachInterrupt(timer0, &stopPWM1, true);
    timerAlarmWrite(timer0, (uint32_t)TIMER_INTERVAL_US*Angle/360, true);
    ledcWrite(PWM1_CHANNEL, 50);
    timerAlarmEnable(timer0);
  }else{ // typeMotor == 0 => EL stepper
    (Angle > 0) ? digitalWrite(DIR_EL, HIGH) : digitalWrite(DIR_EL, LOW);
    timer1 = timerBegin(1, 80, true);
    timerAttachInterrupt(timer1, &stopPWM2, true);
    timerAlarmWrite(timer1, (uint32_t)TIMER_INTERVAL_US*Angle/360, true);
    ledcWrite(PWM2_CHANNEL, 50);
    timerAlarmEnable(timer1);
  }
}
/*
void rotateStepper(AccelStepper& mystepper,double Angle)
{  
  int target = (STEP_ONE_TURN*(int)Angle/360);
  int directionRotate = (Angle < 0 ? -1 : 1);
  int speedRotate = directionRotate * SPEED_ROTATE;
  mystepper.setCurrentPosition(0); 
  while(mystepper.currentPosition() != target) 
  {
    mystepper.setSpeed(speedRotate); 
    mystepper.runSpeed();
  }
} 
void rotateInTrackingMode(AccelStepper& stepperAz, AccelStepper& stepperEl, Sgp4& satInfo, bool resetFlag)
{
  static double preDegreeAz = 0;
  static double preDegreeEl = 0;
  
  double curDegreeAz = satInfo.satAz;
  double curDegreeEl = satInfo.satEl;
  
  double targetDegreeAz = curDegreeAz - preDegreeAz;
  double targetDegreeEl = curDegreeEl - preDegreeEl;
  
  if(targetDegreeAz < -CLOCKWISE_THRESHOLD){
    targetDegreeAz = (360 - preDegreeAz + curDegreeAz);
  }
  else if(targetDegreeAz > CLOCKWISE_THRESHOLD){
    targetDegreeAz = -(360 - curDegreeAz + preDegreeAz);
  }
  if(!resetFlag){
    if(targetDegreeAz >= MIN_DEGREE_ROTATE || targetDegreeAz <= -MIN_DEGREE_ROTATE){
      preDegreeAz = curDegreeAz;
    }else{
      return;
    }
  }else{
    rotateStepper(stepperAz, -preDegreeAz);
    rotateStepper(stepperEl, -preDegreeEl);
    preDegreeAz = 0;
    preDegreeEl = 0;
    return;
  }
  rotateStepper(stepperEl, targetDegreeEl);
  rotateStepper(stepperAz, targetDegreeAz); 
}
bool rotateInBasicMode(AccelStepper& stepperAz, AccelStepper& stepperEl, Sgp4& satInfo, unsigned long t_now)
{
  unsigned long t_max = Predict(satInfo, t_now);
  if(t_max == 0){ // failed predict
    return false;
  }
  satInfo.findsat(t_max);
  
  double targetDegreeEl = satInfo.satEl;
  double targetDegreeAz = satInfo.satAz;
  
  rotateStepper(stepperEl, targetDegreeEl);
  rotateStepper(stepperAz, targetDegreeAz);
  return true;
}
*/
