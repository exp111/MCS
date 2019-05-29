//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// File: C-UserProgBody.C      EMT/MCS	      23.06.2009 Kzy  //
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#include "HC12RegDefsMCS.h"
#pragma interrupt_handler IRQ_Routine	// Interrupt Routine (vgl. HC12IntVecMCS.C)

#define BIT0 0b00000001
#define BIT1 0b00000010
#define BIT2 0b00000100
#define BIT3 0b00001000
#define BIT4 0b00010000
#define BIT5 0b00100000
#define BIT6 0b01000000
#define BIT7 0b10000000

//=== Globale Variablen ===
volatile int ready = 0;
int synchronized = 0;

int hours = 0;
int minutes = 0;
int seconds = 0;
volatile int nextMin = 0;
volatile int nextHour = 0;
int setOnNextSec = 0;
int adder[7] = {1,2,4,8,10,20,40};

//=== Funktionsdeklarationen ===
// Display:
int LCD4x20C (int LineSel, int CsrPos, char* ChrPntr); //LCD-Display


//=== main ===========================================================================

void main(void) 
{
//=== Variablendeklarationen ===
//=== Initialisierungen ===

// LCD-Display 
	LCD4x20C(0,0,0); // Display löschen

// PORTs , Timer  etc.	 
	DDRA = 0b00001000;
	PORTA |= BIT3;
	
	//PR2 | PR1 | PR0
	TSCR2 = BIT2 | BIT1 | BIT0;
	//Timer Enable | Quick Easy Mode
	TSCR1 = BIT7 | BIT4;
	TIOS = BIT0;

	asm ("CLI"); // Clear Interrupt-Mask (falls IRQ Routine vorhanden)

//=== Hauptroutine =============================================
	LCD4x20C(2,2,"  :  :  ");
	while(1)
	{ 
		if (ready) //Timer overflowed
		{
		   	  TC0 = TCNT + 8750;
		      while (!(TFLG1 & BIT0))
		      {
		      }
		   	  if (!(PORTA & BIT0))
		   	  {
		 	   	 if (seconds < 29) //minutes
		   	   	 {
			   	    if (seconds != 28) //fuck the parity bit
			   	    {
						   nextMin += adder[seconds - 21];
			   		}
		   	   	 }
		   	     else if (seconds < 36)//hours
		   	     {
			   		if (seconds != 35) //fuck the parity bit
			   	    {
					   	   nextHour += adder[seconds - 29];
			   		}
		   	     }
		      }
		      ready = 0;
		}
	} //Abschluss while(1)


// Wenn MAIN kein CLOSED LOOP erfolgt ein kontrollierter Programm-Abbruch
// (mit Meldung auf dem LCD-Display) durch MCSsetupC.S

} //ende main()

//=== end of main ====================================================================

void IncreaseTime()
{
   if (setOnNextSec)
   {
      seconds = 0;
	  minutes = nextMin;
	  hours = nextHour;
	  setOnNextSec = 0;
	  return;
   }
   if (++seconds >= 60)
	   {
		  seconds = 0;
		  if (++minutes >= 60)
		  {
			 minutes = 0;
			 if (++hours >= 24)
			 {
				hours = 0;
			 }
		  }
	   }
}

void OutputTime(int hours, int minutes, int seconds)
{
	char text[3];
	text[2] = '\0';
	
	text[0] = '0' + (hours / 10);
	text[1] = '0' + (hours % 10);
	LCD4x20C(2,2,text);
	
	text[0] = '0' + (minutes / 10);
	text[1] = '0' + (minutes % 10);
	LCD4x20C(2,5,text);
	
	text[0] = '0' + (seconds / 10);
	text[1] = '0' + (seconds % 10);
	LCD4x20C(2,8,text);
}

//=== IRQ Interrupt Routine ==========================================================

void IRQ_Routine(void)
{
//=== Variablendeklarationen ===
//=== IRQ-Routine =============================================
	//Set IRQ low
	PORTA &= ~BIT3;
	
	if (!(PORTA & BIT1)) //Bitankündigung
	{
	   if (synchronized)
	   {
	   	  setOnNextSec = 1;
	   }
	   seconds = 59;
	   synchronized = 1;
	}
	else
	{
	   IncreaseTime();
	}
	
	OutputTime(hours, minutes, seconds);
	
	// Only read if synchronized and reads minute or hour
	if (synchronized && seconds > 20 && seconds < 36)
	{
	    if (seconds == 21) //reset aua
		{
		   nextHour = 0;
		   nextMin = 0;
		}
		ready = 1;
		if (TC0 & BIT0) //dummy read to clear flag
		{
		
		}
	}
	
	//Set IRQ high
	PORTA |= BIT3;
}
//=== Ende IRQ Interrupt Routine =====================================================
