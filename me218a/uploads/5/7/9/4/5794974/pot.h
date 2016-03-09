#ifndef POT_H
#define POT_H

/**************************************************************************

Potentiometer Module
 
**************************************************************************/


/*-------------------------- Public Variables ---------------------------*/
typedef enum {	
      LEFT_LEVER,
			RIGHT_LEVER
}LEVER_t;
/*-------------------------- Public Functions ---------------------------*/
unsigned short ReturnMoney(LEVER_t side);
unsigned char CheckLeverReset(LEVER_t side);


#endif // POT_H
