//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// File: C-UserProgBody.C      EMT/MCS	      23.06.2009 Kzy  //
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#include "HC12RegDefsMCS.h"
#pragma interrupt_handler IRQ_Routine	// Interrupt Routine (vgl. HC12IntVecMCS.C)

//=== Globale Variablen ===

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
    LCD4x20C(1,2,"SCHALTER-PB0 = ?");
    LCD4x20C(2,2,"SCHALTER-PB1 = ?");
    LCD4x20C(3,2,"SCHALTER-PB2 = ?");
    LCD4x20C(4,2,"SCHALTER-PB3 = ?");

// PORTs , Timer  etc.	 
    DDRB = 0b00000000;
		DDRS = 0b11110000;

    asm ("CLI"); // Clear Interrupt-Mask (falls IRQ Routine vorhanden)

//=== Hauptroutine =============================================

    while(1)
    { 
	       PORTS = PORTB << 4;
    } //Abschluss while(1)


// Wenn MAIN kein CLOSED LOOP erfolgt ein kontrollierter Programm-Abbruch
// (mit Meldung auf dem LCD-Display) durch MCSsetupC.S

} //ende main()

//=== end of main ====================================================================




//=== IRQ Interrupt Routine ==========================================================

void IRQ_Routine(void)
{
//=== Variablendeklarationen ===
    char cur[2];
    int i = 0;
//=== IRQ-Routine =============================================
	  cur[1] = '\0';
	  for (i = 0; i < 4; i++)
	  {
	      cur[0] = '0' + ((PORTB >> i) & 1);
	      LCD4x20C(i + 1,17,cur);
	  }
}
//=== Ende IRQ Interrupt Routine =====================================================
