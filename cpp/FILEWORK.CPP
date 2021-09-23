/********************************************************************/
/* F I L E W O R K . C P P                                          */
/*------------------------------------------------------------------*/
/* Aufgabe       :  Quellcode fuer alle Datei-Routinen fuer         */
/*                  K U N O   -   D E R   R I T T E R               */
/*------------------------------------------------------------------*/
/* Autor         :  Ralf Guder                                      */
/* Arbeitsbeginn :  30.01.96                                        */
/* Arbeitsende   :                                                  */
/* letztes Update:  04.03.96                                        */
/*------------------------------------------------------------------*/
/* Funktionen                                                       */
/* ----------                                                       */
/********************************************************************/

/*== INCLUDE-Dateien ===============================================*/
#include <iostream.h>
#include <alloc.h>
#include <stdio.h>
#include <graphics.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <dos.h>
#include "kuno.h"

/*== externe Funktionen ============================================*/
/*-- Modul "VGAWORK.CPP" -------------------------------------------*/
void setpix(int,int,unsigned char)      ;/* setzten eines Pixels    */
unsigned char Seite_aktiv( void )       ;/* liefert die derzeit     */
					 /*  aktive Seite           */

/*-- Modul "MAINFILE.CPP" ------------------------------------------*/
void huge error(const char *)           ;/* Fehlerfunktion          */

/*== Variablen fuer das ganze Modul ================================*/
int x,y                                 ;
char *DAC_Block                         ;/* Zeiger auf die Farb-    */
					 /*  palette der aktiven    */
					 /*  Bildschirmseite        */
char  DAC_Block0[3*256]                 ;/* Puffer fuer Farbpalette */
char  DAC_Block1[3*256]                 ;/* Puffer fuer Farbpalette */
char  DAC_Block2[3*256]                 ;/* Puffer fuer Farbpalette */
char  DAC_Block3[3*256]                 ;/* Puffer fuer Farbpalette */
unsigned char Blend=5                   ;/* Parameter fuer delay()  */
char *Levelname[]={
     "level0.kun","level1.kun",
     "level2.kun","level3.kun",
     "level4.kun","level5.kun",
     "level6.kun","level7.kun",
     "level8.kun","level9.kun"}        ;

/********************************************************************/
/* DAC_SETZEN: setzt die Farbpalette des Bildschirmes ueber das     */
/*             VGA-BIOS                                             */
/*------------------------------------------------------------------*/
/* Eingabe   : x      = Nummer des ersten Farbregisters (0...255)   */
/*             Anzahl = Anzahl der zu ladenen Farbregister (0...255)*/
/* Ausgabe   : keine                                                */
/********************************************************************/
void DAC_setzen(int x,int Anzahl)
{
 if (Seite_aktiv()==0) DAC_Block=DAC_Block0;
 if (Seite_aktiv()==1) DAC_Block=DAC_Block1;
 if (Seite_aktiv()==2) DAC_Block=DAC_Block2;
 if (Seite_aktiv()==3) DAC_Block=DAC_Block3;

/*-- Variablen -----------------------------------------------------*/
 int BlockOff = FP_OFF (DAC_Block)      ;
 int BlockSeg = FP_SEG (DAC_Block)      ;

 _AH = 0x10                             ;
 _AL = 0x12                             ;
 _BX = x                                ;
 _CX = Anzahl                           ;
 _ES = BlockSeg                         ;
 _DX = BlockOff                         ;
 geninterrupt(0x10)                     ;

 return                                 ;
}

/********************************************************************/
/* TGA_LADEN:  laedt ein VGA-Bild in die aktive Bildschirmseite     */
/*------------------------------------------------------------------*/
/* Eingabe  :  Filename = Name des gespeicherten Bildes             */
/* Ausgabe  :  keine                                                */
/****************************************************************** */
void TGA_laden(char *Filename)
{
/*-- Variablen -----------------------------------------------------*/
 FILE *fp                               ;/* File-Zeiger             */
 char rot,gruen,blau                    ;/* RGB-Farben              */
 char dummy                             ;/* Leer-Variable           */
 char xlow,xhigh                        ;/* x-Koordinate geteilt    */
 char ylow,yhigh                        ;/* y-Koordinate geteilt    */
 int xmax,ymax                          ;/* x,y-Koordinaten gesammt */
 int Adr                                ;/* berechnete BS-Adresse   */
 int Punkt                              ;/* geladener Farb-Pixel    */

/*-- Oeffnen des gewuenschten Bild-File ----------------------------*/
 fp=fopen(Filename,"rb")                ;
 if (fp == NULL)
 {                                       /* Meldung wenn Fehler     */
  error("Bilddatei nicht gefunden!\n")  ;
 }

 if (Seite_aktiv()==0) DAC_Block=DAC_Block0;
 if (Seite_aktiv()==1) DAC_Block=DAC_Block1;
 if (Seite_aktiv()==2) DAC_Block=DAC_Block2;
 if (Seite_aktiv()==3) DAC_Block=DAC_Block3;


/*-- Dateikopf des Files Åbergehen ---------------------------------*/
 for (x=0;x<12;x++)
 {
  dummy=fgetc(fp)                     ;
 }
/*-- Koordinatenanteile laden --------------------------------------*/
 xlow=fgetc(fp)                       ;/* Low  X-Koordinate         */
 xhigh=fgetc(fp)                      ;/* High X-Koordinate         */
 ylow=fgetc(fp)                       ;/* Low  Y-Koordinate         */
 yhigh=fgetc(fp)                      ;/* High Y-Koordinate         */
 dummy=fgetc(fp)                      ;/* Zwischenraum zur DAC-     */
 dummy=fgetc(fp)                      ;/*  Palette Åbergehen        */
 dummy=dummy+1                        ;/* Fehlermeldung beim        */
				       /*  Compilieren ausschlie·en */

/*-- DAC_Palette einladen und berechnen ----------------------------*/
 for (x=0;x<3*256;x=x+3)
 {
  blau=fgetc(fp)	;
  asm 	SHR blau,2	;
  gruen=fgetc(fp)	;
  asm 	SHR gruen,2	;
  rot=fgetc(fp)		;
  asm 	SHR rot,2	;
  DAC_Block[x]=rot	;
  DAC_Block[x+1]=gruen	;
  DAC_Block[x+2]=blau	;
 }


/*-- Umwandeln Low & High in komplette Koordinaten                         */
 _AL=xlow;
 _AH=xhigh;
 xmax=_AX;
 _AL=ylow;
 _AH=yhigh;
 ymax=_AX;
/* ymax++;*/
/*   xmax++;*/

/*-- Pixel der Bilddatei auf die Bildschirmseite laden                     */
 for (int y=ymax-1;y>-1;y--)
 {
  for (x=0;x<xmax;x++)
  {
   Punkt=fgetc(fp);
   setpix(x,y,Punkt);
  }
 }

/*-- Bilddatei schlie·en                                                   */
fclose(fp);
return;
}
/********************************************************************/
/* TGA_SCHIEBEN:  Verschiebt ein Bild in die aktive Bildschirmseit  */
/*------------------------------------------------------------------*/
/* Eingabe  :  Speicher = Name des Speichers mit dem Bildes         */
/* Ausgabe  :  keine                                                */
/****************************************************************** */
void TGA_SCHIEBEN(char *Speicher)
{
/*-- Variablen -----------------------------------------------------*/
 char rot,gruen,blau                    ;/* RGB-Farben              */
 char dummy                             ;/* Leer-Variable           */
 char xlow,xhigh                        ;/* x-Koordinate geteilt    */
 char ylow,yhigh                        ;/* y-Koordinate geteilt    */
 int xmax,ymax                          ;/* x,y-Koordinaten gesammt */
 int Adr                                ;/* berechnete BS-Adresse   */
 int Punkt                              ;/* geladener Farb-Pixel    */

/*-- Oeffnen der gewuenschten Farbpalette --------------------------*/

 if (Seite_aktiv()==0) DAC_Block=DAC_Block0;
 if (Seite_aktiv()==1) DAC_Block=DAC_Block1;
 if (Seite_aktiv()==2) DAC_Block=DAC_Block2;
 if (Seite_aktiv()==3) DAC_Block=DAC_Block3;


/*-- Koordinatenanteile laden --------------------------------------*/
 xlow=Speicher[0]                     ;/* Low  X-Koordinate         */
 xhigh=Speicher[1]                    ;/* High X-Koordinate         */
 ylow=Speicher[2]                     ;/* Low  Y-Koordinate         */
 yhigh=Speicher[3]                    ;/* High Y-Koordinate         */
 Speicher=Speicher+4;

/*-- DAC_Palette einladen und berechnen ----------------------------*/
 for (x=0;x<3*256;x=x+3)
 {
  rot=Speicher[0]	;
  Speicher++		;
  gruen=Speicher[0]	;
  Speicher++		;
  blau=Speicher[0]	;
  Speicher++		;
  DAC_Block[x]=rot	;
  DAC_Block[x+1]=gruen	;
  DAC_Block[x+2]=blau	;
 }


/*-- Umwandeln Low & High in komplette Koordinaten                         */
 _AL=xlow;
 _AH=xhigh;
 xmax=_AX;
 _AL=ylow;
 _AH=yhigh;
 ymax=_AX;
/* ymax++;*/
/*   xmax++;*/

/*-- Pixel der Bilddatei auf die Bildschirmseite laden                     */
 for (int y=ymax;y>0;y--)
 {
  for (x=0;x<xmax;x++)
  {
   Punkt=Speicher[0];
   Speicher++;
   setpix(x,y,Punkt);
  }
 }

/*-- Bilddatei schlie·en                                                   */
return;
}

/********************************************************************/
/* TGA_LADEN_SP:  laedt ein VGA-Bild in einen Speicher              */
/*------------------------------------------------------------------*/
/* Eingabe  :  Filename = Name des gespeicherten Bildes             */
/*             Speicher = Zeiger auf den BildPuffer                 */
/* Ausgabe  :  keine                                                */
/****************************************************************** */
void TGA_LADEN_SP(char *Filename, char* Speicher)
{
/*-- Variablen -----------------------------------------------------*/
 FILE *fp                               ;/* File-Zeiger             */
 char rot,gruen,blau                    ;/* RGB-Farben              */
 char dummy                             ;/* Leer-Variable           */
 char xlow,xhigh                        ;/* x-Koordinate geteilt    */
 char ylow,yhigh                        ;/* y-Koordinate geteilt    */
 int xmax,ymax                          ;/* x,y-Koordinaten gesammt */
 int Adr                                ;/* berechnete BS-Adresse   */
 int Punkt                              ;/* geladener Farb-Pixel    */

/*-- Oeffnen des gewuenschten Bild-File ----------------------------*/
 fp=fopen(Filename,"rb")                ;
 if (fp == NULL)
 {                                       /* Meldung wenn Fehler     */
  error("Bilddatei nicht gefunden!\n")  ;
 }

 DAC_Block=Speicher+4;


/*-- Dateikopf des Files Åbergehen ---------------------------------*/
 for (x=0;x<12;x++)
 {
  dummy=fgetc(fp)                     ;
 }
/*-- Koordinatenanteile laden --------------------------------------*/
 xlow=fgetc(fp)                       ;/* Low  X-Koordinate         */
 xhigh=fgetc(fp)                      ;/* High X-Koordinate         */
 ylow=fgetc(fp)                       ;/* Low  Y-Koordinate         */
 yhigh=fgetc(fp)                      ;/* High Y-Koordinate         */
 dummy=fgetc(fp)                      ;/* Zwischenraum zur DAC-     */
 dummy=fgetc(fp)                      ;/*  Palette Åbergehen        */
 dummy=dummy+1                        ;/* Fehlermeldung beim        */
				       /*  Compilieren ausschlie·en */
 Speicher[0]=xlow;
 Speicher[1]=xhigh;
 Speicher[2]=ylow;
 Speicher[3]=yhigh;

/*-- DAC_Palette einladen und berechnen ----------------------------*/
 for (x=0;x<3*256;x=x+3)
 {
  blau=fgetc(fp)	;
  asm 	SHR blau,2	;
  gruen=fgetc(fp)	;
  asm 	SHR gruen,2	;
  rot=fgetc(fp)		;
  asm 	SHR rot,2	;
  DAC_Block[x]=rot	;
  DAC_Block[x+1]=gruen	;
  DAC_Block[x+2]=blau	;
 }


/*-- Umwandeln Low & High in komplette Koordinaten                         */
 _AL=xlow;
 _AH=xhigh;
 xmax=_AX;
 _AL=ylow;
 _AH=yhigh;
 ymax=_AX;

/*-- Speicher atualisieren ------------------------------------------------*/
 Speicher=Speicher+4+(3*256);

/*-- Pixel der Bilddatei in den Pufferspeicher laden                       */
 for (int y=ymax;y>0;y--)
 {
  for (x=0;x<xmax;x++)
  {
   Punkt=fgetc(fp);
   Speicher[0]=Punkt;
   Speicher++;
  }
 }

/*-- Bilddatei schlie·en                                                   */
fclose(fp);
return;
}

/***************************************************************************/
/* BILD_AUFBLENDEN:  einblenden des TGA-Bildes				   */
/*-------------------------------------------------------------------------*/
/* Eingabe        :  keine 						   */
/* Ausgabe        :  keine                                                 */
/* Info           :  die ben"tigte Farbpalette ist im Array DAC_Block[]    */
/*		     abgespeichert					   */
/***************************************************************************/
void Bild_aufblenden(void)
{
/*-- Variablendefinition                                                   */
 char Anteil;					/* Z"hlvariable Farbanteil */


 if (Seite_aktiv()==0) DAC_Block=DAC_Block0;
 if (Seite_aktiv()==1) DAC_Block=DAC_Block1;
 if (Seite_aktiv()==2) DAC_Block=DAC_Block2;
 if (Seite_aktiv()==3) DAC_Block=DAC_Block3;

/*-- Farbintensit"t von wei· bis schwarz                                   */
 for (x=0;x<63;x++)
 {
/*-- fÅr alle Farben(RGB)                                                  */
  for (y=0;y<3*256;y=y+3)
  {
/*-- DAC_Farbregister der Farbe"Y/3" activieren                            */
   outportb(0x3C8,y/3);
   disable;					/* Interrupts sperren      */
/*-- Rotanteil ermitteln und ausgeben                                      */
   if (x<DAC_Block[y])
    Anteil=x;
   else
    Anteil=DAC_Block[y];
   outportb(0x3C9,Anteil);
/*-- GrÅnanteil ermitteln und ausgeben					   */
   if (x<DAC_Block[y+1])
    Anteil=x;
   else
    Anteil=DAC_Block[y+1];
   outportb(0x3C9,Anteil);
/*-- Blauanteil ermitteln und ausgeben					   */
   if (x<DAC_Block[y+2])
    Anteil=x;
   else
    Anteil=DAC_Block[y+2];
   outportb(0x3C9,Anteil);

   enable;					/* Interrupts freigeben    */
  }
  delay(Blend);					/* Verz"gerung             */
 }
 return;
}


/***************************************************************************/
/* BILD_AUSBLENDEN : blendet das Bild der aktuellen Seite aus              */
/*-------------------------------------------------------------------------*/
/* Eingabe         : keine						   */
/* Ausgabe         : keine						   */
/* Info            : Farbpalette ist in DAC_Block[] 			   */
/***************************************************************************/
void Bild_ausblenden(void)
{
/*-- Variablendefinition                                                   */
 char Anteil;					/* Farbanteil              */

 if (Seite_aktiv()==0) DAC_Block=DAC_Block0;
 if (Seite_aktiv()==1) DAC_Block=DAC_Block1;
 if (Seite_aktiv()==2) DAC_Block=DAC_Block2;
 if (Seite_aktiv()==3) DAC_Block=DAC_Block3;

/*-- Farbintensit"t von wei· bis schwarz                                   */
 for (x=63;x>0;x--)
 {
/*-- fÅr alle Farben(RGB)                                                  */
  for (y=0;y<3*256;y=y+3)
  {
/*-- DAC_Farbregister der Farbe"Y/3" activieren                            */
   outportb(0x3C8,y/3);
   disable;					/* Interrupts sperren      */
/*-- Rotanteil ermitteln und ausgeben                                      */
   if (x<DAC_Block[y])
    Anteil=x;
   else
    Anteil=DAC_Block[y];
   outportb(0x3C9,Anteil);
/*-- GrÅnanteil ermitteln und ausgeben					   */
   if (x<DAC_Block[y+1])
    Anteil=x;
   else
    Anteil=DAC_Block[y+1];
   outportb(0x3C9,Anteil);
/*-- Blauanteil ermitteln und ausgeben                                     */
   if (x<DAC_Block[y+2])
    Anteil=x;
   else
    Anteil=DAC_Block[y+2];
   outportb(0x3C9,Anteil);

   enable;                                      /* Interrupts freigeben    */
  }
  delay(Blend);					/* Verz"gerung 	           */
 }
 return;
}

/***************************************************************************/
/* Levelladen :  laedt ein Kuno-Level				           */
/*-------------------------------------------------------------------------*/
/* Eingabe  :  Nr     = Nummer des Levels				   */
/*	       Name   = Name des Levelspeichers				   */
/*	       Objekt = Name des Objektspeichers			   */
/* Ausgabe  :  keine                                                       */
/***************************************************************************/

void Levelladen(int Nr,unsigned char *Name,unsigned int *Objekt)
{
/*-- Variablen                                                             */
 FILE *fp;					/* File-Zeiger             */
 char Punkt;

/*-- ôffnen des gewÅnschten Level-Files                                    */
 fp=fopen(Levelname[Nr],"rb");
 if (fp == NULL)
 {                                              /* Meldung wenn Fehler     */
  return;
 }

/*-- Leveldaten einladen			                           */
 for (int y=0;y<31*19*9;y++)
 {
   Punkt=fgetc(fp);
   (unsigned char)Name[y]=Punkt;
 }

/*-- Objektdaten einladen			                           */
 for (y=0;y<4*19*9;y++)
 {
   Punkt=fgetc(fp);
   (unsigned char)Objekt[y]=Punkt;
 }

/*-- Bilddatei schlie·en                                                   */
fclose(fp);
return;
}

/***************************************************************************/
/* Levelspeichern :  saved ein Kuno-Level				           */
/*-------------------------------------------------------------------------*/
/* Eingabe  :  Nr     = Nummer des Levels				   */
/*	       Name   = Name des Levelspeichers				   */
/*	       Objekt = Name des Objektspeichers			   */
/* Ausgabe  :  keine                                                       */
/***************************************************************************/

void Levelspeichern(int Nr,unsigned char *Name,unsigned int *Objekt)
{
/*-- Variablen                                                             */
 FILE *fp;					/* File-Zeiger             */
 char Punkt;

/*-- ôffnen des gewÅnschten Level-Files                                    */
 fp=fopen(Levelname[Nr],"wb");
 if (fp == NULL)
 {                                              /* Meldung wenn Fehler     */
  return;
 }

/*-- Leveldaten einladen			                           */
 for (int y=0;y<31*19*9;y++)
 {
   Punkt=(unsigned char)Name[y];
   fputc(Punkt,fp);
 }

/*-- Objektdaten einladen			                           */
 for (y=0;y<4*19*9;y++)
 {
  Punkt=(unsigned char)Objekt[y];
  fputc(Punkt,fp);
 }

/*-- Bilddatei schlie·en                                                   */
fclose(fp);
return;
}

/***************************************************************************/
/* Charladen : laedt die Textur 8x8 in einen Speicher                      */
/*-------------------------------------------------------------------------*/
/* Eingabe : Filename = Name der Datei					   */
/*           Name     = Name des Texturspeichers                           */
/*           Anzahl   = Anzahl der zu ladenden Texturen                    */
/* Ausgabe : keine                                                         */
/*-------------------------------------------------------------------------*/
/* INFO: Die Datei mu· eine Gr"·e von 296 * 8 Bildpunkten haben            */
/***************************************************************************/
VOID Charladen ( PBYTE Filename,PBYTE Name,BYTE Anzahl)
{
/*-- Variablen                                                             */
 FILE *fp;					/* File-Zeiger             */
 char dummy;                                    /* Leer-Variable           */
 int Punkt;					/* geladener Farb-Pixel    */

/*-- ôffnen des gewÅnschten Bild-File                                      */
 fp=fopen(Filename,"rb");
 if (fp == NULL)
 {                                              /* Meldung wenn Fehler     */
  return;					/* zurÅck ohne Laden       */
 }

/*-- Dateikopf des Files Åbergehen                                         */
 for (x=0;x<12;x++)
 {
  dummy=fgetc(fp);
 }
/*-- Koordinatenanteile laden und Åbergehen                                */
 dummy=fgetc(fp) ;				/* Low  X-Koordinate       */
 dummy=fgetc(fp);				/* High X-Koordinate       */
 dummy=fgetc(fp) ;				/* Low  Y-Koordinate       */
 dummy=fgetc(fp);				/* High Y-Koordinate       */
 dummy=fgetc(fp);                               /* Zwischenraum zur DAC-   */
 dummy=fgetc(fp);                               /*  Palette Åbergehen      */
/*-- Fehlermeldung beim Compilieren ausschlie·en                           */
 dummy=dummy+1  ;



/*-- DAC-Block Åbergehen						   */
 for (x=0;x<3*256;x=x+3)
 {
  dummy=fgetc(fp)	;
  dummy=fgetc(fp)	;
  dummy=fgetc(fp)		;
 }
/*-- Pixel der Bilddatei in den Texturpuffer laden                         */
 int z=0;
 for (int y=8;y>0;y--)
 {
  for (z=0;z<Anzahl;z++)
  {
   for (x=0;x<8;x++)
   {
    Punkt=fgetc(fp);
    Name[(8*y-8)+(64*z)+x]=Punkt;
   }
  }
 }

/*-- Bilddatei schlie·en                                                   */
fclose(fp);
return;
}


/***************************************************************************/
/* Spriteladen : l"dt ein Sprite in den Spritespeicher                     */
/*-------------------------------------------------------------------------*/
/* Eingabe : Filename = Name der Spritedatei                               */
/*           Name     = Name des Spries					   */
/***************************************************************************/
void Spriteladen(unsigned char *Filename, unsigned char* Name)
{
/*-- Variablen                                                             */
 FILE *fp;					/* File-Zeiger             */
 char dummy;                                    /* Leer-Variable           */
 int Punkt;					/* geladener Farb-Pixel    */

/*-- ôffnen des gewÅnschten Bild-File                                      */
 fp=fopen(Filename,"rb");
 if (fp == NULL)
 {                                              /* Meldung wenn Fehler     */
  return;
 }


/*-- Dateikopf des Files Åbergehen                                         */
 for (x=0;x<12;x++)
 {
  dummy=fgetc(fp);
 }
/*-- Koordinatenanteile laden                                              */
 dummy=fgetc(fp) ;				/* Low  X-Koordinate       */
 dummy=fgetc(fp);				/* High X-Koordinate       */
 dummy=fgetc(fp) ;				/* Low  Y-Koordinate       */
 dummy=fgetc(fp);				/* High Y-Koordinate       */
 dummy=fgetc(fp);                               /* Zwischenraum zur DAC-   */
 dummy=fgetc(fp);                               /*  Palette Åbergehen      */
/*-- Fehlermeldung beim Compilieren ausschlie·en                           */
 dummy=dummy+1  ;

/*-- DAC_Palette einladen und berechnen                                    */
 for (x=0;x<3*256;x=x+3)
 {
  dummy=fgetc(fp)	;
  dummy=fgetc(fp)	;
  dummy=fgetc(fp)	;
 }


/*-- Pixel der Bilddatei auf die Bildschirmseite laden                     */
 for (int y=21;y>0;y--)
 {
  for (x=0;x<24;x++)
  {
   Punkt=fgetc(fp);
   Name[x+(24*y-24)]=Punkt;
  }
 }

/*-- Bilddatei schlie·en                                                   */
fclose(fp);
return;
}

/***************************************************************************/
/* Charladen2 : l"dt die Textur 6x6 in einen Speicher                       */
/*-------------------------------------------------------------------------*/
/* Eingabe : Filename = Name der Datei					   */
/*           Name     = Name des Texturspeichers                           */
/* Ausgabe : keine                                                         */
/*-------------------------------------------------------------------------*/
/* INFO: Die Datei mu· eine Gr"·e von 222 * 6 Bildpunkten haben            */
/***************************************************************************/
void Charladen2 (unsigned char *Filename,unsigned char *Name)
{
/*-- Variablen                                                             */
 FILE *fp;					/* File-Zeiger             */
 char dummy;                                    /* Leer-Variable           */
 int Punkt;					/* geladener Farb-Pixel    */

/*-- ôffnen des gewÅnschten Bild-File                                      */
 fp=fopen(Filename,"rb");
 if (fp == NULL)
 {                                              /* Meldung wenn Fehler     */
  return;					/* zurÅck ohne Laden       */
 }

/*-- Dateikopf des Files Åbergehen                                         */
 for (x=0;x<12;x++)
 {
  dummy=fgetc(fp);
 }
/*-- Koordinatenanteile laden und Åbergehen                                */
 dummy=fgetc(fp) ;				/* Low  X-Koordinate       */
 dummy=fgetc(fp);				/* High X-Koordinate       */
 dummy=fgetc(fp) ;				/* Low  Y-Koordinate       */
 dummy=fgetc(fp);				/* High Y-Koordinate       */
 dummy=fgetc(fp);                               /* Zwischenraum zur DAC-   */
 dummy=fgetc(fp);                               /*  Palette Åbergehen      */
/*-- Fehlermeldung beim Compilieren ausschlie·en                           */
 dummy=dummy+1  ;



/*-- DAC-Block Åbergehen						   */
 for (x=0;x<3*256;x=x+3)
 {
  dummy=fgetc(fp)	;
  dummy=fgetc(fp)	;
  dummy=fgetc(fp)		;
 }
/*-- Pixel der Bilddatei in den Texturpuffer laden                         */
 int z=0;
 for (int y=6;y>0;y--)
 {
  for (z=0;z<37;z++)
  {
   for (x=0;x<6;x++)
   {
    Punkt=fgetc(fp);
    Name[(6*y-6)+(36*z)+x]=Punkt;
   }
  }
 }

/*-- Bilddatei schlie·en                                                   */
fclose(fp);
return;
}

/***************************************************************************/
/* Charladen1 : l"dt den Zeichensatz 8x8 in einen Speicher                 */
/*-------------------------------------------------------------------------*/
/* Eingabe : Filename = Name der Datei					   */
/*           Name     = Name des Texturspeichers                           */
/* Ausgabe : keine                                                         */
/*-------------------------------------------------------------------------*/
/* INFO: Die Datei mu· eine Gr"·e von 488 * 8 Bildpunkten haben            */
/***************************************************************************/
void Charladen1 (unsigned char *Filename,unsigned char *Name)
{
/*-- Variablen                                                             */
 FILE *fp;					/* File-Zeiger             */
 char dummy;                                    /* Leer-Variable           */
 int Punkt;					/* geladener Farb-Pixel    */

/*-- ôffnen des gewÅnschten Bild-File                                      */
 fp=fopen(Filename,"rb");
 if (fp == NULL)
 {                                              /* Meldung wenn Fehler     */
  return;					/* zurÅck ohne Laden       */
 }

/*-- Dateikopf des Files Åbergehen                                         */
 for (x=0;x<12;x++)
 {
  dummy=fgetc(fp);
 }
/*-- Koordinatenanteile laden und Åbergehen                                */
 dummy=fgetc(fp) ;				/* Low  X-Koordinate       */
 dummy=fgetc(fp);				/* High X-Koordinate       */
 dummy=fgetc(fp) ;				/* Low  Y-Koordinate       */
 dummy=fgetc(fp);				/* High Y-Koordinate       */
 dummy=fgetc(fp);                               /* Zwischenraum zur DAC-   */
 dummy=fgetc(fp);                               /*  Palette Åbergehen      */
/*-- Fehlermeldung beim Compilieren ausschlie·en                           */
 dummy=dummy+1  ;



/*-- DAC-Block Åbergehen						   */
 for (x=0;x<3*256;x=x+3)
 {
  dummy=fgetc(fp)	;
  dummy=fgetc(fp)	;
  dummy=fgetc(fp)		;
 }
/*-- Pixel der Bilddatei in den Texturpuffer laden                         */
 int z=0;
 for (int y=8;y>0;y--)
 {
  for (z=0;z<67;z++)
  {
   for (x=0;x<8;x++)
   {
    Punkt=fgetc(fp);
    Name[(8*y-8)+(64*z)+x]=Punkt;
   }
  }
 }

/*-- Bilddatei schlie·en                                                   */
fclose(fp);
return;
}
