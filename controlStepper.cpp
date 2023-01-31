#include "controlStepper.h"

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
    preDegreeAz = 0;
    preDegreeEl = 0;
    rotateStepper(stepperAz, -curDegreeAz);
    rotateStepper(stepperEl, -curDegreeEl);
    return;
  }
  rotateStepper(stepperEl, targetDegreeEl);
  rotateStepper(stepperAz, targetDegreeAz); 
}
