;
; Required USER PROGRAM BODY  when using  MCSsetupASM.S               24.05.2012 Kzy
;

	.AREA	Memory (abs)
STACK	=	$1000		; Nach RESET $1000, nach SetUp $2000=(End of RAM)+1
SetUp	=	$F800		; Suboutine System SetUp

; User Support:		
LCD4x20A=	SetUp+3		; Subroutine LCD4x20A (Version Assembler)
VoltAnz	=	SetUp+6		; Subroutine VoltAnz  (Ausgabe "#,##V")
Warte1ms=	SetUp+9		; Subroutine Warte1ms (Verzögerungszeit tv=1ms)

; **********************************************************************************

PORTA   = 0x0000
DDRA    = 0x0002
PORTB   = 0x0001
DDRB    = 0x0003 

SPICR1  = 0x00D8
SPICR2  = 0x00D9
SPIBR   = 0x00DA
SPISR   = 0x00DB
SPIDR   = 0x00DD

	.ORG	$1000		; ASM Variablen ab 0x1000
BIT0    = 0b00000001
BIT3    = 0b00001000
BIT7    = 0b10000000 ; SPIF auslesen

ADU4:    .BLKB 2
ADUASC:  .BLKB 2
ADUTERM: .BLKB 1

 .ORG    $4000		; ASM Code ab 0x4000
 
START:   LDS #STACK
		 JSR SetUp ; Init stack & stuff
		 MOVB #0b0, DDRB ; PB0 as Input
	     LDX #0xFFFE ; Init LCD
         JSR LCD4x20A
         LDX #LCDEI ; Init First Line (ADU Wert)
         JSR LCD4x20A

SPIINI: BSET DDRA, #BIT3 ; SYNC as Output 
		BSET PORTA, #BIT3
        MOVB #0b01010100, SPICR1 ; SPE 1 MSTR 1 CPOL 0 CPHA 1 LSBFE 0
        MOVB #0b00000000, SPIBR ; 8MHz
        

MLOOP: LDY  #0x0308 ; Position
	   LDAA #0xFF ; Volt Value
       JSR VoltAnz
	   LDAA #0b11111111
	   JSR DAOUT
	   JSR Warte1ms
       BRA MLOOP

	   
DAOUT:  BCLR PORTA, #BIT3 ; SYNC
		LSRD ; From
		LSRD ; 0b11111111
		LSRD ; to
		LSRD ; 0b00001111
        LDAA #0b00001111 ;xxDD7654
		LDAB #0b11110000 ;3210xxxx
        STAA SPIDR ; Write A
LOOP1:  BRCLR SPISR, #BIT7, LOOP1
        LDAA SPIDR ; Dummy Read
        STAB SPIDR ; Write B
LOOP2:  BRCLR SPISR, #BIT7, LOOP2
        LDAB SPIDR ; Dummy Read
        BSET PORTA, #BIT3 ; SYNC
		RTS


		; Init consts
LCDEI: .DB 2,6
        .DW TXT
TXT:   .ASCIZ "ADU-WERT:"

; **********************************************************************************

; RESTART VECTOR

	.ORG	$FFFE
	.DW	START		; RESTART VECTOR
