#define MAIN_TEST
//#define	CONTROL_ON

/*
Main PIC Module

This module:
(1) Receives RFID card swipes from the Security PIC
(2) Handles all the boat I/O
(3) Sends/receives XBee messages via UART
(4) Runs the gameplay, transmit, and receive state machines

*/

/*   PINOUT FOR MAIN PIC
******************************************
1   MLCR       |MCLR  B7|  PGD	        40
2   Analog In  |A0    B6|  PGC          39
3   OSC2       |A1    B5|  Servo        38
4              |A2    B4|               37
5              |A3    B3|  			    36
6              |A4    B2|               35
7   SS		   |A5    B1|               34
8  			   |E0    B0|  		        33
9			   |E1   VDD|  +5V          32
10 	TX         |E2   VSS|  GND  		31
11  +5V        |VDD   D7|  		        30
12  GND        |VSS   D6|               29
13  OSC1       |OSC1  D5|  DIR-R        28
14  OSC2       |OSC2  D4|  DIR-L        27
15             |C0    C7|  XBEE TX		26
16  PWM-R      |C1    C6|  XBEE RX      25
17  PWM-L	   |C2    C5|               24
18  SPI-SCK	   |C3    C4|  SPI-SDI      23
19  DEBUG	   |D0    D3|  DEBUG        22
20 	DEBUG      |D1    D2|  DEBUG	    21
*/

//#include <pic16f777.h>
#include <stdio.h>
#include <htc.h>
#define _LEGACY_HEADERS
#include "BITDEFS.h"
#include "TimerPIC.h"
#include "SCIMessage.h"
#include "Communication.h"
#include "Messages.h"
#include "MainIO.h"
#include "TimerNumber.h"

#define		TURN_THRESHOLD	5
#define		TURN_OFFSET		5

#ifdef SECURITY_TEST
__CONFIG(UNPROTECT & WDTDIS & PWRTEN & HS);
//__CONFIG(CP_OFF & WDTE_OFF & PWRTE_ON & FOSC_HS );
#endif

void InitSPI(void);
void SPIReceive(void);
void GetReply(void);
void HandleReceivedMessage(void);
unsigned char CalculateLeftSpeed(unsigned char Speed, unsigned char Direction, unsigned char Heading);
unsigned char CalculateRightSpeed(unsigned char Speed, unsigned char Direction, unsigned char Heading);

typedef enum {
	GAME_IDLE,
	LISTEN_FOR_TEAMMATE,
	WAITING_FOR_ACK,	
	BROADCAST_TEAM_MESSAGE,	
	GAME_ON
} GameState_t;

// Speed messages from bike
static unsigned char RefDirection = 1;
static unsigned char RefHeading = 90;
static unsigned char Speed;

// Closed loop speed control
static signed int RefSpeed = -45;
static unsigned char InputDirection;

// SPI Comm
static unsigned char SSPArray[6];
static unsigned char counter;

// UART XBee Comm
static unsigned char ReceivedData[10];
static unsigned char DataToSend[10];
static unsigned char DataLength;

// Gameplay state machine
static Swipe_t RFIDScanned = NO_SWIPE;
static unsigned char ColorSwipeFlag = 0;
static Message_t CurrentMessage = NoMessage;
static unsigned char TeamColor = NO_COLOR;
static GameState_t GameState = GAME_IDLE;

#ifdef MAIN_TEST
// Rising edge

void interrupt Security_ISR(void) {
  // If interrupt was due to the timer subsystem
	if (TMR0IE && TMR0IF)
    // 
		TimerPIC_ISR();
  // If interrupt was due to the SPI subsystem
	if (SSPIE && SSPIF) {
    //  Call SPIReceive to process the message
		SPIReceive();
    // Turn off the SPI interrupt flag
		SSPIF = 0;
	}	
  // If interrupt was due to CCP3
	if(CCP3IF)
    // Call the OutputCompareISR function
		OutputCompareISR();	
}
#endif


void InitSPI(void) {

	// Configure SPI port 
	// 0xxxxxxx = No write collision detect
	WCOL = 0;
	// x0xxxxxx = No receive overflow indicator
	SSPOV = 0;
	// xx1xxxxx = Enable serial port, SCK, SDO, SDI as serial pins
	SSPEN = 1;
	// xxx1xxxx = Clock idles high
	CKP = 1;
	// xxxxx100 = SPI Slave, SS pin control enabled, Use Timer2/2
	SSPM3 = 1;
	SSPM2 = 1;
	SSPM1 = 0;
	SSPM0 = 0;
	
	/* SPI status register
	0xxxxxxx = Data sampled in middle of output
	x0xxxxxx = Data transmitted on falling edge of SCK
	xxXXXXXX = Read only
	*/
	
	SSPSTAT = 0;
	SSPIE = 1;		// Turn On SPI interrupt
  	SSPBUF = 0;		// Clear buffer
}

// Get RFID scan info
void SPIReceive(void){
		
    // If SPI buffer is full
	  if (BF){
      // Store the buffer value into SSPArray at the appropriate index
      SSPArray[counter] = SSPBUF;
      // Increment the index by one
      counter++;
	}

  // If the index is greater than 6, we have received all 6 bytes
	if (counter >= 6) {

    // If the Swiped Card is not one of the Team Color Cards
		if (SSPArray[0] == 0x9E){
      // Set the ColorSwipeFlag to Zero
      ColorSwipeFlag = 0;
		}
		
		// Check the 3rd byte of SSPArray, and see what it matches
		if (SSPArray[2] == 0x47)
			// Atoll #1, Card 1: Set RFIDScanned to ATOLL1_SWIPE
			RFIDScanned = ATOLL1_SWIPE;
		else if (SSPArray[2] == 0x66)
			// Atoll #1, Card 2: Set RFIDScanned to ATOLL1_SWIPE
			RFIDScanned = ATOLL1_SWIPE;
		else if (SSPArray[2] == 0xC8)
			// Atoll #2, Card 1: Set RFIDScanned to ATOLL2_SWIPE
			RFIDScanned = ATOLL2_SWIPE;
		else if (SSPArray[2] == 0xE0)
			// Atoll #2, Card 2: Set RFIDScanned to ATOLL2_SWIPE
			RFIDScanned = ATOLL2_SWIPE;
		else if (SSPArray[2] == 0x36)
			// Atoll #3, Card 1: Set RFIDScanned to ATOLL3_SWIPE
			RFIDScanned = ATOLL3_SWIPE;
		else if (SSPArray[2] == 0x31)
			// Atoll #3, Card 2: Set RFIDScanned to ATOLL3_SWIPE
			RFIDScanned = ATOLL3_SWIPE;
		else if (SSPArray[2] == 0xF5)
			// Atoll #4, Card 1: Set RFIDScanned to ATOLL4_SWIPE
			RFIDScanned = ATOLL4_SWIPE;	
		else if (SSPArray[2] == 0xF0)
			// Atoll #4, Card 2: Set RFIDScanned to ATOLL4_SWIPE
			RFIDScanned = ATOLL4_SWIPE;
		else if (SSPArray[2] == 0x5E)
			// Atoll #5, Card 1: Set RFIDScanned to ATOLL5_SWIPE
			RFIDScanned = ATOLL5_SWIPE;
		else if (SSPArray[2] == 0x6A)
			// Atoll #5, Card 2: Set RFIDScanned to ATOLL5_SWIPE
			RFIDScanned = ATOLL5_SWIPE;
    // If State is GAME_IDLE or GAME_ON and Red Tag was Swiped
		else if (SSPArray[2] == 0xEC && (GameState == GAME_IDLE || GameState == GAME_ON)){
			// Red Team Tag
			RFIDScanned = RED_SWIPE;
			// Set TeamColor to RED and ColorSwipeFlag to 1
      TeamColor = RED;
			ColorSwipeFlag = 1;
      // Turn on the RED debug LED
			RD2 = 1;
		}
    // If State is GAME_IDLE or GAME_ON and Green Tag was Swiped
		else if (SSPArray[2] == 0x5A && (GameState == GAME_IDLE || GameState == GAME_ON)){
			// Green Team Tag
			RFIDScanned = GREEN_SWIPE;
      // Set TeamColor to GREEN and ColorSwipeFlag to 1
			TeamColor = GREEN;
			ColorSwipeFlag = 1;
      // Turn on the Green Debug LED
			RD3 = 1;
		}
    
    // If White Card was scanned
		else if (SSPArray[2] == 0x06){
			// "White" Team Tag
			RFIDScanned = WHITE_SWIPE;
      // Set team Color to No Color
			TeamColor = NO_COLOR;
      // Turn off all Team Color LEDs
			RD2 = 0;
			RD3 = 0;
      // Set ColorSwipeFlag to 1
			ColorSwipeFlag = 1;
      // Reset Servos to the NO_FLAG position
			SetServo(NO_FLAG);
		}
    
		else{
    
		ColorSwipeFlag = 0;
		}
	
    // If RFIDcard scanned was not a color flag
		if (!(RFIDScanned == RED_SWIPE || RFIDScanned == GREEN_SWIPE || RFIDScanned == WHITE_SWIPE)){
			// Send request atoll capture message to the Atoll 
			DataToSend[0] = TeamColor;	
			DataToSend[1] = RFIDScanned;	// Atoll number
			DataToSend[2] = SSPArray[0];	// serial #1
			DataToSend[3] = SSPArray[1];
			DataToSend[4] = SSPArray[2];
			DataToSend[5] = SSPArray[3];
			DataToSend[6] = SSPArray[4];
			DataToSend[7] = SSPArray[5];
			CreatePacket(8, BROADCAST_MSB, BROADCAST_LSB,BROADCAST_MESSAGE, REQ_CAPTURE, DataToSend);
		}	

		counter = 0;
		
	}	
}	

// Control loop
void UpdateSpeed(void) {
	
	static signed int WheelInput, Delta, uSpeed, WheelMag;
	static unsigned char left_speed, right_speed, Output, Value, LastValue;
	
	// Update our speed every CONTROL_PERIOD
	if(TimerPIC_IsTimerExpired(CONTROL_TIMER) == TimerPIC_EXPIRED){
			TimerPIC_InitTimer(CONTROL_TIMER, CONTROL_PERIOD);
			Value = (unsigned char)(51*(unsigned int)ReadADC()/41 - 30);
			//SetDuty(Value, Value);
			//SetDuty((unsigned int)100*Value/255, (unsigned int)100*Value/255);
			// Overflow
			WheelInput = (signed int)((((unsigned char)Value - LastValue)*75)/(512)); // Get wheel speed in RPM/2
			// Don't respond to the noise
			if((signed int)Value - LastValue < -127)
				WheelInput = -WheelInput;
			SetDuty((unsigned char)WheelInput, (unsigned char)WheelInput);
			if(WheelInput < 5 && WheelInput >-5)
				WheelInput = 0;
			if(WheelInput > 100 || WheelInput < -100)
				WheelInput++;
			LastValue = Value;	// Update the last A/D reading
			Delta = (RefSpeed - WheelInput);	// Find the error in speed
			// Turn off control at low speeds
			if(RefSpeed < 40 && RefSpeed> -40)
				Delta = 0;
			uSpeed = RefSpeed + 1*(Delta)/20;	// Combine feedforward and feedback terms	
			InputDirection = (uSpeed >= 0)?(0):(1);		// Figure out what direction the command is in
			Output = (uSpeed >=0)?(unsigned char)uSpeed:(unsigned char)(-uSpeed);	// Convert back to an unsigned char
			left_speed = CalculateLeftSpeed(Output, InputDirection, RefHeading);
			right_speed = CalculateRightSpeed(Output, InputDirection, RefHeading);
			//SetDuty(left_speed, right_speed);
		}
	
}	

// Check if we got a new message
void GetReply(void) {
	
	static unsigned char i, length;
	
  // If a new message has been completely received
	if(CheckNewMessage() == 1) {
    // Get the length of the message by calling ReturnDataSize 
		length = ReturnDataSize();
    // For each byte in the message
		for (i=0; i<length; i++) {
      // Store the message into the ReceivedData array of the appropriate index
			ReceivedData[i] = QueryMessage(i);
		}
    // Set the CurrentMessage received to the output of ProcessMSG
		CurrentMessage = ProcessMSG();
	}		
}

unsigned char CalculateLeftSpeed(unsigned char Speed, unsigned char Direction, unsigned char Heading) {
	
	unsigned char returnSpeed;
	signed char tempSpeed;
	
	// If Heading is less than 90 and Speed greater than 0
	if (Heading <= 90 && Speed > 0) {
		// tempSpeed = Speed - Speed*(90 - Heading)/90
		tempSpeed = ((signed char)Speed - ((unsigned int) Speed*(90 - Heading))/90);
		// If 90 - Heading is greater than the TURN_THRESHOLD
		if ((90 - Heading) > TURN_THRESHOLD)
			// Subtract TURN_OFFSET from tempSpeed
			tempSpeed -= TURN_OFFSET;
		// If tempSpeed is negative
		if (tempSpeed < 0)
			// Set returnSpeed to 0
			returnSpeed = 0;
		else
			// Otherwise tempSeed is returnSpeed
			returnSpeed = tempSpeed;
	}
	// else if Heading is greater than 90 and Speed is greater than 0
	else if (Heading > 90 && Speed > 0) {
		// returnSpeed = Speed + Speed*(Heading - 90)/90
		returnSpeed = (Speed + ((unsigned int) Speed*(Heading - 90))/90 );
		// If Heading - 90 is greater than TURN_THRESHOLD
		if ((Heading - 90) > TURN_THRESHOLD)
			// Subtract TURN_OFFSET from returnSpeed
			returnSpeed -= TURN_OFFSET;
	}
	// Otherwise returnSpeed is 0
	else
		returnSpeed = 0;

	// If Direction is equal to 1, We are going Forward
	if (Direction == 1){
	 	// Set Direction Pin (RD4) HI
		RD4 = 1;
	}
	// If Direction is equal to 0, We are going Backward
	else if (Direction == 0){ // Going Backward
		// Set Direction Pin (RD4) LO
		RD4 = 0;
	} 
	// Convert speeds 0-100 to PWM 20-100
	if (returnSpeed > 0)
		returnSpeed = (unsigned char)(20 + ((unsigned int)returnSpeed*4)/5);
	return returnSpeed;
}

unsigned char CalculateRightSpeed(unsigned char Speed, unsigned char Direction, unsigned char Heading) {

	unsigned char returnSpeed;
	signed char tempSpeed;

	// If Heading is less than 90 and Speed greater than 0
	if (Heading <= 90 && Speed > 0) {
		// returnSpeed = Speed + Speed*(90 - Heading)/90
		returnSpeed = (signed char)(Speed + ((unsigned int) Speed*(90 - Heading))/90);
		// If 90 - Heading is greater than the TURN_THRESHOLD	
		if ((90 - Heading) > TURN_THRESHOLD)
			// Subtract TURN_OFFSET from returnSpeed
			returnSpeed -= TURN_OFFSET;
	}
	
	// else if Heading is greater than 90 and Speed is greater than 0
	else if (Heading > 90  && Speed > 0) {
		// tempSpeed = Speed - Speed*(Heading - 90)/90
		tempSpeed = (Speed - ((unsigned int) Speed*(Heading - 90))/90 );
		// If Heading - 90 is greater than TURN_THRESHOLD
		if ((Heading - 90) > TURN_THRESHOLD)
			// Subtract TURN_OFFSET from tempSpeed
			tempSpeed -= TURN_OFFSET;
		// If tempSpeed is negative
		if (tempSpeed < 0)
			// returnSpeed is 0
			returnSpeed = (unsigned char)0;
		// Otherwise returnSpeed is tempSpeed
		else
			returnSpeed = tempSpeed;
	}
	
	// Otherwise returnSpeed is 0	
	else	
		returnSpeed = 0;
		
	// If Direction is equal to 1, We are going Forward
	if (Direction == 1){ // Going Forward
		// Set Direction Pin (RD5) LO
		RD5 = 0;
	}
	// If Direction is equal to 0, We are going Backward
	else if (Direction == 0){ // Going Backward
		// Set Direction Pin (RD4) HI
		RD5 = 1;
	} 

	// Convert speeds 0-100 to PWM 20-100
	if (returnSpeed > 0)
		returnSpeed = (unsigned char)(20 + ((unsigned int)returnSpeed*4)/5);
	return returnSpeed;
}

// Handle all the possible received messages using our Game SM
void HandleReceivedMessage(void) {
	
	
	static unsigned char LeftSpeed, RightSpeed;
	static unsigned char BroadcastCounter = 0;
	static unsigned char Teammate_MSB;
	static unsigned char Teammate_LSB;
	
  // Call GetReply to see if a new message was received
	GetReply();	
	
  // If TeamColor Flag is set to No_COLOR, we have been swiped by a white card
	if (TeamColor == NO_COLOR)
    // Set GameState to GAME_IDLE
		GameState = GAME_IDLE;
	
  // Switching on CurrentMessage
	switch(CurrentMessage) {
        // If it is a drive command
				case DriveCommand:
          // Toggle Port B0 for debugging
					RB0 ^= 1;
          // Set the Speed and RefDirection from the received message
					Speed = ReceivedData[6];
					RefDirection = ReceivedData[7];
					// Save our reference speed for controller
					RefSpeed = (RefDirection == 0)?(signed int)Speed:(signed int)-Speed;
					RefHeading = ReceivedData[8];
					#ifndef CONTROL_ON
					// Call CalculateLeftSpeed with Speed/RefDirection/RefHeading to get Left Speed
					LeftSpeed = CalculateLeftSpeed(Speed, RefDirection, RefHeading);
					// Call CalculateRightSpeed with Speed/RefDirection/RefHeading to get Right Speed
					RightSpeed = CalculateRightSpeed(Speed, RefDirection, RefHeading);
					// Call SetDuty with LeftSpeed and RightSpeed
					SetDuty(LeftSpeed, RightSpeed);
					#endif
          // Reset the CurrentMessage to NoMesssage
					CurrentMessage = NoMessage;
				break;
	}			
	
  // Switching on GameState
	switch(GameState) {
		// If it is GAME_IDLE
		case GAME_IDLE:

			// If SPI Message was a Color Swipe
			if (ColorSwipeFlag == 1 && TeamColor != NO_COLOR){
        // Set ColorSwipeFlag to 0
				ColorSwipeFlag = 0;
				// Start Teammmate Timer
				TimerPIC_InitTimer(TEAMMATE_TIMER, TEAMMATE_TIMEOUT);
				// Set The next GameState to LISTEN_FOR_TEAMMATE
				GameState = LISTEN_FOR_TEAMMATE;
			}
      // If a White Card Was Swiped
			if (ColorSwipeFlag == 1 && TeamColor == NO_COLOR) {
        // Set ColorSwipeFlag to 0
				ColorSwipeFlag = 0;
        // Send the Team Color
				DataToSend[0] = TeamColor;	// This will be ignored by CVC
        // Send the Atoll# 0 to indicate a clear atoll command
				DataToSend[1] = 0;	// Clear all atolls
        // Create the Packet as a B2C (Boat to Controller) Command
				CreatePacket(2, CVC_ADDRESS_MSB, CVC_ADDRESS_LSB,ACK_ON, COMMAND_B2C, DataToSend);
        // Reset the current message to no message
				CurrentMessage = NoMessage;
			}	
			
		break;
     
     // If game state is LISTEN_FOR_TEAMMATE
		case LISTEN_FOR_TEAMMATE:
			// If current Message was a Color BroadCast from Team Mate
			switch(CurrentMessage) {
				case ColorBroadcast:
					// if Teammate was your color
					if (ReceivedData[TEAM_COLOR] == TeamColor){
						// Found a Teammate, Send A Directed Reply Team ACK until ACK Received Back
						// Extract Address MSB
						Teammate_MSB = ReceivedData[1];
						Teammate_LSB = ReceivedData[2];
						BroadcastCounter = 0;
						DataToSend[0] = TeamColor;
						CreatePacket(1, Teammate_MSB, Teammate_LSB, ACK_ON, FIND_TEAM, DataToSend);
            // Set Next GameState to WAITING_FOR_ACK
						GameState = WAITING_FOR_ACK;
					}
          // Reset current message to NoMessage
					CurrentMessage = NoMessage;
				break;
        
        // If CurrentMessage is NoMessage
				case NoMessage:
				// If Teammate Timer Times out, No Teammate Found
					if(TimerPIC_IsTimerExpired(TEAMMATE_TIMER) == TimerPIC_EXPIRED){
				// Start Broadcasting Request For Teammate
						DataToSend[0] = TeamColor;
						CreatePacket(1, BROADCAST_MSB, BROADCAST_LSB, ACK_ON, FIND_TEAM, DataToSend);
            // Initalize BroadcastCounter to 0
						BroadcastCounter = 0;
            // Set GameState to BROADCAST_TEAM_MESSAGE
						GameState = BROADCAST_TEAM_MESSAGE;
					}	
          
        // If any other message was received
				default:
          // Ignore, and reset current message to NoMessage
					CurrentMessage = NoMessage;
				break;
			}	
		break;
		
		//  If GameState is WAITING_FOR_ACK
		case WAITING_FOR_ACK:
			switch(CurrentMessage) {
			    // If message received was ModemACK
				case ModemACK:
				    // Reset BroadcastCounter to 0
					BroadcastCounter = 0;
					// If TeamColor is Red
					if (TeamColor == RED)
					    // Call SetServo to Raise the Red Flag
						SetServo(RED_FLAG);
				    // else if TeamColor is Green
					else if (TeamColor == GREEN)
					    // Call SetServo to Raise the Green Flag
						SetServo(GREEN_FLAG);
				        // Set GameState to GAME_ON
				    	GameState = GAME_ON;
					// Reset CurrentMessage to No Message
					CurrentMessage = NoMessage;
				break;
				
				// If NoMessage Received
				case NoMessage:
						// Start Broadcasting Request For Teammate
						if (BroadcastCounter < 10){
							// If Message Was Broadcasted
							if(CheckTransmit() == 1){
								// Send Another Message
								DataToSend[0] = TeamColor;
								CreatePacket(1, Teammate_MSB, Teammate_LSB, ACK_ON, FIND_TEAM, DataToSend);
								BroadcastCounter++;
								}
						} else {
							// We've Exceeded max 10 Messages
							// Return to GAME_IDLE State
							GameState = GAME_IDLE;
							BroadcastCounter = 0;
						}
				break;

                // If any other message was received
				default:
				    // Ignore, and reset current message to NoMessage
					CurrentMessage = NoMessage;
				break;
			}	
		break;
		
		// If GameState is BROADCAST_TEAM_MESSAGE	
		case BROADCAST_TEAM_MESSAGE:

				switch(CurrentMessage) {
				    // If CurrentMessage is TeammateACK
					case TeammateACK:
					    // If ACK received, Teammate Found with same color
						if (ReceivedData[TEAM_COLOR] == TeamColor){
							// Set GameState to GAME_ON
							GameState = GAME_ON;
							// Reset BroadcastCounter to 0
							BroadcastCounter = 0;
							// If TeamColor is Red
							if (TeamColor == RED)
							    // Call SetServo to RED_FlAG
								SetServo(RED_FLAG);
						    // If TeamColor is Green
							else if (TeamColor == GREEN)
							    // Call SetServo to GREEN_FLAG
								SetServo(GREEN_FLAG);
							// Reset CurrentMessage to No Message	
							CurrentMessage = NoMessage;
						}
					break;
					
					// If NoMessage was Received
					case NoMessage:
						// If BroadcastCounter is less than 255
						if (BroadcastCounter < 255){
						    // If Message Was Broadcasted
						if(CheckTransmit() == 1){
							// Send Another Message
							DataToSend[0] = TeamColor;
							CreatePacket(1, BROADCAST_MSB, BROADCAST_LSB, ACK_ON, FIND_TEAM, DataToSend);
							// Increment the BroadcastCounter
							BroadcastCounter++;
							}
						} else {
						// We've Timed Out, Return to GAME_IDLE State
						GameState = GAME_IDLE;
						// Call SetServo to place Flag in TIMEOUT_FLAG position
						SetServo(TIMEOUT_FLAG);
						// Reset BroadcastCounter to 0
						BroadcastCounter = 0;
						}
					break;
				
					// If any other message was received 
					default:
						// Ignore, and reset current message to NoMessage
						CurrentMessage = NoMessage;
					break;
				}

		break;
		
		// If GameState is GAME_ON	
		case GAME_ON:
			// If TeamColor is NO_COLOR, a White Card was Swiped
			if (TeamColor == NO_COLOR){
				// Set GameState to GAME_IDLE
				GameState = GAME_IDLE;
			// Otherwise
			} else {
						switch(CurrentMessage) {
							// If Current Message was AtollSuccess
							case AtollSuccess:
								// Send Atoll Captured Broadcast
								DataToSend[0] = TeamColor;	
								DataToSend[1] = ReceivedData[ATOLL_NUM_REPLY];	// Atoll number
								CreatePacket(2, BROADCAST_MSB, BROADCAST_LSB,BROADCAST_MESSAGE, BROADCAST_CAPTURE, DataToSend);
								// Set Current Message to NoMessage
								CurrentMessage = NoMessage;
							break;
			
							// If Current Message was AtollFailure
							case AtollFailure:
								// Resend Atoll Capture Request
								DataToSend[0] = ReceivedData[COLOR_REPLY];	// New Owner	
								DataToSend[1] = ReceivedData[ATOLL_NUM_REPLY];	// Atoll number
								// If we fail, just send out who owns it
								CreatePacket(2, BROADCAST_MSB, BROADCAST_LSB,BROADCAST_MESSAGE, BROADCAST_CAPTURE, DataToSend);
								// Set Current Message to NoMessage
								CurrentMessage = NoMessage;
							break;
			
							// If Current Message was AtollCaptured
							case AtollCaptured:
								// Do Nothing
							break;
			
							// If Any other message was recieved
							default:
								// And the message was not NoMessage
								if (CurrentMessage != NoMessage)
									// Set the CurrentMessage to NoMessage
									CurrentMessage = NoMessage;
							break;
						}
					break;
				}
	}		
}	

#ifdef MAIN_TEST

void main(void) {

	// Turn off analog inputs (PCFG = 1111)
	PCFG0 = 1;
	PCFG1 = 1;
	PCFG2 = 1;
	PCFG3 = 1;
	
	
	
	// Set Debug LED ports as outputs (B0,D0,D2,D3,D4,D5)
	TRISB0 = 0;
	TRISD0 = 0;
	TRISD2 = 0; // Red LED
	TRISD3 = 0; // Green LED
	TRISD4 = 0;
	TRISD5 = 0;
	
	// Initialize Ports (RB0 = ON/ RD0 = RD2 = RD3 = RD4 = RD5 = OFF)
	RB0 = 1;
	RD0 = 0;
	RD2 = 0;
	RD3 = 0;
	RD4 = 0;
	RD5 = 0;
	
	// Initialize Servo Subsytem
	InitServo();
	// Initialize PWM Subsystem
	InitPWM();
	// Initialize SPI Subsystem (Make sure to InitSPI After InitPWM)
	InitSPI();
	// Initialize UART Subsystem 
	InitComm();
	// Initialize A2D Subsystem
	InitADC();
	// Initialize TimerPIC Subsystem
	TimerPIC_Init();
	TimerPIC_InitTimer(SEND_TIMER, SENDTIMEOUT);
	TimerPIC_InitTimer(CONTROL_TIMER, CONTROL_PERIOD);

    // Initialize Counter to 0
	counter = 0;
	
	// Enable Interrupts
	PEIE = 1;
	GIE = 1;	

    // Loop Forever
	while(1) {	
		#ifdef CONTROL_ON
		UpdateSpeed();
		#endif
		// Run Transmit State Machine
		TransmitSM();
		// Run Received Message State Machine		
		ReceiveMessageSM();
		// Run Handle Received Message State Machine
		HandleReceivedMessage();
	}

}

#endif // MAIN_TEST

