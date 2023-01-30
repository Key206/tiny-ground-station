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
void rotateInTrackingMode(AccelStepper& stepperAz, AccelStepper& stepperEl, Sgp4& satInfo, bool resetPre)
{
  static double preDegree = 0;
  double curDegree = satInfo.satAz;
  double targetDegree = curDegree - preDegree;
  if(targetDegree < -300){
    targetDegree = (360 - preDegree + curDegree);
  }
  else if(targetDegree > 300){
    targetDegree = -(360 - curDegree + preDegree);
  }
  rotateStepper(stepperAz, targetDegree); 
  preDegree = (resetPre == false) ? curDegree : 0;
}
