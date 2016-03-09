#ifndef REGION_BUTTONS_H
#define REGION_BUTTONS_H

/**************************************************************************

Region Buttons Module

Returns unsigned char:
0   No Event
1   1A
2   1B
3   2A
4   2B
5   3A
6   3B
7   4A
8   4B
 
**************************************************************************/


/*-------------------------- Public Variables ---------------------------*/
/*-------------------------- Public Functions ---------------------------*/
void InitializeButtons(void);
unsigned char CheckButtonPress(void);

#endif // REGION_BUTTONS_H