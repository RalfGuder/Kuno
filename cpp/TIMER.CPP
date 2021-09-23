/*************************************************************************/
/* T I M E R . C P P                                                     */
/*-----------------------------------------------------------------------*/
/* Aufgabe: Quellcode fuer die Umprogrammierung des Timer-               */
/*          Interrupt (sonst sind die 25 Frames pro sec nicht            */
/*          zu schaffen)                                                 */
/*                  K U N O   -   D E R   R I T T E R                    */
/*-----------------------------------------------------------------------*/
/* Autor         :  Ralf Guder                                           */
/* Arbeitsbeginn :  18.04.99                                             */
/* Arbeitsende   :  18.04.99                                             */
/* letztes Update:  18.04.99                                             */
/*-----------------------------------------------------------------------*/
/* Funktionen                                                            */
/* ----------                                                            */
/* set_rate : Initialisiert oder de-initialisiert den Timer-Interrupt    */
/* fastclock: Interrupt-Routine                                          */
/* fast     : stellt die Interrupt-Vektortabelle (INT 08)                */
/*            auf fastclock um                                           */
/* normal   : stellt die Interrupt-Vectortabelle (INT 08)                */
/*            wieder auf normal                                          */
/*************************************************************************/

/*-----------------------------------------------------------------------*/
#include "kuno.h"


/*-- Deklaration der internen Routinen ----------------------------------*/
VOID fast(VOID);
VOID normal(VOID);


/*-- Modul-Variablen ----------------------------------------------------*/
UINT countdown                        ;// clock tick interrupt countdown
                                       // leer
VOID interrupt(*old_int08)(__CPPARGS) ;// enthaelt alten Tastatur-Vektor
UINT speed= 0                         ;// clock acceleration factor
WORD chan_0 = 0x40                    ;// Port-Adresse fuer Kanal 0
WORD comm_reg =0x43                   ;// Port-Adresse fuer Commando-
                                       // Register



/*************************************************************************/
/* set_rate  : initialisiert die interne Ticker-Rate                     */
/*-----------------------------------------------------------------------*/
/* Eingabe   : rate = Faktor um den der Ticker erhoeht werden soll       */
/* Ausgabe   : keine                                                     */
/*************************************************************************/
VOID set_rate (int rate)
{
    if (rate==0) goto regular        ;// alte Einstellung herstellen

accel:
    if (speed!=0) goto return1       ;// Ticker wurde schon gesetzt
    speed=rate                       ;
    countdown=rate                   ;
    fast()                           ;// Ticker-Rate erhoehen
    goto return1                     ;

regular:
    if (speed==0) goto return1       ;// Ticker ist schon normal
    speed=rate                       ;
    normal()                         ;// Ticker-Rate zurueck auf
                                      // 18.2 ticks/second
return1:
    return                           ;// zurueck zum Aufrufer
}


/*************************************************************************/
/* fasteclock: Interrupt-Routine fuer INT08                              */
/*-----------------------------------------------------------------------*/
/* Eingabe   : keine                                                     */
/* Ausgabe   : keine                                                     */
/*************************************************************************/
VOID interrupt fastclock(__CPPARGS)  // interrupt handler to replace BIOS
{                                    // INT 08h

    countdown--                     ;// Countdown -1
    if (countdown==0) goto blastoff ;// Bei Null neusetzen

    geninterrupt(0x1C)              ;// wie ueblich nach Int08 Int1C auf- 
                                     // rufen
    outportb(0x20,0x20)             ;// Lower-Level-Interrupt wieder zu-  
                                     // lassen
    goto Ende1                      ;// fertig

blastoff:
asm pushf                           ;// naechsten Aufruf als Interrupt-
                                     // Aufruf simulieren, dazu Ruecksprung
                                     // in den Stack eintragen 
asm call [old_int08]                ;// Originalen BIOS-Ticker-Interrupt 
                                     // aufrufen
    countdown=speed                 ;// Countdown zuruecksetzen
    outportb(0x20,0x20)             ;// Lower-Level-Interrupt wieder zu-  
                                     // lassen

Ende1:                               // und Tschuesz!!
}

/*************************************************************************/
/* fast      : setzt neue INT08-Routine und die neue Ticker-Rate         */
/*-----------------------------------------------------------------------*/
/* Eingabe   : keine                                                     */
/* Ausgabe   : keine                                                     */
/*************************************************************************/
VOID fast(VOID)                      // Ticker um den Faktor speed erhoehen
{

asm cli                             ;// Interrupts sperren
    old_int08=getvect(0x8)          ;// alten Interrupt-Vector retten
    setvect(0x8,fastclock)          ;// neuen Interrupt-Vector setzen
    outportb(comm_reg,0x36)         ;// Kanal 0 auf Modus 3 einstellen

    _AX=65536/speed                 ;// Tickerwert berechnen und ausgeben
    outportb(chan_0,_AL)            ;
    outportb(chan_0,_AH)            ;

asm sti                             ;// Interrupts freigeben
    return                          ;// Bye-bye
}




/*************************************************************************/
/* normal    : setzt alten INT08 und alte Ticker-Rate                    */
/*-----------------------------------------------------------------------*/
/* Eingabe   : keine                                                     */
/* Ausgabe   : keine                                                     */
/*************************************************************************/
VOID normal(VOID)                    // Reset Tickerrate auf 18.2
{                                    // ticks/sek zurueck

asm cli                             ;// Interrupts sperren

    outportb(comm_reg,0x36)         ;// Kanal 0 auf Modus 3 einstellen

    outportb(chan_0,0)              ;// Ticker setzen
    outportb(chan_0,0)              ;

    setvect(0x8,old_int08)          ;// alten Int08-Vector setzen

asm sti                             ;// Interrupts freigeben

    return                          ;// Weidmannheil
}
/*--  E N D E  ----------------------------------------------------------*/

