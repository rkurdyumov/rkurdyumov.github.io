#ifndef CARD_SENSOR_H
#define CARD_SENSOR_H

/**************************************************************************

Card Sensor Module
 
**************************************************************************/


/*-------------------------- Public Variables ---------------------------*/
typedef enum {	
				RED_CARD,
				GREEN_CARD,
				BLUE_CARD,
				NO_CARD
}CARD_STATE_t;


/*-------------------------- Public Functions ---------------------------*/
void InitializeCardSensor(void);
CARD_STATE_t CheckLeftCardState(void);
CARD_STATE_t CheckRightCardState(void);
void SetCardCalibration(unsigned int sideCard);

#endif // CARD_SENSOR_H