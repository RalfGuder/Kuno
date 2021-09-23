/*************************************************************************/
/* V G A W O R K . C P P                                                 */
/*-----------------------------------------------------------------------*/
/* Aufgabe       :  Quellcode fuer alle VGA-Routinen                     */
/*                  K U N O   -   D E R   R I T T E R                    */
/*-----------------------------------------------------------------------*/
/* Autor         :  Ralf Guder                                           */
/* Arbeitsbeginn :  30.01.96                                             */
/* Arbeitsende   :                                                       */
/* letztes Update:  16.04.99                                             */
/*-----------------------------------------------------------------------*/
/* Funktionen                                                            */
/* ----------                                                            */
/* init320200  : initialisiert den 320*200 Punkte-Modus                  */
/* setpix      : verleiht einem Punkt eine bestimmte Farbe               */
/* getpix      : liefert die Farbe eines Punktes                         */
/* setpage     : Waehlt die Seite aus, auf die bei Funktionsaufrufen von */
/*               setpix und getpix zugegriffen werden soll               */
/* showpage    : Bringt eine der Bildschirmseiten zur Anzeige            */
/* Seite_aktiv : liefert Nummer der BS-Seite, die beschrieben werden kann*/
/* blockmove   : verschiebt eine BS-Seite im Video-RAM                   */
/*************************************************************************/

/*-- INCLUDE-Dateien ----------------------------------------------------*/
#include "ZEICHEN.CPP"
#include "kuno.h"

WORD VIO_SEG =0xA000               ;// Video-Segment 

/*-- externe-Routinen ---------------------------------------------------*/
/*   Modul "FILEWORK.CPP"                                                */
void Charladen1 (unsigned char*,   /* laedt 8x8 Zeichensatz              */
		 unsigned char*)  ;
void Charladen2 (unsigned char*,   /* laedt 6x5 Zeichensatz              */
		 unsigned char*)  ;

/*-- Konstanten ---------------------------------------------------------*/
unsigned char SRC_PAGE_AKTIV      ;/* Speicher der aktiven Seite         */


/*************************************************************************/
/* INIT320200: initialisiert den 320*200 Punkte-Modus                    */
/*-----------------------------------------------------------------------*/
/* Eingabe   : keine                                                     */
/* Ausgabe   : keine                                                     */
/*************************************************************************/
void init320200( void )
{

	 /*-- zunaechst ganz normal den Modus 13h einstellen, damit das  */
	 /*-- BIOS die meiste Arbeit der Initialisierung uebernimmt.     */
	 /*-- Danach die Register veraendern, die sich noch nicht in     */
	 /*-- der gewuenschten Stellung befinden.                        */

 asm     mov   ax,0x0013        ;/* normalen Modus 13h einstellen        */
 asm     int   0x10             ;

 asm     mov   dx,GC_INDEX      ;/* Auftrennung der Speicheradressen     */
 asm     mov   al,GC_GRAPH_MODE ;/* im Graphics-Mode-Register des        */
 asm     out   dx,al            ;/* Gaphics-Controllers ueber das        */
 asm     inc   dx               ;/* Bit #4 abschalten                    */
 asm     in    al,dx            ;
 asm     and   al,0xEF          ;
 asm     out   dx,al            ;
 asm     dec   dx               ;

 asm     mov   al,GC_MISCELL    ;/* und das Ganze noch einmal im         */
 asm     out   dx,al            ;/* Miscellanous-Register ueber das      */
 asm     inc   dx               ;/* Bit #1                               */
 asm     in    al,dx            ;
 asm     and   al,0xFD          ;
 asm     out   dx,al            ;

 asm     mov   dx,SC_INDEX      ;/* das Memory-Mode-Register im          */
 asm     mov   al,SC_MEM_MODE   ;/* Sequencer-Controlller dahingehend    */
 asm     out   dx,al            ;/* modifizieren, dasz keine Auftrennung */
 asm     inc   dx               ;/* der Adressen in verschiedene Bit-    */
 asm     in    al,dx            ;/* Planes mehr erfolgt und die aktuelle */
 asm     and   al,0xF7          ;/* Plane ueber das Bit-Mask-Register    */
 asm     or    al,4             ;/* festgelegt wird                      */
 asm     out   dx,al            ;

 asm	 mov   ax,VIO_SEG       ;/* alle vier Bit-Planes komplett mit    */;
 asm     mov   es,ax            ;/* dem Farbcode 00h fuellen und damit   */
 asm     xor   di,di            ;/* den Bildschirm zunaechst loeschen    */
 asm     mov   ax,di            ;
 asm     mov   cx,0x8000        ;
 asm     rep   stosw            ;

 asm	   mov   dx,CRTC_INDEX    ;/* Doppelwort-Modus ber das Bit 6      */
 asm	   mov   al,CC_UNDERLINE  ;/* im Underline-Register des CRT-       */
 asm	   out   dx,al            ;/* Controllers abstellen                */
 asm	   inc   dx	          ;
 asm	   in    al,dx            ;
 asm	   and   al,10111111b     ;
 asm	   out   dx,al            ;
 asm	   dec   dx               ;

 asm	   mov   al,CC_MODE_CTRL  ;/* ber das Bit 6 im Mode-Control-      */
 asm 	   out   dx,al            ;/* Register des CRT-Controllers jetzt   */
 asm	   inc   dx               ;/* vom Word- in den Byte-Modus wechseln */
 asm	   in    al,dx		  ;
 asm	   or    al,01000000b     ;
 asm	   out   dx,al            ;

 return		                  ;/* zurck zum Aufrufer		   */
}

/***************************************************************************/
/* SETPIX  : verleiht einem Punkt eine bestimmte Farbe                     */
/*-------------------------------------------------------------------------*/
/* Eingabe : SRC_X     = X-Koordinate des Punktes (0...319)                */
/*	     SRC_Y     = Y-Koordinate des Punktes (0...199)                */
/*	     SRC_FARBE = Farbe des Punktes (0...FFh)                       */
/* Ausgabe : keine						           */
/***************************************************************************/
void setpix( int SRC_X, int SRC_Y, unsigned char SRC_FARBE)
{
 asm	   mov   ax,PIXX          ;/* Offset in den Video-RAM berechnen    */
 asm	   mul   SRC_Y            ;/* und nach DI laden			   */
 asm	   mov   cx,SRC_X         ;
 asm	   mov   bx,cx		  ;
 asm	   shr   bx,1		  ;
 asm	   shr   bx,1		  ;
 asm	   add   ax,bx		  ;
 asm	   mov   di,ax		  ;

 asm	   and   cl,3             ;/* Bit-Maske fr die zu adressierende   */
 asm	   mov   ah,1             ;/* Map in AH berechnen                  */
 asm	   shl   ah,cl            ;


 asm	   mov   al,SC_MAP_MASK   ;/* nach AL die Registernummer           */
 asm	   mov   dx,SC_INDEX      ;/* Indexadresse des Sequencers laden    */
 asm	   out   dx,ax            ;/* Bit-Mask-Register laden              */

 asm	   mov   ax,VIO_SEG       ;/* ES auf den Video-RAM setzen          */
 asm	   mov   es,ax            ;
 asm	   mov   al,SRC_FARBE  	  ;/* Farbe des Punktes laden              */
 asm	   stosb                  ;/* und in die gew"hlte Bit-Map schreiben*/

 return		                  ;/* zurck zum Aufrufer		   */
}

/***************************************************************************/
/* GETPIX  : liefert die Farbe eines Punktes 				   */
/*-------------------------------------------------------------------------*/
/* Eingabe : SRC_X = X-Koordinate des Punktes				   */
/*           SRC_Y = Y-Koordinate des Punktes				   */
/* Ausgabe : SRC_FARBE des Punktes					   */
/***************************************************************************/

unsigned char getpix( int SRC_X, int SRC_Y )
{
 asm	   mov   ax,PIXX          ;/* Offset in den Video-RAM berechnen    */
 asm	   mul   SRC_Y	          ;/* und nach SI laden                    */
 asm	   mov   si,SRC_X	  ;
 asm	   mov   cx,si		  ;
 asm	   shr   si,1             ;
 asm	   shr   si,1             ;
 asm	   add   si,ax            ;

 asm	   and   cl,3             ;/* Bit-Maske fr die zu adressierende   */
 asm	   mov   ah,cl            ;/* Map in AH berechnen                  */
 asm	   mov   al,GC_READ_MAP   ;/* nach AL die Registernummer           */
 asm	   mov   dx,GC_INDEX      ;/* Indexadresse des Graphic-Ctrl laden  */
 asm	   out   dx,ax            ;/* Read-Map-Register laden              */

 asm	   mov   ax,VIO_SEG       ;/* ES auf den Video-RAM setzen          */
 asm	   mov   es,ax            ;
 asm	   mov   al,es:[si]       ;/* Farbe des Punktes laden              */

 return(_AL)	                  ;/* zurck zum Aufrufer                  */
}

/***************************************************************************/
/* SETPAGE: Waehlt die Seite aus, auf die bei Funktionsaufrufen von 	   */
/*          setpix und getpix zugegriffen werden soll                      */
/*-------------------------------------------------------------------------*/
/* Eingabe: SRC_PAGE = Nummer der Seite, auf die zugegriffen werden soll   */
/* Ausgabe: keine							   */
/***************************************************************************/
VOID setpage(BYTE SRC_PAGE)
{
 SRC_PAGE_AKTIV=SRC_PAGE	  ;/* Abspeichern der aktiven Seite        */
 asm	   mov   cl,SRC_PAGE	  ;

 asm	   mov   al,4             ;/* Hi-Byte der Segadr = Seite * 4 + A0h */
 asm	   mul   cl		  ;
 asm	   or    al,0xA0	  ;
 asm	   mov   cl,al		  ;

 asm       mov   ax,VIO_SEG       ;
 asm 	   mov   ah,cl		  ;
 VIO_SEG=_AX		          ;/* neue Segmentadresse merken           */

 return	   		          ;/* zurck zum Aufrufer                  */
}

/***************************************************************************/
/* SHOWPAGE: Bringt eine der Bildschirmseiten zur Anzeige                  */
/*-------------------------------------------------------------------------*/
/* Eingabe : SRC_PAGE = Nummer der Seite, die angezeigt werden soll        */
/* Ausgabe : keine                                                         */
/***************************************************************************/
VOID showpage( BYTE SRC_PAGE)
{
 asm	   mov   cl,SRC_PAGE	  ;/* Seite vorbereiten		 	   */
 asm	   mov   al,64            ;/* Hi-Byte des Offsets = Seite * 64     */
 asm	   mul   cl               ;
 asm	   mov   ah,al            ;/* Hi-Byte des Offsets nach AH          */

	   /*-- jetzt die neue Startadresse laden -----------------------  */

 asm	   mov   dx,CRTC_INDEX    ;/* CRT-Controller adressieren           */
 asm	   mov   al,CC_START_HI   ;/* Registernummer nach AL               */
 asm	   out   dx,ax            ;/* und ab gehts                         */

	   /*-- wieder auf den Start eines Bildschirmaufbaus warten -----  */

 asm	   mov   dx,VERT_RETRACE  ;/* zun"chst auf Ende des vertikalen     */

sp3:
 asm       in    al,dx            ;/* Rcklaufs warten                     */
 asm	   test  al,8             ;
 asm	   jne   sp3		  ;

sp4:
 asm       in    al,dx            ;/* dann auf den Start des Rcklaufs     */
 asm	   test  al,8             ;
 asm	   je    sp4              ;

return	                          ;/* zurck zum Aufrufer                  */
}


/***************************************************************************/
/* SEITE_AKTIV :  liefert Nummer der BS-Seite, die beschrieben werden kann */
/*-------------------------------------------------------------------------*/
/* Eingabe     : keine							   */
/* Ausgabe     : SRC_PAGE_AKTIV=Nummer der Seite                           */
/***************************************************************************/
BYTE Seite_aktiv(void)
{
 return (SRC_PAGE_AKTIV);
}


/***************************************************************************/
/* BLOCKMOVE: verschiebt eine BS-Seite im Video-RAM      		   */
/*-------------------------------------------------------------------------*/
/* Eingabe  : VONPAGE   - Quellseite					   */
/*            NACHPAGE  - Zielseite				           */
/*            VONSPALTE - StartSpalte auf der Quellseite                   */
/*            NACHSPALTE- StartSpalte auf der Zielseite                    */
/*            VONZEILE  - StartZeile auf der Quellseite                    */
/*            NACHSPALTE- StartZeile auf der Zielseite                     */
/*            ANZSPALTE - Anzahl der Spalten                               */
/*            ANZZEILE  - Anzahl der Zeilen                                */
/* AUSGABE  : keine							   */
/***************************************************************************/
void blockmove( BYTE VONPAGE, BYTE NACHPAGE, WORD VONSPALTE, WORD VONZEILE,
		WORD NACHSPALTE, WORD NACHZEILE, WORD ANZSPALTE,
		BYTE ANZZEILE)
{
 BYTE DUMMY;
 asm	   mov   dx,GC_INDEX      ;/* aktuellen Write-Mode holen und       */
 asm	   mov   al,GC_GRAPH_MODE ;/* Write-Mode 1 einstellen              */
 asm	   out   dx,al	          ;
 asm	   inc   dx		  ;
 asm	   in    al,dx            ;
 asm	   push  ax               ;/* aktuellen Modus auf Stack merken     */
 asm	   and   al,not 3	  ;
 asm	   or    al,1		  ;
 asm	   out   dx,al            ;
 asm	   push  ds		  ;
 asm       push  es		  ;

 asm	   mov   al,4             ;/* DS auf Anfang der VON-Seite          */
 asm	   mov   cl,VONPAGE       ;
 asm	   mul   cl               ;
 asm	   or    al,0x0A0	  ;
 asm	   xchg  ah,al		  ;
 asm	   mov   ds,ax		  ;

 asm	   mov   al,4             ;/* ES auf Anfang der NACH-Seite         */
 asm	   mov   cl,NACHPAGE      ;
 asm	   mul   cl               ;
 asm	   or    al,0xA0	  ;
 asm	   xchg  ah,al		  ;
 asm	   mov   es,ax            ;

 asm	   mov   ax,VONZEILE	  ;/* SI auf Startposition VON             */
 asm	   mov   si,VONSPALTE	  ;
 asm	   shr   si,1		  ;
 asm	   shr   si,1             ;
 asm	   add   si,ax            ;

 asm	   mov   ax,NACHZEILE	  ;/* DI auf Zielposition NACH 		   */
 asm	   mov   di,NACHSPALTE    ;
 asm	   shr   di,1             ;
 asm	   shr   di,1             ;
 asm	   add   di,ax            ;

	   ANZSPALTE=ANZSPALTE/4  ;
	   DUMMY = (BYTE) ANZSPALTE;
 asm	   mov   dl,DUMMY         ;/* DL = Bytes                           */
 asm	   mov   dh,ANZZEILE      ;/* DH = Zeilen                          */

 asm	   mov   bx,80            ;/* BX als Offset zur naechsten Zeile    */
 asm	   sub   bl,dl            ;
 asm	   xor   ch,ch            ;/* Hi-Byte des Counters ist immer 0     */


 asm	   push  dx               ;/* DX auf dem Stack sichern		   */
 asm	   mov   dx,964           ;/* Zugriff auf alle Bit-Planes sichern  */
 asm	   mov   ah,0x0F	  ;
 asm	   mov   al,2             ;
 asm	   out   dx,ax            ;
 asm	   pop   dx               ;/* DX zurckholen                       */

/*	   ;-- Kopierroutine fr alle vier Bit-Planes, also ohne Berck-
	   ;-- sichtigung des Hintergrundes             */

mt1:
 asm	   mov   cl,dl            ;/* Anzahl Bytes nach CL                 */

 asm	   rep movsb              ;/* Zeile kopieren                       */
 asm	   add   di,bx            ;/* DI in n"chste Zeile                  */
 asm	   add   si,bx            ;/* und auch SI                          */
 asm	   dec   dh               ;/* noch eine Zeile?                     */
 asm	   jne   mt1              ;/* Ja ---> weiter                       */

 asm	   pop   es               ;
 asm       pop   ds               ;
 asm       mov   dx,GC_INDEX      ;/* wieder den alten Write-Mode ein-     */
 asm	   pop   ax               ;/* stellen                              */
 asm	   mov   ah,al            ;
 asm	   mov   al,GC_GRAPH_MODE ;
 asm	   out   dx,ax            ;

 return;
}


/***************************************************************************/
/* Zeichen_Ausgabe : Ausgabe eines Textzeichens auf den ArbeitsBS          */
/*-------------------------------------------------------------------------*/
/* Eingabe : Textx    = X-Koordinate					   */
/*           Textx    = Y-Koordinate					   */
/*           Texth    = Texth"he (1x ,2x ,3x)				   */
/*           Bitmaske = Datenzeiger					   */
/* Ausgabe : keine							   */
/***************************************************************************/
void Zeichen_Ausgabe (int Textx, int Texty,int Texth,
		      unsigned char *Bitmaske)
{
 int x,y,h,z;
 y=Texth;
 if (y==0) Texth=6;
 if (y==1) Texth=8;
 z=0;
 for (y=Texty;y<(Texty+Texth);y++)
 {
  for (x=Textx;x<(Textx+Texth);x++)
  {
    if ((x>0)&&(x<319))
    {
     if ((y>0)&&(y<199))
     {
       setpix(x,y,(char)Bitmaske[z]);
     }
    }
   z++;
  }
 }
 return;
}

/***************************************************************************/
/* Text_Ausgabe : Ausgabe eines Stringes                                   */
/*-------------------------------------------------------------------------*/
/* Eingabe : Text   = Zeiger auf Textstring				   */
/*           Laenge = L"nge des Textes	   (1..40)			   */
/*           Hoehe  = H"he der Textzeichen (0..2)			   */
/*	     TextX  = X-Koordinate					   */
/*           TextY  = Y-Koordinate					   */
/* Ausgabe : keine							   */
/***************************************************************************/
void Text_Ausgabe (unsigned char *Text,
		   unsigned char Laenge, unsigned char Hoehe,
		   int TextX, int TextY)
{
unsigned char Textl,Texta,Texth;		/* Z"hlvariable            */
int Textx,Texty,x,y;
unsigned char* Bitmaske;

/*-- šberprfung der Parameter						   */
 if ((Laenge<1)||(Laenge>40)) return;
 if (Hoehe>2)   Hoehe=0;

if (Hoehe>0) goto Textmarke1;

/*-- Text ausgeben							   */
 for (Textl=0;Textl<Laenge;Textl++)
 {
	Texta=(unsigned char)*(Text+Textl);
/*-- Bitmaske holen							   */
	Bitmaske=(unsigned char*)TBit;
	for (x=0;x<sizeof(TZeichen);x++)
	{
	  if (Texta==TZeichen[x]) Bitmaske=(unsigned char*)(TBit+(x*36));
	}
	Zeichen_Ausgabe(TextX,TextY,Hoehe,Bitmaske);
	TextX=TextX+6;
 }
 return;
Textmarke1:

/*-- Text ausgeben							   */
 for (Textl=0;Textl<Laenge;Textl++)
 {
	Texta=(unsigned char)*(Text+Textl);
/*-- Bitmaske holen							   */
	Bitmaske=(unsigned char*)TBit8;
	for (x=0;x<sizeof(TZeichen);x++)
	{
	  if (Texta==TZeichen[x]) Bitmaske=(unsigned char*)(TBit8+(x*64));
	}
	Zeichen_Ausgabe(TextX,TextY,Hoehe,Bitmaske);
	TextX=TextX+8;
 }

 return;
}

/***************************************************************************/
/* Zeichenladen : l"dt die Zeichens"tze in den Speicher                    */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine              					   */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void Zeichenladen (void)
{
 Charladen2("6x6.tga",TBit);
 Charladen1("8x8.tga",TBit8);
 return;
}
