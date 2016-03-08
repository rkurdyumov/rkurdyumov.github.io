#ifndef BALLOT_SENSOR_H
#define BALLOT_SENSOR_H

/**************************************************************************

LED Module
 
**************************************************************************/


/*-------------------------- Public Variables ---------------------------*/
typedef enum{
  NO_EVENT,
  BALLOT_SENSED
}BALLOT_EVENT_t;

/*-------------------------- Public Functions ---------------------------*/
BALLOT_EVENT_t CheckBallotState(void);

#endif // BALLOT_SENSOR_H