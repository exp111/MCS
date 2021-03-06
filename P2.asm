;
; Required USER PROGRAM BODY  when using  MCSsetupASM.S               24.05.2012 Kzy
;

.AREA	Memory (abs)
STACK	=	$1000		; Nach RESET $1000, nach SetUp $2000=(End of RAM)+1
SetUp	=	$F800		; Suboutine System SetUp

; User Support:		
LCD4x20A=	SetUp+3		; Subroutine LCD4x20A (Version Assembler)
VoltAnz	=	SetUp+6		; Subroutine VoltAnz  (Ausgabe "#,##V")
Wait    =	SetUp+9		; Subroutine Warte1ms (Verzögerungszeit tv=1ms)

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
BIT4    = 0b00010000
BIT7    = 0b10000000 ; SPIF auslesen

SUCCSTEP: .BLKB 1
UE: .BLKB 1

.ORG    $4000		; ASM Code ab 0x4000

START:  LDS #STACK
        JSR SetUp ; Init stack & stuff
        MOVB #0b00000000, DDRA ; PB3 & PB4 as Output
        BSET DDRA, #BIT3
        BSET DDRA, #BIT4
        MOVB #0b0, DDRB ; PB0 as Input
        LDX #0xFFFE ; Init LCD
        JSR LCD4x20A
        LDX #LCDEI ; Init First Line (ADU Wert)
        JSR LCD4x20A

SPIINI: BSET PORTA, #BIT3
        MOVB #0b01010100, SPICR1 ; SPE 1 MSTR 1 CPOL 0 CPHA 1 LSBFE 0
        MOVB #0b00000000, SPIBR ; 8MHz
        
        
MLOOP:  MOVB #0b10000000, SUCCSTEP ; Reset/Init Values
        MOVB #0b00000000, UE

        JSR TRIGGER
SUCCLOP:JSR INCUE ; UE = UE + STEP
        LDAA UE ; Read current value into akka
        JSR DAOUT ; Output akka
        JSR Wait ; Wait 1 ms
        JSR SUCC
        LDAA SUCCSTEP
        CMPA #0
        BNE SUCCLOP ; JMP if mask != 0 -> its still going on
        
        ; Output the end value to display
        LDY  #0x0308 ; Position
        LDAA UE ; Volt Value
        JSR VoltAnz
        
        BRA MLOOP
        
TRIGGER:LDAA #0 ; 0V ausgeben
        JSR DAOUT ; 
        JSR Wait
        BSET PORTA, #BIT4 ; Triggerimpuls
        JSR Wait
        BCLR PORTA, #BIT4 ; Reset Trigger
        JSR Wait
        RTS
        
SUCC:   LDAA PORTB
        BRSET PORTB, #BIT0, NEXT ; cmp to PB0
ELSEN:  LDAA UE
        SUBA SUCCSTEP; larger than -> remove bit
        STAA UE
NEXT:   LDAB SUCCSTEP
        LSRB
        STAB SUCCSTEP
        RTS
	   
INCUE:  LDAA UE
        ADDA SUCCSTEP
        STAA UE
        RTS
        
DAOUT:  BCLR PORTA, #BIT3 ; SYNC
        LDAB #0 ; Clear B
        LSRD ; From
        LSRD ; 0b11111111
        LSRD ; to
        LSRD ; 0b00001111
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
