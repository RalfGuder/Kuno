/********************************************************************/
/* A D L I B . C P P                                                */
/*------------------------------------------------------------------*/
/* Aufgabe       :  Quellcode fuer alle Sound-Routinen fuer         */
/*                  K U N O   -   D E R   R I T T E R               */
/*------------------------------------------------------------------*/
/* Autor         :  Ralf Guder                                      */
/* Arbeitsbeginn :  30.01.96                                        */
/* Arbeitsende   :                                                  */
/* letztes Update:  12.03.96                                        */
/*------------------------------------------------------------------*/
/* Funktionen                                                       */
/* ----------                                                       */
/*------------------------------------------------------------------*/
/* Info                                                             */
/* ----                                                             */
/********************************************************************/

/*== INCLUDE-Dateien ===============================================*/
#include <dos.h>
#include "types.h"
#include "adlib.h"

/*== externe Funktionen ============================================*/
PBYTE Voice(INT)                      ;/* liefert den Zeiger auf das*/
                                       /*  benoetigte Instrument    */

/*== Typendeklarationen ============================================*/


/*== Konstante =====================================================*/

CWORD PORT=0x388                      ;/* Adlib-Port                */
CBYTE FT1 =0x40                       ;/* Timer 1 Ueberlauf         */

/*== Arrays ========================================================*/
WORD  TonLast   [11]                  ;/* letzter gespielter Ton    */
BYTE  StAktiv   [11]                  ;/* Stimme aktiv (JA/NEIN)    */
BYTE  LiedAktiv [11]                  ;/* Lied aktiv (JA/NEIN)      */
PWORD LiedZeiger[11]                  ;/* Zeiger auf das aktive Lied*/
PWORD StZeiger  [11]                  ;/* Zeiger auf die aktive Spur*/
BYTE  oszoffs   [  ]=
      {0x00,0x01,0x02,0x03,0x04,
       0x05,0x08,0x09,0x0a,0x0b,
       0x0c,0x0d,0x10,0x11,0x12,
       0x13,0x14,0x15}                ;/* Oszillatorenoffsets       */

WORD  voioffs   [  ]=
      {0x0401,0x0502,0x0603,
       0x0a07,0x0b08,0x0c09,
       0x100d,0x110e,0x1210,
       0x1210,0x110e}                 ;
BYTE  sounds    [12]                  ;

/*== Variable ======================================================*/
WORD  DRUM                            ;/* Registerspiegel fuer      */
                                       /*  Rhythmus                 */
PWORD Melodie                         ;/* Zeiger auf die Melodie    */
PBYTE Instrument                      ;/* Zeiger auf das benoetigte */
                                       /*  Instrument               */
WORD  ADLIB                           ;/* NEIN = keine Adlibkarte   */
BYTE  Zaehler,Wert                    ;
BYTE  MODULATOR,TRAEGER               ;/* Nr der Oszillatorten      */

/********************************************************************/
/* StimmeActiv : aktiviert die entsprechende Adlibstimme            */
/*------------------------------------------------------------------*/
/* Eingabe : StNr = Nummer der Stimme                               */
/* Ausgabe : keine                                                  */
/********************************************************************/
VOID StimmeActiv(BYTE StNr)
{
 StAktiv[StNr]=AN                     ;
 return                               ;
}

/********************************************************************/
/* LiedActiv : aktiviert die entsprechende Liedstimme               */
/*------------------------------------------------------------------*/
/* Eingabe : StNr = Nummer der Stimme                               */
/* Ausgabe : keine                                                  */
/********************************************************************/
VOID LiedActiv(BYTE StNr)
{
/*-- Lied ein ------------------------------------------------------*/
 LiedAktiv[StNr]=AN                   ;

/*-- Stimme ein ----------------------------------------------------*/
 StimmeActiv(StNr)                    ;
 return                               ;
}
/********************************************************************/
/* StimmeDeactiv : aktiviert die entsprechende Adlibstimme          */
/*------------------------------------------------------------------*/
/* Eingabe : StNr = Nummer der Stimme                               */
/* Ausgabe : keine                                                  */
/********************************************************************/
VOID StimmeDeactiv(BYTE StNr)
{
 StAktiv[StNr]=AUS                    ;
 return                               ;
}

/********************************************************************/
/* StimmeDaten : Daten fuer die entsprechende Adlibstimme           */
/*------------------------------------------------------------------*/
/* Eingabe : StNr   = Nummer der Stimme                             */
/*           StZeig = Zeiger auf die entsprechenden Daten           */
/* Ausgabe : keine                                                  */
/********************************************************************/
VOID StimmeDaten(BYTE StNr, PWORD StZeig)
{
 StZeiger[StNr]=StZeig                ;
 return                               ;
}
/********************************************************************/
/* LiedDaten : Daten fuer die entsprechende Adlibstimme             */
/*------------------------------------------------------------------*/
/* Eingabe : StNr   = Nummer der Stimme                             */
/*           StZeig = Zeiger auf die entsprechenden Daten           */
/* Ausgabe : keine                                                  */
/********************************************************************/
VOID LiedDaten(BYTE StNr, PWORD StZeig)
{
 WORD OFF,SEG                         ;
				      ;
 LiedZeiger[StNr]=StZeig              ;/* Zeiger notieren           */
 OFF=(WORD)StZeig[0]                  ;/* Offset der Stimme         */
 SEG=(WORD)StZeig[1]                  ;/* Segment der Stimme        */
 StZeiger[StNr]=(PWORD)MK_FP(SEG,OFF) ;/* erste Spur notieren       */
 return                               ;
}

/********************************************************************/
/* Adlibprog : Programmiert ein Adlib-Register                      */
/*------------------------------------------------------------------*/
/* Eingabe : Register (High=Reg.-Nr; Low=Wert)                      */
/* Ausgabe : keine                                                  */
/********************************************************************/
VOID Adlibprog ( WORD Register )
{
 BYTE HIGH,LOW                        ;
				      ;
/*-- Werte vertauschen ---------------------------------------------*/
 HIGH=LOBYTE(Register)                ;
 LOW =HIBYTE(Register)                ;
                                      ;
/*-- Register ansprechen -------------------------------------------*/
 outportb(PORT,LOW)                   ;
                                      ;
/*-- Warten --------------------------------------------------------*/
 Zaehler=6                            ;
 while (Zaehler!=0)
  {
   LOW=inportb(PORT)                  ;
   Zaehler--                          ;
  }

/*-- Daten Åbergeben -----------------------------------------------*/
 outportb(PORT+1,HIGH)                ;

/*-- Warten --------------------------------------------------------*/
 Zaehler=35                           ;
 while (Zaehler!=0)
 {
  LOW=inportb(PORT)                   ;
  Zaehler--                           ;
 }

return                                ;/* zurÅck zum Aufrufer       */
}

/********************************************************************/
/* oszprog : Programmiert einen Oszillator                          */
/*------------------------------------------------------------------*/
/* Eingabe : Oszillator = Nummer des Oszillators                    */
/* Ausgabe : --                                                     */
/********************************************************************/
VOID oszprog(WORD Oszillator)
{
 WORD Register                        ;
 BYTE LOW,HIGH                        ;

 HIGH=0x20+oszoffs[Oszillator-1]      ;/* erstes Registeroffset     */
 while (HIGH<0xA0)                     /* Register 20-80 programm   */
 {
  LOW=Instrument[0]                   ;/* Registerdaten laden       */
  Instrument++                        ;/* Zeiger auf Daten erhoehen */
  Register=MAKEWORD(HIGH,LOW)         ;
  Adlibprog(Register)                 ;
  HIGH=HIGH+0x20                      ;
 }
 HIGH=HIGH+0x40                       ;/* Register E0               */
 Instrument++                         ;
 LOW=Instrument[0]                    ;
 Instrument++                         ;
 Register=MAKEWORD(HIGH,LOW)          ;
 Adlibprog(Register)                  ;

 return                               ;
}

/********************************************************************/
/* Adlibvoice: Programmiert eine Stimme (Oszillator-Paar)           */
/*------------------------------------------------------------------*/
/* Eingabe : Nummer der Stimme und Zeiger auf die Instrumenten-Daten*/
/* Ausgabe : keine                                                  */
/********************************************************************/
VOID Adlibvoice(WORD Stimme, PBYTE Geraet)
{
 WORD Sounds4                         ;
 INSTRUMENT* VOICE                    ;
                                      ;
 if (ADLIB==0)  return                ;/* keine Karte               */
 Zaehler=0                            ;/* Zaehler auf 0             */
                                      ;
 (PBYTE)VOICE=Geraet                  ;
                                      ;
/*-- Werte fÅr den Modulator einstellen ----------------------------*/
 sounds[8]=(VOICE->M_AR<<4)           ;/* Anstiegs-Rate             */
 sounds[8]=(sounds[8] | VOICE->M_DR)  ;/* Abstiegs-Rate             */
 sounds[9]=(VOICE->M_SL<<4)           ;/* Halte-Rate                */
 sounds[9]=(sounds[9] | VOICE->M_RR)  ;/* Auskling-Rate             */
 sounds[6]=((VOICE->M_EG^0x10)<<5)    ;/* Halten                    */
 sounds[6]=(sounds[6] |                /* Huellkurverskalierung     */
                    (VOICE->M_KSR<<4));
 sounds[6]=(sounds[6] |                /* Frequenzmultiplikator     */
                     VOICE->M_MULTI)  ;
 sounds[10]=(VOICE->M_FB<<1)          ;/* RÅckwirkung               */
 sounds[6]=(sounds[6] |                /* Pitchvibrato              */
                    (VOICE->M_VIB<<6));
 sounds[7]=VOICE->M_TL                ;/* Daempfung                 */
 sounds[7]=(sounds[7] |                /* Filter                    */
                    (VOICE->M_KSL<<6));
 sounds[6]=(sounds[6] |                /* Amplitudenvibrato         */
                    (VOICE->M_AM<<7)) ;
 sounds[10]=(sounds[10] |              /* Frequenzmodulation        */
                    (VOICE->M_C^0x01));
 sounds[11]=VOICE->M_WS               ;/* Wellenform                */

/*--Werte fÅr den Traeger einstellen -------------------------------*/
 sounds[2]=(VOICE->T_AR<<4)           ;/* Anstiegs-Rate             */
 sounds[2]=(sounds[2] | VOICE->T_DR)  ;/* Abstiegs-Rate             */
 sounds[3]=(VOICE->T_SL<<4)           ;/* Halte-Rate                */
 sounds[3]=(sounds[3] | VOICE->T_RR)  ;/* Auskling-Rate             */
 sounds[0]=((VOICE->T_EG^0x10)<<5)    ;
 sounds[0]=(sounds[0] | 
                    (VOICE->T_KSR<<4));
 sounds[0]=(sounds[0] | 
                     VOICE->T_MULTI)  ;
 sounds[4]=(VOICE->T_FB<<1)           ;
 sounds[0]=(sounds[0] | 
                    (VOICE->T_VIB<<6));
 sounds[1]=VOICE->T_TL                ;
 sounds[1]=(sounds[1] | 
                    (VOICE->T_KSL<<6));
 sounds[0]=(sounds[0] |
                    (VOICE->T_AM<<7)) ;
 sounds[4]=(sounds[4] |
                    (VOICE->T_C^0x01));
 sounds[5]=VOICE->T_WS                ;
                                      ;
 Instrument=sounds                    ;
 Sounds4  =voioffs[Stimme]            ;
 TRAEGER  =HIBYTE(Sounds4)            ;
 MODULATOR=LOBYTE(Sounds4)            ;
                                      ;
 oszprog(TRAEGER)                     ;
                                      ;
 if (MODULATOR<0xF0)
 {
  oszprog(MODULATOR)                  ;
 }
                                      ;
 TRAEGER  =Stimme+0xC0                ;
 MODULATOR=sounds[10]                 ;
 Sounds4  =MAKEWORD(TRAEGER,MODULATOR);
 Adlibprog(Sounds4)                   ;
                                      ;
  return                              ;
}

/********************************************************************/
/* Adlibtest : Testet, ob eine Soundkarte vorhanden ist             */
/*------------------------------------------------------------------*/
/* Eingabe : keine                                                  */
/* Ausgabe : 0 = Adlib erkannt                                      */
/*           1 = keine Karte                                        */
/********************************************************************/
int Adlibtest (void)
{
 WORD Register                        ;/* enthaelt den Adlib        */
                                      ;/*   Registerwert            */
 WORD cx                              ;/* Zaehler                   */
                                      ;
 Register = 0x02FF                    ;/* Timer 1 mit Ffh           */
 Adlibprog(Register)                  ;
                                      ;
 Register = 0x0480                    ;/* Timer 1 Reset             */
 Adlibprog(Register)                  ;
                                      ;
 Register = 0x0401                    ;/* Timer 1 Starten           */
 Adlibprog(Register)                  ;
                                      ;
 cx=0xFFFF                            ;/* Durchlaufschleife         */
                                      ;
Adlibt1:                              ;
 Register=inportb(PORT)               ;
 if ((Register&FT1)==FT1) goto Adlibt2;/* Wenn öberlauf, dann ist   */
                                      ;/* Adlib vorhanden           */
 if (cx==0) goto Adlibt2              ;/* Wenn cx=0, dann Adlib     */
                                      ;/*  nicht vorhanden          */
  cx--                                ;
  goto Adlibt1                        ;
                                      ;
Adlibt2:                              ;
 ADLIB=cx                             ;/* Merke, ob ja oder nein    */
 return (cx)                          ;/* zurÅck mit ERROR          */
}

/********************************************************************/
/* Adlibreset : setzte die Soundkarte zurÅck                        */
/*------------------------------------------------------------------*/
/* Eingabe : keine                                                  */
/* Ausgabe : keine      	                                            */
/********************************************************************/
void Adlibreset(void)
{
 BYTE x                               ;/* Zaehler                   */
 if (ADLIB==0) return                 ;/* keine Karte vorhanden     */

/*-- Lautstaerke fuer alle Oszillatoren zuruecksetzen --------------*/
 _AL=0x3F                             ;
 for (x=0x40;x<0x56;x++)
 {
 _AL=0x3F                             ;
 _AH=x                                ;
  Adlibprog(_AX)                      ;
 }                                    ;
                                      ;
/*-- 11-Stimmenmodus einschalten -----------------------------------*/
 DRUM=0xED20                          ;
 Adlibprog(DRUM)                      ;
                                      ;
/*-- Musikmodus einschalten ----------------------------------------*/
 Adlibprog(0x0800)                    ;
                                      ;
/*-- Wellenformat zulassen -----------------------------------------*/
 Adlibprog(0x0120)                    ;
                                      ;
/*-- Operatoren ausschalten ----------------------------------------*/
 _AL=0x00                             ;
 for (x=0xB0;x<0xB9;x++)
 {
  _AL=0x00                            ;
  _AH=x                               ;
  Adlibprog(_AX)                      ;
 }                                    ;
                                      ;
 return                               ;
}

/********************************************************************/
/* Adlibton : spielt einen Ton ab                                   */
/*------------------------------------------------------------------*/
/* Eingabe : Ton,Stimme                                             */
/* Ausgabe : keine      	                                            */
/********************************************************************/
void Adlibton(WORD Ton, BYTE Stimme)
{
 INT FNL                              ;
 INT BLOCK                            ;
 if (Ton==0) return                   ;/* zurÅck wenn kein Ton      */
 Ton=Ton|0x2000                       ;/* KON setzen                */
 FNL=((0xA0+Stimme)<<8)+LOBYTE(Ton)   ;/* LowFrequenz berechnen     */
 BLOCK=((0xB0+Stimme)<<8)+HIBYTE(Ton) ;/* HighFrequenz berechnen    */
 Adlibprog(FNL)                       ;/* OPL programmieren         */
 Adlibprog(BLOCK)                     ;
 return                               ;/* zurÅck zum Aufrufer       */
}

/********************************************************************/
/* Adlibtonaus : schaltet einen Ton ab                              */
/*------------------------------------------------------------------*/
/* Eingabe : Ton,Stimme                                             */
/* Ausgabe : keine                                                  */
/********************************************************************/
void Adlibtonaus(WORD Ton, BYTE Stimme)
{
 INT FNL                              ;
 INT BLOCK                            ;
                                      ;
 if (Ton==0) return                   ;/* zurueck wenn kein Ton     */
 _BX=Ton                              ;
 _AH=0xa0+Stimme                      ;
 _AL=_BL                              ;
 FNL=_AX                              ;
 _AH=_AH+0x10                         ;
 _AL=_BH                              ;
 BLOCK=_AX                            ;
 Adlibprog(FNL)                       ;
 Adlibprog(BLOCK)                     ;
return                                ;
}
/********************************************************************/
/* PlayStimme : spielt die entsprechende Adlibstimme                */
/*------------------------------------------------------------------*/
/* Eingabe : StNr = Nummer der Stimme                               */
/* Ausgabe : keine                                                  */
/********************************************************************/
int PlayStimme(BYTE StNr)
{
 BYTE Nr                              ;
 WORD Ton                             ;
 WORD lTon                            ;
 PWORD Platz                          ;
                                      ;
 if (ADLIB==0)         return(0)      ;
 if (StAktiv[StNr]==0) return(2)      ;
                                      ;
 Platz=StZeiger[StNr]                 ;
 Ton=Platz[0]                         ;
 Platz++                              ;
 StZeiger[StNr]=Platz                 ;
 Nr=StNr                              ;
 if (StNr==9) Nr=8                    ;
 if (StNr==10) Nr=7                   ;
                                      ;
 if (Ton==0xBBBB) return(1)           ;/* Spurende                  */
                                      ;
 if (Ton==0xDDDD) return(3)           ;/* Liedende                  */
 if ((Ton< 0x0100)&&(Ton>0))           /* Voice einstellung         */
 {
  (PBYTE)Platz=Voice(Ton)             ;
  Adlibvoice(StNr,(PBYTE)Platz)       ;
  return (4)                          ;
 }
                                      ;
 if (Ton==0xFFFF)                      /* Pause                     */
 {
  if (StNr==6 ) DRUM=DRUM^0x10        ;
  if (StNr==7 ) DRUM=DRUM^0x08        ;
  if (StNr==8 ) DRUM=DRUM^0x04        ;
  if (StNr==9 ) DRUM=DRUM^0x02        ;
  if (StNr==10) DRUM=DRUM^0x01        ;
  if (StNr>5 )  Adlibprog(DRUM)       ;
  lTon=TonLast[StNr]                  ;
  TonLast[StNr]=0                     ;
  Adlibprog(DRUM)                     ;
  if (lTon!=0) Adlibtonaus(lTon,Nr)   ;
  return(0)                           ;
 }
                                      ;
 lTon=TonLast[StNr]                   ;
 if ((Ton!=0)&&(StNr<6))
 {
  if (lTon!=0)
  {
   Adlibtonaus(lTon,Nr)               ;
  }
 }
 else
 {
    if (StNr>5)
    {
    if (StNr==6 ) DRUM=DRUM&0xFFeF    ;
    if (StNr==7 ) DRUM=DRUM&0xFFF7    ;
    if (StNr==8 ) DRUM=DRUM&0xFFFB    ;
    if (StNr==9 ) DRUM=DRUM&0xFFFD    ;
    if (StNr==10) DRUM=DRUM&0xFFFE    ;
    if (StNr>5 ) Adlibprog(DRUM)      ;
    Adlibtonaus(lTon,Nr)              ;
    }
 }
 if (Ton!=0)
 {
  TonLast[StNr]=Ton                   ;
  if (StNr==6 )  DRUM=DRUM|0x10       ;
  if (StNr==7 )  DRUM=DRUM|0x08       ;
  if (StNr==8 )  DRUM=DRUM|0x04       ;
  if (StNr==9 )  DRUM=DRUM|0x02       ;
  if (StNr==10)  DRUM=DRUM|0x01       ;
  if (StNr>5 )   Adlibprog(DRUM)      ;
 }
  Adlibton(Ton,Nr)                    ;
                                      ;
 return (0)                           ;
}

/********************************************************************/
/* Stimme : spielt die entsprechende Adlibstimme                    */
/*------------------------------------------------------------------*/
/* Eingabe : StNr = Nummer der Stimme                               */
/* Ausgabe : keine                                                  */
/********************************************************************/
void Stimme(BYTE StNr)
{
 BYTE  Nr                             ;
 WORD  Ton,lTon,SEG,OFF               ;
 PWORD Platz                          ;

 if (ADLIB==0)         return         ;
 if (LiedAktiv[StNr]==0)
 {
  if (StAktiv[StNr]==0) return        ;
  Nr=PlayStimme(StNr)                 ;
  if ((Nr==1)||(Nr==3))
     StimmeDeactiv(StNr)              ;/* Spur-, bwz Liedende       */
  return                              ;
 }

Stimm1:

 Nr=PlayStimme(StNr)                  ;
 if (Nr==3) StimmeDeactiv(StNr)       ;/* Lied zu Ende              */
 if (Nr==4) goto Stimm1               ;
 if (Nr==1)                            /* Spurwechsel               */
 {
  Platz=LiedZeiger[StNr]              ;
  Platz++                             ;
  Platz++                             ;
  LiedZeiger[StNr]=Platz              ;
  OFF=Platz[0]                        ;
  SEG=Platz[1]                        ;
  Platz=(PWORD)MK_FP(SEG,OFF)         ;
  StimmeDaten(StNr,Platz)             ;
  goto Stimm1                         ;
 }

return                                ;
}
