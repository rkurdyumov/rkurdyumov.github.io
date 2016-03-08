#ifndef VIBRATE_MOTOR_H
#define VIBRATE_MOTOR_H

/**************************************************************************

Vibrate Motor Module
 
**************************************************************************/


/*-------------------------- Public Variables ---------------------------*/
typedef enum {
  LEFT_MOTOR,
  RIGHT_MOTOR,
  BOTH_MOTORS
}VIBRATE_MOTOR_t;
/*-------------------------- Public Functions ---------------------------*/
void VibrateMotor(VIBRATE_MOTOR_t motor);
void StopVibrateMotor(VIBRATE_MOTOR_t motor);



#endif // VIBRATE_MOTOR_H