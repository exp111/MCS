//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// File: C-UserProgBody.C      EMT/MCS	      23.06.2009 Kzy  //
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#include "HC12RegDefsMCS.h"
#pragma interrupt_handler IRQ_Routine	// Interrupt Routine (vgl. HC12IntVecMCS.C)

//=== Globale Variablen ===
char caniflag = 0;
volatile int RecDataID;
volatile int RecDataL;
volatile char RecData[8];
volatile int RecDataRTR;

//=== Funktionsdeklarationen ===
// Display:
int LCD4x20C (int LineSel, int CsrPos, char* ChrPntr); //LCD-Display
void CANInit(void);

//=== main ===========================================================================

void main(void) {
//=== Variablendeklarationen ===
char LCDID[5] = {0};
char LCDDATA[9] = {0};
char LCDLength[2] = {0};
int i = 0;

//=== Initialisierungen ===
CANInit();
// LCD-Display 
LCD4x20C(0,0,0); // Display löschen
LCD4x20C(1, 2, "Identifier");
LCD4x20C(2, 2, "DATA");
LCD4x20C(3, 2, "DATALENGTH");

LCDID[0] = '0';
LCDID[2] = '0';
LCDID[3] = '0';
LCDID[5] = 0;

LCDDATA[8] = 0;

LCDLength[1] = 0;

// PORTs , Timer  etc.	 


asm ("CLI"); // Clear Interrupt-Mask (falls IRQ Routine vorhanden)

//=== Hauptroutine =============================================

while(1)
{ 
		if (caniflag == 1)
		{
		   	//RecID
			LCDID[1] = '0' + RecDataID / 100;
			LCD4x20C(1, 13, LCDID);
			
			//RecData
			for (i = 0; i < 8; i++)
			{
			 	if (i < RecDataL) //read
				{
				   LCDDATA[i] = RecData[i];
				}
				else
				{
				 	LCDDATA[i] = ' ';
				}
			}
			LCD4x20C(2, 13, LCDDATA);
			
			//RecDataL
			LCDLength[0] = '0' + RecDataL;
			LCD4x20C(3, 13, LCDLength);
		}
} //Abschluss while(1)

} //ende main()

//=== end of main ====================================================================

void CANInit(void)
{
 CANCTL0 = 1; //Ini 1
 CANCTL1 = 0b10000000; //Enable, Src=Osc, Mode = Normal
 
 CANBTR0 = 1; //Baud Rate Vorteiler 2 - 1 (500kbps)
 CANBTR1 = 0b00111010; //einfache Abtastung, TSEG2 - 1 = 3, TSEG1 - 1 = 10
 
 CANIDAC = 0b00010000; //16bit
 CANIDAR0 = 0b00011001; //id = 200
 CANIDAR1 = 0;
 
 CANIDMR0 = 0; //alle 11 bit überprüfen
 CANIDMR1 = 0b00000111; //untere 3 auf don't care
 
 CANCTL0 = 0; //Ini 0
 
 CANRIER = 1; //Interrupt durch RXFI
}



//=== IRQ Interrupt Routine ==========================================================

void IRQ_Routine(void)
{
//=== Variablendeklarationen ===
int i;
volatile unsigned char* CANPointer;
//=== IRQ-Routine =============================================
	  RecDataID = (CANRXIDR0 * 8) + (CANRXIDR1 >> 5); //Receive ID10 ... ID0
	  RecDataRTR = (CANRXIDR1 & 0b00010000);          //Recieve RTR
	  
	  RecDataL = CANRXDLR; //Receive DataLength
	  
	  if (RecDataL > 0)
	  {
	     CANPointer = &CANRXDSR0; //ReceiveBufferData
		 
		 for (i = 0; i < RecDataL; i++) //Read data into array
		 {
		     RecData[i] = *CANPointer;
			 CANPointer++;
		 }
	  }
	  CANRFLG = 1; //ACK
	  caniflag = 1;
}
//=== Ende IRQ Interrupt Routine =====================================================
