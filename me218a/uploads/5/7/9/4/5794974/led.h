#ifndef LED_H
#define LED_H

/**************************************************************************

LED Module
 
**************************************************************************/


/*-------------------------- Public Variables ---------------------------*/
typedef enum{
  RED,
  GREEN,
  BLUE,
  NONE
}LED_Color_t;

/*-------------------------- Public Functions ---------------------------*/
void InitializeLED(void);
void CheckLEDEvents(void);
void UpdatePattern(LED_Color_t Standings[4][4]); //[region][standing]
void UpdateWinnerPattern(LED_Color_t Winner);
void DisplayOnePlayerColor(LED_Color_t player);
void DisplayTwoPlayerColor(LED_Color_t left, LED_Color_t right);
void TurnOffLEDs(void);


#endif // LED_H