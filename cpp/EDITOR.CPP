/********************************************************************/
/* E D I T O R . C P P                                              */
/*------------------------------------------------------------------*/
/* Aufgabe       :  Quellcode fuer Leveleditor fuer                 */
/*                  K U N O   -   D E R   R I T T E R               */
/*------------------------------------------------------------------*/
/* Autor         :  Ralf Guder                                      */
/* Arbeitsbeginn :  27.02.96                                        */
/* Arbeitsende   :                                                  */
/* letztes Update:  28.03.96                                        */
/*------------------------------------------------------------------*/
/* Funktionen                                                       */
/* ----------                                                       */
/*------------------------------------------------------------------*/
/* Info                                                             */
/* ----                                                             */
/* Bildschirmseite 0 und 1     - Darstellungsseiten                 */
/* Bildschirmseite 2           - Hintergrundseite                   */
/********************************************************************/

/*-- INCLUDE-Dateien -----------------------------------------------*/
#define kuno_c


#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include <graphics.h>
#include <conio.h>
#include <dos.h>
#include <process.h>
#include <alloc.h>
#include "LEVEL.CPP"                   /* enthaelt die Level        */
#include "SPRITEDA.CPP"                /* enthaelt die Daten der    */
/*#include "SPRITED2.CPP"		       /*  Sprites des Spieles      */
#include "CURSOR.CPP"                  /* enthaelt die Daten des    */
				       /*  Spritecursors            */
#include "TYPES.H"                     /* Typendeklarationen        */
#include "kuno.h"

/*-- Definitionen fÅr den Tastatur-Handler -------------------------*/
#ifdef __cplusplus
    #define __CPPARGS ...
#else
    #define __CPPARGS
#endif


/*== externe Funktionen =============================================/
/*   Modul "SPRITE.CPP"                                             */
INT  Spritekollision(VOID)            ;/* beruehrtes Sprite         */
INT  Tuerkollision  (VOID)            ;/* beruehrte Tuer            */
VOID SpriteKoord (INT, INT, INT)      ;/* Uebergabe der Spritepos.  */
VOID SpriteDeactiv (INT)              ;/* Sprite-Deaktivierung      */
VOID SpriteDaten (INT, PBYTE)         ;/* Uebergabe der SpriteDaten */
VOID SpriteFarbe (INT, PBYTE)         ;/* Uebergabe Spritefarben    */
VOID SpriteActiv (INT)                ;/* Sprite-Aktivierung        */
VOID Sprite (INT)                     ;/* Sprites zeichnen          */
VOID Sprite1 (INT)                    ;/* Sprites zeichnen          */
INT  SpriteKoll(INT)                  ;/* liefert den Kollisionscode*/
INT  SpriteUnt (INT)                  ;/* liefert den Spriteuntergr.*/
unsigned char *Spritewo(INT);

/*   Modul "FILEWORK.CPP"                                           */
VOID Spriteladen(PBYTE ,PBYTE)        ;/* laedt ein Sprite in       */
				       /* seinen Speicher           */
VOID Charladen(PBYTE, PBYTE, BYTE)    ;
VOID TGA_laden(PCHAR)                 ;/* laedt ein VGA-Bild (.tga) */
VOID Bild_aufblenden(VOID)            ;/* einblenden des Bildes     */
VOID Bild_ausblenden(VOID)            ;/* blendet das Bild aus      */
VOID Levelladen(INT, PBYTE, PWORD)    ;/* laedt ein Level           */
VOID Levelspeichern(INT,PBYTE, PWORD) ;/* speichert ein Level       */

/*== Variablen =====================================================*/
VOID interrupt(*oldhandler)(__CPPARGS);/* enthaelt alten Tastatur-  */
				       /*  Vektor   */
VOID interrupt(*oldtimehdl)(__CPPARGS);/* enthaelt alten Timer-     */
				       /*  Vektor      */
INT  VGAalt=0                         ;/* enthaelt alten Grafikmodus*/
BYTE scan                             ;/* enthaelt Scancode Tastatur*/
/*WORD Taste=0                        ;/* Dualwert der Tastatur     */
				       /*  (Space,CTRL,ESC,Ret,     */
				       /*   O,U,R,L)                */
BYTE Seite_zeigen                     ;/* Seite, die gerade gezeigt */
				       /*  wird                     */
BYTE Seite_schreiben                  ;/* Seite, die gerade         */
                                       /*  bearbeitet wird          */
//BYTE Seite_hinten=3                   ;/* Hintergrundseite          */
INT Zeit = 2                          ;/* Wartekonstante            */
INT Wartezeit                         ;/* Wartezaehler fuer         */
                                       /*  Timerinterrupt  */
INT SpKollision                       ;/* Kollisionsvaraible Kuno   */
INT SpUntergrund                      ;/* Untergrundvariable Kuno   */
INT Level                             ;/* aktuelles Spiellevel      */
//INT Raum                              ;/* aktueller Spielraum       */
PBYTE ObjTxt[]={
"NICHTS  ","BLUME   ","BAND LI ",
"BAND RE ","100 SACK","WAPPEN  ",
"SCHWERT ","SLIMER  ","100 PKT ",
"20 PKT  ","50 PKT  ","KATAPULT",
"EXPLSION","RALF    ","HEILUNG ",
"DIAMANT ","500 PKT ","KEY BLAU",
"KEY GELB","KEY ROT ","KEY GRUE",
"KEY PKT ","EXIT    ","MAUER   ",
"WAND 1  ","WAND 2  ","WAND 3  ",
"WAND 4  ","TOR BLAU","TOR GELB",
"TOR ROT ","TOR GRUE","GECKO   ",
"SCHALTER","GE WAND1","WASSERF ",
"BURGWASS","WIZROTAT","HEXE    ",
}                                     ;
PBYTE ObjTab[]=                        /* Objektzuordnung beim Raum-*/
{                                      /*  betreten                 */
 KLINKS1  , BLUME1   , BAND1    ,
 BAND1    , SACK     , WAPPEN   ,
 SCHWERT  , SLIMER1  , POINT100 ,
 POINT20  , POINT50  , KATAPULT1,
 BUMM1    , RALF     , HEALTH   ,
 DIAMANT1 , POINT500 , KEYBLAU  ,
 KEYGELB  , KEYROT   , KEYGRUEN ,
 POINTKEY , EXIT     , MAUER    ,
 WAND1    , WAND2    , WAND3    ,
 WAND4    , TORBLAU  , TORGELB  ,
 TORROT   , TORGRUEN , GECKO1   ,
 SCHALTER1, WAND3    , FALL1    ,
 WASSER1  , WIZROT1  , HEXELI1
 }				      ;/* Objektzuordnung beim Raum-*/
				       /*  betreten                 */
INT Objekte[19*(4)*10]                ;/* Spielfeld bewegliche Obj. */
INT Objekt                            ;/* Zaehlvariable fuer Monster*/
INT Modus=0                           ;/* zuerst Textmodus          */
INT FCURSOR=0                         ;/* zuerst Text-Cursor        */
PBYTE FCursor[]={                      /* Cursor-Phasen-Zuordnung   */
CURSOR1,CURSOR11,CURSOR12,CURSOR13,
CURSOR2,CURSOR21,CURSOR22,CURSOR23
}                                     ;
INT SPRITE=1                          ;/* zuerst Sprite-Nr 0        */
INT Art=0                             ;/* zuerst Spriteart 0        */
INT SpriteX=274                       ;/* Position Anzeige Spriteart*/
INT SpriteY=113                       ;
INT LevelX =-1                        ;/* Position Levelcursor      */
INT LevelY =-1                        ;
INT TextX  =0                         ;/* Position Textcursor       */
INT TextY  =171                       ;
INT Levelx =0                         ;/* aktuelle Cursor-Position  */
INT Levely =0                         ;
PCHAR SpriteBeginn		      ;/* Zeiger auf Sprite-Speicher*/


/********************************************************************/
/* Interrupt - Routine fuer den Timer                               */
/*------------------------------------------------------------------*/
/* Eingabe : keine                                                  */
/* Ausgabe : Wartezeit = Zaehler wird pro Durchlauf ums 1 erhoeht   */
/********************************************************************/
VOID interrupt timehdl(__CPPARGS)
{
/*-- Wartezaehler um 1 erhoehen                                     */
 Wartezeit++                          ;
}
/***************************************************************************/
/* Interrupt - Routine fÅr die "Spiel-Tastatur"				   */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine							   */
/* Ausgabe : Variable TASTE wird als Flag benutzt			   */
/***************************************************************************/
void interrupt handler(__CPPARGS)
{
/*-- Scan-Code holen							   */
 scan =inportb(0x60);
 if (scan<128) goto Make; 		/* Taste gedrÅckt oder losgelassen */
/*-- Taste losgelassen						 	   */
   scan=scan-128;
   if (scan==27) Taste=Taste^0x1000;	/* +     losgelassen               */
   if (scan==57) Taste=Taste^0x800;	/* SPACE losgelassen		   */
   if (scan==43) Taste=Taste^0x400;	/* #     losgelassen		   */
   if (scan==14) Taste=Taste^0x200;	/* <-    losgelassen		   */
   if (scan==15) Taste=Taste^0x100;	/* Tab   losgelassen		   */
   if (scan==1 ) Taste=Taste^0x20;	/* ESC   losgelassen		   */
   if (scan==73) Taste=Taste^0x80; 	/* PgUp  losgelassen		   */
   if (scan==81) Taste=Taste^0x40;	/* PgDwn losgelassen		   */
   if (scan==28) Taste=Taste^0x10;	/* ENTER losgelassen		   */
   if (scan==75) Taste=Taste^0x01;	/* LINKS losgelassen		   */
   if (scan==72) Taste=Taste^0x08;	/* HOCH 			   */
   if (scan==77) Taste=Taste^0x02;	/* RECHTS                          */
   if (scan==80) Taste=Taste^0x04;	/* TIEF				   */
   goto SEnde;                          /* und zurÅck			   */

Make:
/*-- Taste gedrÅckt                                                        */
   if (scan==27) Taste=Taste|0x1000;
   if (scan==57) Taste=Taste|0x800;
   if (scan==43) Taste=Taste|0x400;
   if (scan==14) Taste=Taste|0x200;
   if (scan==15) Taste=Taste|0x100;
   if (scan==1 ) Taste=Taste|0x20;
   if (scan==73) Taste=Taste|0x80;
   if (scan==81) Taste=Taste|0x40;
   if (scan==28) Taste=Taste|0x10;
   if (scan==75) Taste=Taste|0x01;
   if (scan==72) Taste=Taste|0x08;
   if (scan==77) Taste=Taste|0x02;
   if (scan==80) Taste=Taste|0x04;

SEnde:
/*-- Interrupts freigeben						   */
   outportb (0x20,0x20);
}

/***************************************************************************/
/* ERROR   : Fehler-Routine mit Programm-Abruch                            */
/*-------------------------------------------------------------------------*/
/* Eingabe :	MESSAGE	= enth"lt einen String, der als Fehlermeldung      */
/*			  ausgegeben wird 				   */
/***************************************************************************/

void huge error( const char *message )
{
 closegraph();      		/* Grafikebene ausschalten (<GRAPHICS.H>)  */
 _AL=VGAalt; 					 /* alten VGA-Modus laden  */
 _AH=0;
 geninterrupt(0x10);  		      /* DOS-Interrupt ausl"sen (<DOS.H>)  */
 cout << ("\n\nError: %s \n\n", message);	      /* Meldung ausgeben  */
 printf  ("\nTaste drÅcken!");			           /* (<STDIO.H>)  */
 getch();		       	    /* Warten auf Tastendruck (<CONIO.H>)  */
 exit(1); 			      /* Programm verlassen (<PROCESS.H>)  */
}

/***************************************************************************/
/* GRAFIKTEST:  Testet die Grafikkarte auf VGA-Tauglichkeit                */
/*-------------------------------------------------------------------------*/
/* Eingabe   :  keine							   */
/* Ausgabe   :  0 - VGA-Karte ist vorhanden				   */
/*  	        1 - Fehler, keine VGA-Karte                                */
/***************************************************************************/

int huge Grafiktest(void)
{
 int x;    			/* RÅcksprungvariable, enth"lt Fehlercode  */

 x=1;				     /* Annahme, da· kein VGA installiert  */
 asm mov ax,0x1A00;		       /* Funktion 1Ah, Unterfunktion 00h  */
 asm int 0x10; 					     /* VGA-BIOS aufrufen  */
 if (_AL==0x1A)   				 /* wenn VGA installiert,  */
 {
  x=0; 						  /* dann kein Fehlercode  */
 }
 return (x);
}

/***************************************************************************/
/* Warten : Zeitschleife zur Anpassung der unterschiedlichen CPUs          */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine                                                         */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void Warten(void)
{
Warte1:
 if (Wartezeit>=Zeit) Wartezeit=0;
 else		      goto Warte1;
 return;
}

/***************************************************************************/
/* Level_Zeichen : wandelt den Levelcode in ein BS-taugliches Zeichen um   */
/*-------------------------------------------------------------------------*/
/* Eingabe : Zeichen = Levelcode (zB A oder 1 usw)                         */
/* Ausgabe : Zeiger auf Pixelstruktur                                      */
/***************************************************************************/
unsigned char *Level_Zeichen (unsigned char Zeichen)
{
 unsigned char *Punkt;			        /* Zeiger                  */
 int xb;				  	/* Z"hler		   */

 for (xb=0;xb<37;xb++)
 {
/*-- Vergleich mit der Umrechnungstabelle                                  */
  if (Tabchar[xb]==Zeichen)
  {
   Punkt=Levelz+(xb*64);			/* Zeiger berechnen	   */
   return (Punkt);
  }
 }

 return (Punkt);
}


/***************************************************************************/
/* Level_zeigen : Zeichnet das aktuelle Level in den BS-Hintergrund        */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine                                                         */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void Level_zeigen (void)
{
 unsigned char* LevelDaten;   		/* Zeiger auf die Levelstruktur    */
 unsigned char* LevelNummer="0";	/* Zeiger auf die LevelNummer	   */
 unsigned char* RaumNummer ="0";	/* Zeiger auf die RaumNummer	   */
 int xb=0;				/* X-Koord BS			   */
 int yb=0;				/* Y-Koord BS			   */
 int xz,yz;				/* 8x8 Z"hler		           */
 int xt;				/* Tabellenz"hler		   */
 unsigned char Zeichen;			/* Levelzeichen			   */
 unsigned char *Punkt;			/* Pixel			   */


/*-- auf Hintergrund-Seite umschalten                                      */
 setpage(Seite_hinten);

/*-- aktuellen LevelZeiger holen					   */
 LevelDaten=Level0;

/*-- Zeichenschleife                                                       */
 for (yb=0;yb<19;yb++)			/* Levelh"he			   */
 {
  for (xb=0;xb<31;xb++)			/* Levelbreite			   */
  {
/*-- Levelzeichen ermitteln						   */
   xt=Raum*31*19;			/* Raum im Level einstellen        */
   xt=xt+(yb*31+xb);			/* Position im Raum ermitteln      */
   Zeichen=LevelDaten[xt];
   Punkt=Level_Zeichen(Zeichen);
   for (yz=0;yz<8;yz++)
   {
    for (xz=0;xz<8;xz++)
    {
     setpix(xb*8+xz,yb*8+yz,(unsigned char)Punkt[yz*8+xz]);
    }
   }
  }
 }

/*-- Sprites Raum 0 aktivieren						   */
 for (xz=1;xz<17;xz++)
 {
  xb=Objekt0[(Raum*19*4)+((xz*4)+0)];
  if (xb==0)
  {
   SpriteDeactiv(xz);
  }
  else
  {
   SpriteActiv(xz);
   SpriteDaten(xz,ObjTab[xb]);
   SpriteKoord(xz,Objekt0[(Raum*19*4)+((xz*4)+2)],Objekt0[(Raum*19*4)+
							((xz*4)+3)]);
  }
 }


/*-- Ausgabe der Level- und RaumNummer auf den Bildschirm		   */
 *(char *)LevelNummer=(char)(Level+48);
 Text_Ausgabe (((unsigned char*)LevelNummer),1,1,280,23);
 *(char *)RaumNummer =(char)(Raum +48);
 Text_Ausgabe (((unsigned char*)RaumNummer ),1,1,280,63);


/*-- auf Bearbeitungs-Seite zurÅckschalten und zurÅck                      */
 setpage(Seite_schreiben);

/*-- unbenutzten Hintergrund kopieren                                      */
 blockmove(Seite_hinten,Seite_schreiben,0,0,0,0,320,200);
 blockmove(Seite_hinten,Seite_zeigen,0,0,0,0,320,200   );

 return;
}

/***************************************************************************/
/* Steuerung : reagiert auf die einzelnen Tasten		  	   */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine							   */
/* Ausgabe : keine							   */
/***************************************************************************/
void Steuerung (void)
{
 if ((Taste&0x800)==0x800)
 {
  if (Modus==0) goto Marke7;
  Objekt0[(Raum*19*4)+((SPRITE*4)+0)]=Art;
  if (Art==1 )  Objekt0[(Raum*19*4)+((SPRITE*4)+1)]=random(5 );
  if (Art==15)  Objekt0[(Raum*19*4)+((SPRITE*4)+1)]=random(20);
  if (Art==7 )  Objekt0[(Raum*19*4)+((SPRITE*4)+1)]=random(70);
  if (Art==32)  Objekt0[(Raum*19*4)+((SPRITE*4)+1)]=random(70);
  Objekt0[(Raum*19*4)+((SPRITE*4)+2)]=SpriteX;
  Objekt0[(Raum*19*4)+((SPRITE*4)+3)]=SpriteY;
  goto Marke6;
Marke7:
  Levelx=(LevelX+1)/8;
  Levely=(LevelY+1)/8;
  int xt=Raum*31*19;			/* Raum im Level einstellen        */
  xt=xt+(Levely*31+Levelx);		/* Position im Raum ermitteln      */
  Level0[xt]=Tabchar[TextX/9];
  Level_zeigen();
  return;
 }
 if ((Taste&0x1000)==0x1000)
 {
  Art--;
  if (Art<0) Art=(sizeof(ObjTab)/4-1);
  SpriteDaten(17,ObjTab[Art]);
  SpriteActiv(17);
  if (Art==0) SpriteDeactiv(17);
  if (Modus==0) goto Marke5;
  SpriteKoord(SPRITE,SpriteX,SpriteY);
  SpriteActiv(SPRITE);
  if (Art==0) SpriteDeactiv(SPRITE);
  SpriteDaten(SPRITE,ObjTab[Art]);
  return;
 }
 if ((Taste&0x400)==0x400)
 {
  Art++;
  if (Art>(sizeof(ObjTab)/4-1)) Art=0;
  SpriteDaten(17,ObjTab[Art]);
  SpriteActiv(17);
  if (Art==0) SpriteDeactiv(17);
  if (Modus==0) goto Marke5;
  SpriteKoord(SPRITE,SpriteX,SpriteY);
  SpriteActiv(SPRITE);
  if (Art==0) SpriteDeactiv(SPRITE);
  SpriteDaten(SPRITE,ObjTab[Art]);
Marke5:
  return;
 }
 if ((Taste&0x200)==0x200)
 {
Marke6:
  Modus=1;
  SpriteDaten(19,CURSOR2);
  SPRITE++;
  if (SPRITE>16) SPRITE=1;
  Art=Objekt0[(Raum*19*4)+((SPRITE*4)+0)];
  SpriteX=Objekt0[(Raum*19*4)+((SPRITE*4)+2)];
  SpriteY=Objekt0[(Raum*19*4)+((SPRITE*4)+3)];
  SpriteKoord(19,SpriteX,SpriteY);
  SpriteActiv(17);
  if (Objekt0[(Raum*19*4)+((SPRITE*4)+0)]==0) SpriteDeactiv(17);
  SpriteDaten(17,ObjTab[Art]);
  return;
 }

 if ((Taste&0x80)==0x80)		/* PgUp				   */
 {
  Modus=0;
  LevelX=-1;
  LevelY=-1;
  SpriteDaten(19,CURSOR1);
  SpriteKoord(19,LevelX,LevelY);
  Raum++;
  if (Raum>9)
  {
   Raum=0;
   if (Level==9) Raum=9;
   Levelspeichern(Level,Level0,Objekt0);
   Level++;
   if (Level>9) Level=9;
   Levelladen(Level,Level0,Objekt0);
  }
  Level_zeigen();
  return;
 }
 if ((Taste&0x40)==0x40)		/* PgDn				   */
 {
  Modus=0;
  LevelX=-1;
  LevelY=-1;
  SpriteDaten(19,CURSOR1);
  SpriteKoord(19,LevelX,LevelY);
  if (Raum==0)
  {
   Raum=9;
   if (Level==0) Raum=0;
   Levelspeichern(Level,Level0,Objekt0);
   Level--;
   if (Level<0) Level=0;
   Levelladen(Level,Level0,Objekt0);
  }
  else Raum--;

  Level_zeigen();
  return;
 }
 if ((Taste&0x100)==0x100)
 {
  Modus=0;
  SpriteDaten(19,CURSOR1);
  TextX=TextX+9;
  if (TextX>200) TextX=0;
  SpriteKoord(18,TextX,TextY);
  return;
 }
 if ((Taste&0x01)==0x01)
 {
  if (Modus==1) goto Marke1;
  LevelX=LevelX-8;
  if (LevelX<-1) LevelX=239;
  SpriteKoord(19,LevelX,LevelY);
  return;
Marke1:
  SpriteX--;
  if (SpriteX<-7) SpriteX=233;
  SpriteKoord(19,SpriteX,SpriteY);
  SpriteKoord(SPRITE,SpriteX,SpriteY);
  return;
 }
 if ((Taste&0x02)==0x02)
 {
  if (Modus==1) goto Marke2;
  LevelX=LevelX+8;
  if (LevelX>239) LevelX=-1;
  SpriteKoord(19,LevelX,LevelY);
  return;
Marke2:
  SpriteX++;
  if (SpriteX>233) SpriteX=-7;
  SpriteKoord(19,SpriteX,SpriteY);
  SpriteKoord(SPRITE,SpriteX,SpriteY);
  return;
 }
 if ((Taste&0x08)==0x08)
 {
  if (Modus==1) goto Marke3;
  LevelY=LevelY-8;
  if (LevelY<-1) LevelY=143;
  SpriteKoord(19,LevelX,LevelY);
  return;
Marke3:
  SpriteY--;
  if (SpriteY<0) SpriteY=132;
  SpriteKoord(19,SpriteX,SpriteY);
  SpriteKoord(SPRITE,SpriteX,SpriteY);
  return;
 }
 if ((Taste&0x04)==0x04)
 {
  if (Modus==1) goto Marke4;
  LevelY=LevelY+8;
  if (LevelY>143) LevelY=-1;
  SpriteKoord(19,LevelX,LevelY);
  return;
Marke4:
  SpriteY++;
  if (SpriteY>132) SpriteY=0;
  SpriteKoord(19,SpriteX,SpriteY);
  SpriteKoord(SPRITE,SpriteX,SpriteY);
  return;
 }

 return;
}

/***************************************************************************/
/* Bild_zeigen : das fertige Spielfeld wird angezeigt und das Hinter-      */
/*               grund-Spielfeld vorbereitet                               */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine                                                         */
/* Ausgabe : keine                                                         */
/***************************************************************************/

void Bild_zeigen(void)
{
/*-- fertige Seite anzeigen                                                */
 showpage(Seite_schreiben);

/*-- zweite Seite zur Arbeit freigeben                                     */
 setpage(Seite_zeigen);

/*-- Seitennummern wechseln                                                */
 if (Seite_schreiben==0)
 {
  Seite_schreiben=1;
  Seite_zeigen=0;
 }
 else
 {
  Seite_schreiben=0;
  Seite_zeigen=1;
 }


 return;
}


/***************************************************************************/
/* Bild_Aufbau : baut das Spielfeld im Hintergrund auf und blendet es ein  */
/*-------------------------------------------------------------------------*/
/* Eingabe     : keine                                                     */
/* Ausgabe     : keine                                                     */
/***************************************************************************/
void Bild_Aufbau(void)
{
 unsigned char *Text="01";
 int alt;
 int xz;

 if (SPRITE>16) SPRITE=1;			/* gr"·er als 17?	   */
 if (SPRITE<1)  SPRITE=1;			/* kleiner als 1?	   */

/*-- Spriteanzahl retten						   */
 alt=SPRITE;

/*-- Zehner- bis Einerstellen abtrennen und in Text umwandeln              */
 *(unsigned char*)(Text+0)=48;
 if (SPRITE>=10) *(unsigned char*)(Text+0)=(SPRITE/10)+48;
 SPRITE=SPRITE-((SPRITE/10)*10);
 if (SPRITE>=0 ) *(unsigned char*)(Text+1)=(SPRITE/1 )+48;
 SPRITE=SPRITE-((SPRITE/1 )*1 );

/*-- Sprite zurÅckholen				         		   */
 SPRITE=alt;

/*-- Umschalten auf Hintergrundseite und Textausgabe			   */
 setpage(Seite_hinten);
 Text_Ausgabe(Text,2,0,280,105);
 Text_Ausgabe(ObjTxt[Art],8,0,265,135);
 setpage(Seite_schreiben);

/*-- unbenutzten Hintergrund kopieren                                      */
 blockmove(Seite_hinten,Seite_schreiben,0,0,0,0,320,200);

/*-- Spiel-Objekte setzten ------------------------------------------------*/
 for (int z=0;z<20;z++)
 {
  Sprite(z);
 }

/*-- Abfrage Kuno-Steuerung -----------------------------------------------*/
 Steuerung();
 Warten();
 return;
}


/***************************************************************************/
/* MAIN    :  Hauptprogramm vom  K U N O -Leveleditor			   */
/***************************************************************************/

void main()
{
/*-- 3.1 SUB-Variablen ----------------------------------------------------*/

/*-- 3.2 Testen der Systemvoraussetzungen ---------------------------------*/
/*-- Alte Einstellungen merken --------------------------------------------*/

/* aktuellen Videomodus holen und in der Variablen "VGAalt" abspeichern    */
 _AH=0xF;
 geninterrupt(0x10);
 VGAalt=_AL;

/* Grafikkarte testen (VGA ist Voraussetzung)      			   */
 if (Grafiktest()==1)
 {
  error("keine VGA-Karte");
 }


/*-- Speicher fÅr Sprites reservieren -------------------------------------*/
 SpriteBeginn = (PCHAR) farmalloc(24*21*40);
 if (SpriteBeginn == NULL)
 {
  error("nicht genÅgend Speicher");
 }


/*-- Globale Variablen initialisieren                                      */
 Seite_hinten=3;
 Raum=0;

/*-- Grafiken einladen ----------------------------------------------------*/
 Zeichenladen();
 Charladen("level_z .tga",Levelz,37);
 Charladen("editorz .tga",Levelz,11);

/*-- Zuordnung der Sprites im Spritespeicher ------------------------------*/
 HEXELI1= SpriteBeginn+(24*21*0);
 WIZROT1= SpriteBeginn+(24*21*1);
 KLINKS1= SpriteBeginn+(24*21*2);
 BAND1=SpriteBeginn+(24*21*3);

/*-- Zuordnung der Spites zur Objekttabelle -------------------------------*/
 ObjTab[0]=(PCHAR)KLINKS1;
 ObjTab[2]=(PCHAR)BAND1;
 ObjTab[3]=(PCHAR)BAND1;
 ObjTab[37]=(PCHAR)WIZROT1;
 ObjTab[38]=(PCHAR)HEXELI1;

/*-- Sprites laden							   */
 Spriteladen("klinks1 .tga",KLINKS1   );
 Spriteladen("band1   .tga",BAND1     );
 Spriteladen("slimer1 .tga",SLIMER1   );
 Spriteladen("blume1  .tga",BLUME1    );
 Spriteladen("katap1  .tga",KATAPULT1 );
 Spriteladen("schwert .tga",SCHWERT   );
 Spriteladen("sack    .tga",SACK      );
 Spriteladen("wappen  .tga",WAPPEN    );
 Spriteladen("punkt100.tga",POINT100  );
 Spriteladen("punkt50 .tga",POINT50   );
 Spriteladen("punkt20 .tga",POINT20   );
 Spriteladen("bumm1   .tga",BUMM1     );
 Spriteladen("ralf    .tga",RALF      );
 Spriteladen("health  .tga",HEALTH    );
 Spriteladen("diamant1.tga",DIAMANT1  );
 Spriteladen("punkt500.tga",POINT500  );
 Spriteladen("keyblau .tga",KEYBLAU   );
 Spriteladen("keygelb .tga",KEYGELB   );
 Spriteladen("keyrot  .tga",KEYROT    );
 Spriteladen("keygruen.tga",KEYGRUEN  );
 Spriteladen("punktkey.tga",POINTKEY  );
 Spriteladen("exit    .tga",EXIT      );
 Spriteladen("mauer   .tga",MAUER     );
 Spriteladen("wand1   .tga",WAND1     );
 Spriteladen("wand2   .tga",WAND2     );
 Spriteladen("wand3   .tga",WAND3     );
 Spriteladen("wand4   .tga",WAND4     );
 Spriteladen("torblau .tga",TORBLAU   );
 Spriteladen("torgelb .tga",TORGELB   );
 Spriteladen("torrot  .tga",TORROT    );
 Spriteladen("torgruen.tga",TORGRUEN  );
 Spriteladen("gecko1  .tga",GECKO1    );
 Spriteladen("schalt1 .tga",SCHALTER1 );
 Spriteladen("fall1   .tga",FALL1     );
 Spriteladen("fall2   .tga",FALL2     );
 Spriteladen("fall3   .tga",FALL3     );
 Spriteladen("wasser1 .tga",WASSER1   );
 Spriteladen("wasser2 .tga",WASSER2   );
 Spriteladen("wasser3 .tga",WASSER3   );
 Spriteladen("wasser4 .tga",WASSER4   );
 Spriteladen("wizrot1 .tga",WIZROT1   );
 Spriteladen("hexere1 .tga",HEXELI1   );


/*-- Cursor-Arten laden							   */
 Spriteladen("cursor1 .tga",CURSOR1   );
 Spriteladen("cursor11.tga",CURSOR11  );
 Spriteladen("cursor12.tga",CURSOR12  );
 Spriteladen("cursor13.tga",CURSOR13  );
 Spriteladen("cursor2 .tga",CURSOR2   );
 Spriteladen("cursor21.tga",CURSOR21  );
 Spriteladen("cursor22.tga",CURSOR22  );
 Spriteladen("cursor23.tga",CURSOR23  );

/*-- PC-System fÅr Spiel einstellen ---------------------------------------*/
/* Grafikmodus einschalten                                                 */
 init320200();				      /* Initialisierung VGA-Modus */
 setpage(0);                                  /* Seite #0 als aktive Seite */
 showpage(0);

/*-- Tastatur-Vektor retten                                                */
   oldhandler = getvect(0x09);

/*-- neuen Tastatur-Vektor setzen                                          */
   setvect(0x09, handler);

/*-- Timer-Vektor retten                                                   */
   oldtimehdl = getvect(0x1C);

/*-- neuen Timer   -Vektor setzen                                          */
   setvect(0x1C, timehdl);

/*-- Setzen der Bildschirm-Variablen auf Anfang                            */
 Seite_zeigen   =0;
 Seite_schreiben=1;

/*-- Hintergrund laden und anzeigen                                        */
 setpage(Seite_hinten);
 TGA_laden("editor.tga");
 showpage(Seite_hinten);
 Bild_aufblenden();

 setpage(Seite_schreiben);

/*-- Auswertung der Parameter                                              */
 Levelladen(Level,Level0,Objekt0);
 Textur_zeigen();
 Level_zeigen();

/*-- Grundeinstellung des Spritecursors					   */
 SpriteActiv(19);
 SpriteDaten(19,CURSOR1);
 SpriteKoord(19,LevelX,LevelY);

 SpriteActiv(18);
 SpriteDaten(18,CURSOR1);
 SpriteKoord(18,TextX,TextY);
 SpriteKoord(17,SpriteX,SpriteY);

/*-- Bearbeitungsschleife -------------------------------------------------*/
weiter:
 Bild_Aufbau();
 Bild_zeigen();
 FCURSOR++;
 if (FCURSOR>3) FCURSOR=0;
 SpriteDaten(19,FCursor[(Modus)*4+FCURSOR]);
/*-- Abfrage, ob die Taste ESC gedrÅckt wurde				   */
 if ((Taste&0x20) != 0x20)
	goto weiter;			/* ESC nicht gedrÅckt		   */


Ende:

 Levelspeichern(Level,Level0,Objekt0);

/*-- alten Tastaturvektor zurÅcksetzen                                     */
 setvect(0x09, oldhandler);

/*-- alten Timervektor zurÅcksetzen                                        */
 setvect(0x1C, oldtimehdl);



/*-- Alte Einstellungen wieder herstellen ---------------------------------*/
/* alten Videomodus herstellen 						   */
 closegraph();
 _AL=VGAalt;
 _AH=0;
 geninterrupt(0x10);

/*-- reservierten Speicher freigeben ----------------------------------------*/
 farfree(SpriteBeginn);

return;
}

/***************************************************************************/
/* Level_Attribut: wandelt den Levelcode in ein Attributbyte um            */
/*-------------------------------------------------------------------------*/
/* Eingabe : Zeichen = Levelcode (zB A oder 1 usw)                         */
/* Ausgabe : Attribut-Byte                                                 */
/***************************************************************************/
BYTE Level_Attribut(BYTE Zeichen)
{
 BYTE Punkt;
 int xb;				  	/* ZÑhler		   */

 for (xb=0;xb<37;xb++)
 {
/*-- Vergleich mit der Umrechnungstabelle                                  */
  if (Tabchar[xb]==Zeichen)
  {
   Punkt=Tabattr[xb]   ;			/* Zeiger berechnen	   */
   return (Punkt);
  }
 }

 return (Punkt);
}
/***************************************************************************/
/* Textur_zeigen : Zeichnet die aktuellen Texturen den BS-Hintergrund      */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine                                                         */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void Textur_zeigen (void)
{
 unsigned char* LevelDaten;   		/* Zeiger auf die Levelstruktur    */
 unsigned char* LevelNummer="0";	/* Zeiger auf die LevelNummer	   */
 unsigned char* RaumNummer ="0";	/* Zeiger auf die RaumNummer	   */
 int xb=1;				/* X-Koord BS			   */
 int yb=172;				/* Y-Koord BS			   */
 int xz,yz;				/* 8x8 Z"hler		           */
 int xt=0;				/* Tabellenzaehler		   */
 unsigned char Zeichen;			/* Levelzeichen			   */
 unsigned char *Punkt;			/* Pixel			   */


/*-- auf Hintergrund-Seite umschalten                                      */
 setpage(Seite_hinten);

/*-- aktuellen LevelZeiger holen					   */
// LevelDaten=Level0;

/*-- Zeichenschleife                                                       */
 for (yb;yb<190;yb=yb+9)			/* Levelhoehe			   */
 {
  for (xb;xb<300;xb=xb+9)			/* Levelbreite			   */
  {
/*-- Levelzeichen ermitteln						   */
//   xt=Raum*31*19;			/* Raum im Level einstellen        */
//   xt=xt+(yb*31+xb);			/* Position im Raum ermitteln      */

   if (xt>37) goto Ende;
   Punkt=Levelz+(xt*64);			/* Zeiger berechnen	   */
   xt++;
   for (yz=0;yz<8;yz++)
   {
    for (xz=0;xz<8;xz++)
    {
     setpix(xb+xz,yb+yz,(unsigned char)Punkt[yz*8+xz]);
    }
   }
  }
 }

Ende:
/*-- auf Bearbeitungs-Seite zurÅckschalten und zurÅck                      */
 setpage(Seite_schreiben);

/*-- unbenutzten Hintergrund kopieren                                      */
// blockmove(Seite_hinten,Seite_schreiben,0,0,0,0,320,200);
// blockmove(Seite_hinten,Seite_zeigen,0,0,0,0,320,200   );

 return;
}

