//#define POT_TEST
/**************************************************************************

Potentiometer Module
 
**************************************************************************/

/*---------------------------- Include Files ----------------------------*/
#include <stdio.h>
#include <me218_c32.h>
#include "ADS12.h"
#include "pot.h"
/*--------------------------- Module Defines ----------------------------*/
#define MIN_LEVER_RESET 30

/*---------------------------- Module Types -----------------------------*/
/*-------------------------- Module Variables ---------------------------*/
/*-------------------------- Module Functions ---------------------------*/
/*----------------------------- Module Code -----------------------------*/
unsigned short ReturnMoney(LEVER_t side){

	unsigned short currentMoneyLevel, currentPotentiometerVoltage;
	
	if (side == LEFT_LEVER){
	
		// Read voltage input from left potentiometer (Port A0)
		currentPotentiometerVoltage = ADS12_ReadADPin(0);
		// Convert voltage to money level from experimental fit of voltage range
		currentMoneyLevel = (40*(currentPotentiometerVoltage - 350))/37;
		
	}
	if (side == RIGHT_LEVER){
		
		// Read voltage input from right potentiometer (Port A1)
		currentPotentiometerVoltage = ADS12_ReadADPin(1);
		// Convert voltage to money level from experimental fit of voltage range
		currentMoneyLevel = (40*(722 - currentPotentiometerVoltage))/52;
	} 
	
	return currentMoneyLevel;
}

unsigned char CheckLeverReset(LEVER_t side) {
  
  unsigned char leverReset = 0;
  unsigned int currMoney;  
  
	if (side == LEFT_LEVER){
	
		currMoney = ReturnMoney(LEFT_LEVER);
		if (currMoney < MIN_LEVER_RESET)
		  leverReset = 1;
	}
	if (side == RIGHT_LEVER){
  	currMoney = ReturnMoney(RIGHT_LEVER);
  	if (currMoney < MIN_LEVER_RESET)
		  leverReset = 1;
	}
 
 return leverReset; 
} 


/*---------------------------- Test Harness -----------------------------*/

#ifdef POT_TEST
void main(void)
{

	//Set ports to analog input
	ADS12_Init("OOOOOOAA");
	
	while(1){
	  
	    //printf("%u %u \r\n", ADS12_ReadADPin(0), ADS12_ReadADPin(1));
	    

	    if(CheckLeverReset(LEFT_LEVER) == 1)
	    printf("Left reset\r\n");
	    if(CheckLeverReset(RIGHT_LEVER) == 1)
	    printf("Right reset\r\n");

    	//Test potentiometer	
    	printf("Left: $%d, Right: $%d\r\n",ReturnMoney(LEFT_LEVER), ReturnMoney(RIGHT_LEVER));
    
    	/*
    	printf("Set the leftpotentiometer to upper limit and press any key\r\n");
    	getchar();
    	Money = ReturnMoney(LEFT_LEVER);
    	printf("You have selected $%d",Money);
    	
    	printf("Set the leftpotentiometer to center of range and press any key\r\n");
    	getchar();
    	Money = ReturnMoney(LEFT_LEVER);
    	printf("You have selected $%d",Money);
    	
    	//Test right potentiometer	
    	printf("Set the right potentiometer to lower limit and press any key\r\n");
    	getchar();
    	Money = ReturnMoney(RIGHT_LEVER);
    	printf("You have selected $%d",Money);
    	
    	printf("Set the right potentiometer to upper limit and press any key\r\n");
    	getchar();
    	Money = ReturnMoney(RIGHT_LEVER);
    	printf("You have selected $%d",Money);
    	
    	printf("Set the right potentiometer to center of range and press any key\r\n");
    	getchar();
    	Money = ReturnMoney(RIGHT_LEVER);
    	printf("You have selected $%d\r\n",Money); */
	}
}
#endif /*POT_TEST*/

/*----------------------------- End of File -----------------------------*/