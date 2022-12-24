#include "controlStepper.h"


void backRotateStepper(AccelStepper& mystepper, double Angle)
{ 
  int target = (STEP_ONE_TURN*(int)Angle/360);
  mystepper.setCurrentPosition(0); 
  while(mystepper.currentPosition() != target) 
  {
    mystepper.setSpeed(SPEED_ROTATE); 
    mystepper.runSpeed();   
  }
}
void rotateStepper(AccelStepper& mystepper,double Angle)
{  
  int target = (-STEP_ONE_TURN*(int)Angle/360);
  mystepper.setCurrentPosition(0); 
  while(mystepper.currentPosition() != target) 
  {
    mystepper.setSpeed(-SPEED_ROTATE); 
    mystepper.runSpeed();
  }
} 
