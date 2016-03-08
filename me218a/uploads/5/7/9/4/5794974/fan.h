#ifndef FAN_H
#define FAN_H

/**************************************************************************

Fan Module
 
**************************************************************************/


/*-------------------------- Public Variables ---------------------------*/
typedef enum {
  LEFT_FAN,
  RIGHT_FAN,
  BOTH_FANS
}FAN_t;
/*-------------------------- Public Functions ---------------------------*/
void StartFan(FAN_t fan);
void StopFan(FAN_t fan);

#endif // FAN_H