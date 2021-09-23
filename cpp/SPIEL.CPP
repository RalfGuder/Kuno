/***************************************************************************/
/* S P I E L . C P P                                                       */
/*-------------------------------------------------------------------------*/
/* Aufgabe       :  Quellcode fÅr alle Spiel-Routinen fÅr                  */
/*                  K U N O   -   D E R   R I T T E R			   */
/*-------------------------------------------------------------------------*/
/* Autor         :  Ralf Guder						   */
/* Arbeitsbeginn :  30.01.96						   */
/* Arbeitsende   :                                                         */
/* letztes Update:  22.03.99                                               */
/*-------------------------------------------------------------------------*/
/* Funktionen                                          			   */
/* ----------								   */
/*-------------------------------------------------------------------------*/
/* Info 								   */
/* ----									   */
/* Bildschirmseite 0 und 1     - Darstellungsseiten			   */
/* Bildschirmseite 2           - Hintergrundseite                          */
/***************************************************************************/

/*-- INCLUDE-Dateien ------------------------------------------------------*/
#include <conio.h>
#include <dos.h>
#include <alloc.h>
#include "types.h"
#include "LEVEL.CPP"                  /* enthÑlt die Level des Spieles     */
#include "SPRITEDA.CPP"      /* enthÑlt die Daten der Sprites des Spieles  */
#include "KUNO.H"


/*-- externe Funktionen ---------------------------------------------------*/


/*   Modul "FILEWORK.CPP"                                                  */
void Spriteladen(unsigned char *, 	/* LÑdt ein Sprite in den Speicher */
		 unsigned char *);
void Charladen (PBYTE,PBYTE,BYTE);         /* LÑdt die Leveltextur            */
void Levelladen(int,unsigned char far*,
		    unsigned int *);
void TGA_laden(char *);			/* lÑdt ein VGA-Bild (.tga)        */
void Bild_aufblenden(void);             /* einblenden des Bildes           */
void Bild_ausblenden(void);             /* blenden das Bild aus            */


/*   Modul "ADLIB.CPP"							   */
void Adlibvoice(unsigned int,   	/* initialisiert eine Stimme       */
		unsigned char*);
void Adlibreset(void);			/* zurÅcksetzen der Karte          */
void StimmeDaten(unsigned char,
		 unsigned int *);	/* Åbergibt Stimmdaten  	   */
void StimmeActiv(unsigned char);
void LiedDaten(unsigned char,
		 unsigned int *);	/* Åbergibt Lieddaten  	   	   */
void LiedActiv(unsigned char);
void StimmeDeactiv(unsigned char);
void Stimme (unsigned char);





/*-- Module-Variablen -----------------------------------------------------*/
CHAR  Note			       ;/* enthÑlt den ZÑhlwert bei Timer  */
BYTE  scan			       ;/* enthÑlt Scancode der Tastatur   */
BYTE  scanalt=0			       ;/* enthÑlt alten Scancode          */

WORD DUMMY=0;
//INT   Kuno=0			       ;/* Kuno ist Sprite Nummer 0        */
INT   KEnde   =0		       ;/* Endverhalten von Kuno	   */
INT   KBeginn =0;			/* Startverhalten von Kuno         */
INT   KSprung =0;			/* SprungzÑhler                    */
INT   KFall   =0;			/* Sprung und FallzÑhler           */
INT   KPult   =0;			/* Sprung  mit Katapult            */
BYTE  KZustand=0;			/* Bewegungsphase Kuno             */

PBYTE KPhasen[19];			/* Array fÅr Kunos BewegungsPhasen */

PBYTE HEXE[4];				/* Bewegungphasen Hexe             */

PBYTE WIZROT[9];

PBYTE SLIMER[]=				/* Bewegungsphasen Slimer	   */
{ SLIMER1  ,SLIMER2   ,
  SLIMER3  ,SLIMER4   };

PBYTE GECKO []=				/* Bewegungsphasen Gecko 	   */
{ GECKO1   ,GECKO2    ,
  GECKO3   ,GECKO4    };

INT   KunoX,KunoY;		        /* Position Kuno                   */
INT   KunoXalt,KunoYalt;		/* Positionzwischenspeicher        */
int KunoXStart,KunoYStart	       ;/* Position Kuno beim Betreten Raum*/

int LevelX=18,LevelY=191;		/* Position LevelNummer		   */
int RaumX =32,RaumY =191;		/* Position RaumNummer		   */
int Level;				/* aktuelles Spiellevel            */
//int Raum;				/* aktueller Spielraum             */
int Kraft;      			/* Anzahl Power			   */
int Gameover;
int GameZaehler;
int GameoverZustand;
int GameoverZahl[]=
{1,2,3,4,5,0};
PBYTE Leben     ="0"		       ;/* Anzahl der Leben		   */
int Wappen =0;				/* Anzahl der Wappen		   */
int Schwert=0;				/* Anzahl der Schwerter		   */
int Diamanten=0;			/* Anzahl der SchlÅssel		   */
int Keyrot=0;
int Keygruen=0;
int Keyblau=0;
int Keygelb=0;
long PUNKTE=0;

PCHAR SpriteBeginn;		        /* Zeiger auf SpriteSpeicher       */
PCHAR SpriteOTTO;
unsigned char*ObjTab[]=			/* Objektzuordnung beim Raum-      */
{					/*  betreten			   */
 KLINKS1  ,                             /* 0                               */
 BLUME1   ,                             /* 1*/
 BAND1    ,
 BAND1    ,
 SACK     ,
 WAPPEN   ,
 SCHWERT  ,
 SLIMER1  ,
 POINT100 ,
 POINT20  ,
 POINT50  ,
 KATAPULT1,
 BUMM1,
 RALF,
 HEALTH,
 DIAMANT1,
 POINT500,
 KEYBLAU,
 KEYGELB,
 KEYROT,
 KEYGRUEN,
 POINTKEY,
 EXIT,
 MAUER,
 WAND1,
 WAND2,
 WAND3,
 WAND4,
 TORBLAU,
 TORGELB,
 TORROT,
 TORGRUEN,
 GECKO1,
 SCHALTER1,
 LEER,					/* 34-  geheime Mauer		    */
 FALL1,                                 /* 35-  Wasserfall                  */
 WASSER1,
 WIZROT1,				/* 37 - Wiz-Rotator                 */
 HEXERE1				/* 38 - Hexe                        */
 };

int Objekte[19*(4)*10];			/* Spielfeld bewegliche Objekte    */
int Objekt;				/* ZÑhlvariable fÅr Monster        */

BYTE SPhasen []=              		/* Zuordnung der einzelnen         */
{ 1,0,3,2,4,5,6,7,9,8 }		       ;/* Bewegungsphasen der Sprites     */
BYTE SPhasenL[]=              		/* Zuordnung der einzelnen         */
{ 1,2,3,0,5,6,7,4}		       ;/* Bewegungsphasen der Sprites     */


BYTE  Seite_zeigen		       ;/* Seite, die gerade gezeigt wird  */
BYTE  Seite_schreiben		       ;/* Seite, die gerade bearb. wird   */
// BYTE  Seite_Reserve=3               ;/* Reserve-Bildschirmseite         */
// BYTE  Seite_hinten=2      	       ;/* Hintergrundseite                */
INT   Zeit = 10			       ;/* Wartekonstante                  */
INT   Wartezeit			       ;/* WartezÑhler fÅr Timerinterrupt  */
INT   SpKollision		       ;/* Kollisionsvaraible Kuno         */
INT   SpUntergrund		       ;/* Untergrundvariable Kuno	   */
PBYTE Zonenname[]={			/* enthÑlt die Beschreibungen der  */
"Burggraben","Burggraben",		/*  einzelnen Level-Zonen	   */
"100 Blumen","Secret one",
"Wappenraum","Pallas    ",
"KÅche     ","          ",
"          ","KC 85     ", }			       ;


/***************************************************************************/
/* Interrupt - Routine fÅr den Timer					   */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine							   */
/* Ausgabe : Wartezeit = ZÑhler wird pro Durchlauf ums 1 erhîht            */
/***************************************************************************/
void interrupt timehdl(__CPPARGS)
{
 INT x;					/* SchleifenzÑhler		   */
/*-- WartezÑhler um 1 erhîhen						   */
 Wartezeit++;
/*-- NotenzÑhler heruntersetzen						   */
 Note--;
 if (Note<0)
 {
  Note=1;
/*-- Musikschleife							   */
 for (x=0;x<11;x++) Stimme(x);
 }
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
 scan =inportb(0x60)		       ;
 if (scan==scanalt) goto SEnde	       ;/* nur bei neuem Scancode	   */
 scanalt=scan			       ;/* Scancode Åbernehmen		   */
 if (scan<128) goto Make	       ;/* Taste gedrÅckt oder losgelassen */
/*-- Taste losgelassen						 	   */
   scan=scan-128;
   if (scan==1 ) Taste=Taste^0x20;	/* ESC   losgelassen		   */
   if (scan==57) Taste=Taste&0x7F; 	/* SPACE losgelassen		   */
   if (scan==29) Taste=Taste^0x40;	/* CTRL  losgelassen		   */
   if (scan==28) Taste=Taste^0x10;	/* ENTER losgelassen		   */
   if (scan==75) Taste=Taste^0x01;	/* LINKS losgelassen		   */
   if (scan==72) Taste=Taste^0x08;	/* HOCH 			   */
   if (scan==77) Taste=Taste^0x02;	/* RECHTS                          */
   if (scan==80) Taste=Taste^0x04;	/* TIEF				   */
   goto SEnde;                          /* und zurÅck			   */

Make:
/*-- Taste gedrÅckt                                                        */
   if (scan==1 ) Taste=Taste|0x20;
   if (scan==57) Taste=Taste|0x80;
   if (scan==29) Taste=Taste|0x40;
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
/* GetTaste : Fragt den Tastaturstatus ab                                  */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine                                                         */
/* Ausgabe : Tastaturwert                                                  */
/***************************************************************************/
BYTE GetTaste(VOID)
{
 return (Taste);
}


/***************************************************************************/
/* Kuno_Steuerung : Fragt den Tastaturstatus ab und reagiert darauf        */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine                                                         */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void Kuno_Steuerung( void )
{

/*-- Abfrage der Sprungvariablen "KSprung" --------------------------------*/
 if ((KSprung!=0)||(KPult!=0))		/* Kuno springt			   */
 {
  KFall++;
  if (KFall>6)	                	/* maximale Sprunghîhe             */
   {KSprung=0;}
//    Taste=Taste&0x7F;}			/* SPACE zurÅcksetzen		   */
  if ((Taste&0x80)!=0x80) KSprung=0;    /* Sprung nur, wenn SPACE-Druck    */
  KunoY=KunoY-4;
  if (KZustand<4)
  {
   if ((KZustand>1)&&(KZustand<4))	/* Kuno lief vorher rechts 	   */
	KZustand=5;
   else
	KZustand=4;                     /* Kuno lief vorher links          */
  }
  if (KZustand>5)			/* Kuno steht nach einem Sprung    */
  {
   if (KZustand==6) KZustand=4;
   if (KZustand==7) KZustand=5;
  }
 }

/*-- Abfrage SPACE-Taste --------------------------------------------------*/
 if ((Taste&0x80) == 0x80)		/* Taste Sprung			   */
 {
  if ((KSprung==0)&&(KFall==0)&&
		(KPult==0))         	/* Sprung nur, wenn Kuno steht     */
	KSprung=1;
 }

/*-- Abfrage Cursor-Taste HOCH --------------------------------------------*/
 if ((Taste&0x08) == 0x08)		/* Taste oben			   */
 {
  SpKollision=SpriteKoll(Kuno);
  if ((SpKollision&0x80)==0x80) 	/* Reaktion nur, wenn Leiter       */
  {
   KunoY=KunoY-4;
   if ((KZustand<8)||(KZustand>9))      /* Kuno geht noch keine Leiter     */
	KZustand=8;                     /*  hoch                           */
  }
 KZustand=SPhasen[KZustand];		/* nÑchste Phase holen     	   */
 return;
 }

/*-- Abfrage Cursor-Taste TIEF --------------------------------------------*/
 if ((Taste&0x04) == 0x04)		/* Taste unten			   */
 {
  SpUntergrund=SpriteUnt (Kuno);
  SpKollision =SpriteKoll(Kuno);
  if (((SpUntergrund&0x80)==0x80)|| 	/* Reaktion nur, wenn Leiter       */
      ((SpKollision&0x80)==0x80))
  {
   KunoY=KunoY+4;
   if ((KZustand<8)||(KZustand>9))     /* Kuno geht noch keine Leiter     */
	KZustand=8;                     /*  hoch                           */
   KZustand=SPhasen[KZustand];		/* nÑchste Phase holen     	   */
  }
  return;
 }

/*-- Abfrage Cursor-Taste LINKS -------------------------------------------*/
 if ((Taste&0x01) == 0x01)		/* Taste links			   */
 {
  if (KZustand>1) KZustand=0;		/* lÑuft schon nach links          */
  KunoX=KunoX-4;
  KZustand=SPhasen[KZustand];		/* nÑchste Phase holen             */
  if (KFall!=0) KZustand=4;		/* Kuno fÑllt nach links	   */
  return;
 }
 if ((Taste&0x02) == 0x02)              /* Taste rechts			   */
 {
  if ((KZustand<2)||(KZustand>3))       /* lÑuft schon nach rechts         */
	KZustand=2;
  KunoX=KunoX+4;
  KZustand=SPhasen[KZustand];		/* nÑchste Phase holen     	   */
  if (KFall!=0) KZustand=5;		/* Kuno fÑllt nach rechts          */
  return;
 }
/*-- keine Taste und kein Sprung				           */
 if (KFall==0)
 {
  if ((KZustand>1)&&(KZustand<4)) KZustand=7;
  if (KZustand<2) KZustand=6;
  if (KZustand==4) KZustand=6;
  if (KZustand==5) KZustand=7;
  if (KZustand>7) KZustand=7;
 }
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
 int xb;				  	/* ZÑhler		   */

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

/*---                                         */
VOID LEVEL_POINT(INT BS_X,INT BS_Y, PCHAR Punkt)
{
   INT xb,yb;     /*Zaehlvariablen*/
   for (yb=0;yb<8;yb++)
   {
    for (xb=0;xb<+8;xb++)
    {
     setpix(BS_X*8+xb,(BS_Y*8+yb),(BYTE)Punkt[yb*8+xb]);
    }
   }

 return;
}

/***************************************************************************/
/* Level_zeigen : Zeichnet das aktuelle Level in den BS-Hintergrund        */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine                                                         */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void Level_zeigen (void)
{
// PBYTE LevelDaten;   			/* Zeiger auf die Levelstruktur    */
 PBYTE LevelNummer="0"		       ;/* Zeiger auf die LevelNummer	   */
 PBYTE RaumNummer ="0"		       ;/* Zeiger auf die RaumNummer	   */
 int xb=0;				/* X-Koord BS			   */
 int yb=0;				/* Y-Koord BS			   */
 int xz,yz;				/* 8x8 ZÑhler		           */
 int xt;				/* TabellenzÑhler		   */
 unsigned char Zeichen;			/* Levelzeichen			   */
 unsigned char *Punkt;			/* Pixel			   */


/*-- auf Hintergrund-Seite umschalten                                      */
 setpage(Seite_hinten);

/*-- aktuellen LevelZeiger holen					   */
// LevelDaten=Level0;
/* LevelDaten=LevelNr[Level];*/

/*-- Zeichenschleife                                                       */
 for (yb=0;yb<19;yb++)			/* Levelhîhe			   */
 {
  for (xb=0;xb<31;xb++)			/* Levelbreite			   */
  {
/*-- Levelzeichen ermitteln						   */
   xt=Raum*31*19;			/* Raum im Level einstellen        */
   xt=xt+(yb*31+xb);			/* Position im Raum ermitteln      */
   Zeichen=(BYTE)LevelDaten[xt];
   Punkt=Level_Zeichen(Zeichen);
   LEVEL_POINT(xb,yb,Punkt);
  }
 }

/*-- Sprites Raum 0 aktivieren						   */
 for (xz=1;xz<17;xz++)
 {
  xb=Objekte[(Raum*19*4)+((xz*4)+0)];
  if (xb==0)
  {
   SpriteDeactiv(xz);
  }
  else
  {
   SpriteActiv(xz);
   SpriteDaten(xz,ObjTab[xb]);
   SpriteKoord(xz,Objekte[(Raum*19*4)+((xz*4)+2)],Objekte[(Raum*19*4)+
							((xz*4)+3)]);
  }
 }


/*-- Ausgabe der Level- und RaumNummer auf den Hintergrund-Bildschirm      */
 setpage(Seite_hinten);
 *(char *)LevelNummer=(char)(Level+48);
 Text_Ausgabe (((unsigned char*)LevelNummer),1,1,LevelX,LevelY);
 *(char *)RaumNummer =(char)(Raum +48);
 Text_Ausgabe (((unsigned char*)RaumNummer ),1,1,RaumX ,RaumY );
 Text_Ausgabe (Zonenname[(Level*9)+Raum],10,1,76,191);


/*-- auf Bearbeitungs-Seite zurÅckschalten und zurÅck                      */
 setpage(Seite_schreiben);

/*-- unbenutzten Hintergrund kopieren                                      */
 blockmove(Seite_hinten,Seite_schreiben,0,0,0,0,320,200);
 blockmove(Seite_hinten,Seite_zeigen,0,0,0,0,320,200);

 return;
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

/*-- Synchronzeit abwarten                                                 */
 Warten();

 return;
}


/***************************************************************************/
/* Power : dezimiert die Poweranzahl und gibt sie auf den BS               */
/*-------------------------------------------------------------------------*/
/* Eingabe : Pwr = Anzahl der Punkte, die dezimiert werden sollen          */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void Power(int Pwr)
{
 unsigned char Punkt;			/* Farbe des Powerbalkens          */
 int x,z;				/* ZÑhlvariablen		   */
 Kraft=Kraft+Pwr;
 if (Kraft>45) Kraft=45;
 if (Kraft<=0)
 {
  Kraft=45;
  KEnde=8;
  *(char*)Leben=*(char*)Leben-1;
  if (*(char*)Leben==47)
  {
   Gameover=1;
  }
 }
/*-- Farbe des Balkens ermitteln					   */
 if (Kraft>=0)  Punkt=251;
 if (Kraft>=15) Punkt=249;
 if (Kraft>=30) Punkt=247;
/*-- Umschalten auf Hintergrund-bild					   */
 setpage(Seite_hinten);
/*-- Farbbalken zeichnen						   */
 for (x=181;x<(181+Kraft);x++)
 {
  for (z=192;z<196;z++)
  {
   setpix(x,z,Punkt);
  }
 }
/*-- Rest des Balkens in schwarz                                           */
 for  (x=(181+Kraft);x<(181+45);x++)
 {
  for (z=192;z<196;z++)
  {
   setpix(x,z,252);
  }
 }
/*-- Umschalten auf Schreibseite            				   */
 Text_Ausgabe(Leben,1,1,200,180);
 setpage(Seite_schreiben);
 return;
};

/***************************************************************************/
/* Sword : Anzeigen und bearbeiten der Schwerter                           */
/*-------------------------------------------------------------------------*/
/* Eingabe : Swd = Anzahl der zu bearbeitenden Schwerter                   */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void Sword (int Swd)
{
/*-- Funktionsvariablen							   */
 unsigned char *Text="  ";			/* Zeiger auf Ausgabetext  */
 int alt;					/* Zwischenspeicher        */

/*-- neue Schwertanzahl berechnen und ÅberprÅfen			   */
 Schwert=Schwert+Swd;				/* neue Schwertanzahl	   */
 if (Schwert>=100) Schwert=99;			/* grî·er als 99?	   */
 if (Schwert<0)    Schwert=0;			/* kleiner als 0?	   */

/*-- Schwertanzahl retten						   */
 alt=Schwert;

/*-- Zehner- bis Einerstellen abtrennen und in Text umwandeln              */
 if (Schwert>=10) *(unsigned char*)(Text+0)=(Schwert/10)+48;
 Schwert=Schwert-((Schwert/10)*10);
 if (Schwert>=0 ) *(unsigned char*)(Text+1)=(Schwert/1 )+48;
 Schwert=Schwert-((Schwert/1 )*1 );

/*-- Schwertanzahl zurÅckholen						   */
 Schwert=alt;

/*-- Umschalten auf Hintergrundseite und Textausgabe			   */
 setpage(Seite_hinten);
 Text_Ausgabe(Text,2,1,292,80);
 setpage(Seite_schreiben);

/*-- zurÅck zum Aufrufer						   */
 return;
};

/***************************************************************************/
/* Diamant : Anzeigen und Bearbeiten der Diamanten                         */
/*-------------------------------------------------------------------------*/
/* Eingabe : Dia = Anzahl der zu bearbeitenden Schwerter                   */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void Diamant (int Dia)
{
/*-- Funktionsvariablen							   */
 unsigned char *Text="  ";			/* Zeiger auf Ausgabetext  */
 int alt;					/* Zwischenspeicher        */

/*-- neue Schwertanzahl berechnen und ÅberprÅfen			   */
 Diamanten=Diamanten+Dia;			/* neue Diamantenanzahl    */
 if (Diamanten>=100)				/* grî·er als 99?	   */
 {
  Diamanten=0;
  *(char*)Leben=*(char*)Leben+1;
  if (*(char*)Leben==58)
  {
   *(char*)Leben=57;
  }
  Power(0);
 }

 if (Diamanten<0) Diamanten=0;			/* kleiner als 0?	   */

/*-- Schwertanzahl retten						   */
 alt=Diamanten;

/*-- Zehner- bis Einerstellen abtrennen und in Text umwandeln              */
 if (Diamanten>=10) *(unsigned char*)(Text+0)=(Diamanten/10)+48;
 Diamanten=Diamanten-((Diamanten/10)*10);
 if (Diamanten>=0 ) *(unsigned char*)(Text+1)=(Diamanten/1 )+48;
 Diamanten=Diamanten-((Diamanten/1 )*1 );

/*-- Diamantenanzahl zurÅckholen					   */
 Diamanten=alt;

/*-- Umschalten auf Hintergrundseite und Textausgabe			   */
 setpage(Seite_hinten);
 Text_Ausgabe(Text,2,1,292,124);
 setpage(Seite_schreiben);

/*-- zurÅck zum Aufrufer						   */
 return;
};

/***************************************************************************/
/* Wapp : Anzeigen und bearbeiten der Wappen                               */
/*-------------------------------------------------------------------------*/
/* Eingabe : Wpp = Anzahl der zu bearbeitenden Schwerter                   */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void Wapp (int Wpp)
{
/*-- Funktionsvariablen							   */
 unsigned char *Text="  ";			/* Zeiger auf Ausgabetext  */
 int alt;					/* Zwischenspeicher        */

/*-- neue Wappenanzahl berechnen und ÅberprÅfen				   */
 Wappen=Wappen+Wpp;				/* neue Wappenanzahl	   */
 if (Wappen>=100) Wappen=99;			/* grî·er als 99?	   */
 if (Wappen<0)    Wappen=0;			/* kleiner als 0?	   */

/*-- Wappenanzahl retten						   */
 alt=Wappen;

/*-- Zehner- bis Einerstellen abtrennen und in Text umwandeln              */
 if (Wappen>=10) *(unsigned char*)(Text+0)=(Wappen/10)+48;
 Wappen=Wappen-((Wappen/10)*10);
 if (Wappen>=0 ) *(unsigned char*)(Text+1)=(Wappen/1 )+48;
 Wappen=Wappen-((Wappen/1 )*1 );

/*-- Wappenanzahl zurÅckholen						   */
 Wappen=alt;


/*-- Umschalten auf Hintergrundseite und Textausgabe			   */
 setpage(Seite_hinten);
 Text_Ausgabe(Text,2,1,292,102);
 setpage(Seite_schreiben);

/*-- zurÅck zum Aufrufer						   */
 return;
};

/***************************************************************************/
/* Key : Anzeigen und Bearbeiten der SchlÅssel                             */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine					                   */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void Key (void)
{
/*-- Funktionsvariablen							   */
 PBYTE Text="    "		       ;/* Zeiger auf Ausgabetext  */

 *(unsigned char*)(Text+3)=32;
 *(unsigned char*)(Text+2)=32;
 *(unsigned char*)(Text+1)=32;
 *(unsigned char*)(Text+0)=32;
 if (Keyrot  ==1) *(unsigned char*)(Text+3)=38;
 if (Keygruen==1) *(unsigned char*)(Text+2)=37;
 if (Keyblau ==1) *(unsigned char*)(Text+1)=36;
 if (Keygelb ==1) *(unsigned char*)(Text+0)=21;

/*-- Umschalten auf Hintergrundseite und Textausgabe			   */
 setpage(Seite_hinten);
 Text_Ausgabe(Text,4,1,196,170);
 setpage(Seite_schreiben);

/*-- zurÅck zum Aufrufer						   */
 return;
};
/***************************************************************************/
/* Punkte : erhîht die Punktanzahl und gibt sie auf den BS                 */
/*-------------------------------------------------------------------------*/
/* Eingabe : Pkt = Anzahl der Punkte, die erhîht werden sollen             */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void Punkte (int Pkt)
{
 unsigned char* Text="0000000";
 long alt;
 PUNKTE=PUNKTE+Pkt;
 if (PUNKTE>=10000000) PUNKTE=0;
 alt=PUNKTE;
 if (PUNKTE>=1000000) *(unsigned char*)(Text+0)=(PUNKTE/1000000)+48;
 PUNKTE=PUNKTE-((PUNKTE/1000000)*1000000);
 if (PUNKTE>=100000 ) *(unsigned char*)(Text+1)=(PUNKTE/100000 )+48;
 PUNKTE=PUNKTE-((PUNKTE/100000 )*100000 );
 if (PUNKTE>=10000  ) *(unsigned char*)(Text+2)=(PUNKTE/10000  )+48;
 PUNKTE=PUNKTE-((PUNKTE/10000  )*10000  );
 if (PUNKTE>=1000   ) *(unsigned char*)(Text+3)=(PUNKTE/1000   )+48;
 PUNKTE=PUNKTE-((PUNKTE/1000   )*1000   );
 if (PUNKTE>=100    ) *(unsigned char*)(Text+4)=(PUNKTE/100    )+48;
 PUNKTE=PUNKTE-((PUNKTE/100    )*100    );
 if (PUNKTE>=10     ) *(unsigned char*)(Text+5)=(PUNKTE/10     )+48;
 PUNKTE=PUNKTE-((PUNKTE/10     )*10     );
 if (PUNKTE>=0      ) *(unsigned char*)(Text+6)=(PUNKTE/1      )+48;
 PUNKTE=PUNKTE-((PUNKTE/1      )*1      );

 setpage(Seite_hinten);
 Text_Ausgabe (((unsigned char*)Text),7,0,268,28);
 setpage(Seite_schreiben);

 PUNKTE=alt;
 return;
};

/***************************************************************************/
/* Kuno_Reak : Reaktionsunterprogramm fÅr Kuno                             */
/*-------------------------------------------------------------------------*/
/* Eingabe : Kollision = Kollisionscode fÅr Kuno                           */
/* Ausgabe : keine                                                         */
/***************************************************************************/
VOID Kuno_Reak()
{
 PBYTE LevelNummer="0"		       ;
 PBYTE RaumNummer ="0"		       ;
 BYTE  SpKollAlt                       ;/* Rettungsvariable */

 SpKollAlt=SpKollision;

/*-- Kollision mit einem Sprite                                            */
 if ((SpKollision&0x10)==0x10)
 {
  SpKollision=Spritekollision();	        /* SpriteNummer holen      */
  Objekt=Objekte[(SpKollision*4)+(Raum*19*4)];  /* Art des Objektes holen  */

/*-- 100-Punkte-Sack							   */
  if (Objekt==4)			        /* 100-Sack                */
  {
   Objekte[(SpKollision*4)+(Raum*19*4)]=8;	/* 100-Punkte aufsteigen   */
   SpriteDaten(SpKollision,ObjTab[8]);
   Punkte(100);
   goto Reak1;
  }


/*-- Feuerblume	-----------------------------------------------------------*/
  if (Objekt==1)				/* Feuerblume		   */
  {
   if (Schwert>0)                               /* Schwert vorhanden ?     */
   {
    Sword(-1);
    Objekte[(SpKollision*4)+(Raum*19*4+0)]=12;
    Objekte[(SpKollision*4)+(Raum*19*4+1)]=-1;
    Punkte(10);
    goto Reak1;
   }
   if (Wappen>0)				/* Wappen vorhanden ?      */
   {
    Wapp(-1);
    goto Reak1;
   }
   Power(-2);					/* Kuno war blank          */
   goto Reak1;
  }

/*-- Wappen ---------------------------------------------------------------*/
  if (Objekt==5)				/* Wappen 		   */
  {
   Wapp(1);
   Objekte[(SpKollision*4)+(Raum*19*4)]=10; 	/* 50-Punkte aufsteigen    */
   SpriteDaten(SpKollision,ObjTab[10]);
   Punkte(50);
   goto Reak1;
  }

/*-- Schwert								   */
  if (Objekt==6)				/* Schwert		   */
  {
   Sword(1);
   Objekte[(SpKollision*4)+(Raum*19*4)]=9; 	/* 20-Punkte aufsteigen    */
   SpriteDaten(SpKollision,ObjTab[9]);
   Punkte(20);
   goto Reak1;
  }

/*-- Slimer	-----------------------------------------------------------*/
  if (Objekt==7)				/* Slimer    		   */
  {
   if (Schwert>0)                               /* Schwert vorhanden ?     */
   {
    Sword(-1);
    Objekte[(SpKollision*4)+(Raum*19*4+0)]=12;
    Objekte[(SpKollision*4)+(Raum*19*4+1)]=-1;
    Punkte(25);
    goto Reak1;
   }
   if (Wappen>0)				/* Wappen vorhanden ?      */
   {
    Wapp(-1);
    goto Reak1;
   }
   Power(-2);					/* Kuno war blank          */
   goto Reak1;
  }

/*-- Wiz-Rotator ----------------------------------------------------------*/
  if (Objekt==37)				/* Wiz-Rotator	   */
  {
   if (Schwert>0)                               /* Schwert vorhanden ?     */
   {
    Sword(-1);
    Objekte[(SpKollision*4)+(Raum*19*4+0)]=12;
    Objekte[(SpKollision*4)+(Raum*19*4+1)]=-1;
    Punkte(30);
    goto Reak1;
   }
   if (Wappen>0)				/* Wappen vorhanden ?      */
   {
    Wapp(-1);
    goto Reak1;
   }
   Power(-2);					/* Kuno war blank          */
   goto Reak1;
  }

/*-- Powerflasche ---------------------------------------------------------*/
  if (Objekt==14)				/* Powerflasche		   */
  {
   Power(10);
   Objekte[(SpKollision*4)+(Raum*19*4)]=0; 	/* Objekt ausblenden       */
   SpriteDeactiv(SpKollision);
   goto Reak1;
  }

/*-- Diamant---------------------------------------------------------------*/
  if (Objekt==15)				/* Diamant 		   */
  {
   Diamant(1);
   Objekte[(SpKollision*4)+(Raum*19*4)]=16; 	/* 500-Punkte aufsteigen   */
   SpriteDaten(SpKollision,ObjTab[16]);
   Punkte(500);
   goto Reak1;
  }

/*-- SchlÅssel-------------------------------------------------------------*/
  if (Objekt==17)				/* SchlÅssel blau          */
  {
   if (Keyblau==0)			        /* wenn noch keinen Key    */
   {
    Objekte[(SpKollision*4)+(Raum*19*4)]=21; 	/* 500-Punkte aufsteigen   */
    SpriteDaten(SpKollision,ObjTab[21]);
    Keyblau=1;
    Key();
    goto Reak1;
   }
   goto Reak1;
  }
  if (Objekt==18)				/* SchlÅssel gelb          */
  {
   if (Keygelb==0)			        /* wenn noch keinen Key    */
   {
    Objekte[(SpKollision*4)+(Raum*19*4)]=21; 	/* 500-Punkte aufsteigen   */
    SpriteDaten(SpKollision,ObjTab[21]);
    Keygelb=1;
    Key();
    goto Reak1;
   }
   goto Reak1;
  }
  if (Objekt==19)				/* SchlÅssel rot          */
  {
   if (Keyrot==0)			        /* wenn noch keinen Key    */
   {
    Objekte[(SpKollision*4)+(Raum*19*4)]=21; 	/* 500-Punkte aufsteigen   */
    SpriteDaten(SpKollision,ObjTab[21]);
    Keyrot=1;
    Key();
    goto Reak1;
   }
   goto Reak1;
  }
  if (Objekt==20)				/* SchlÅssel grÅn          */
  {
   if (Keygruen==0)			        /* wenn noch keinen Key    */
   {
    Objekte[(SpKollision*4)+(Raum*19*4)]=21; 	/* 500-Punkte aufsteigen   */
    SpriteDaten(SpKollision,ObjTab[21]);
    Keygruen=1;
    Key();
    goto Reak1;
   }
   goto Reak1;
  }
/*-- Tore -----------------------------------------------------------------*/
  if (Objekt==31)				/* Tor grÅn                */
  {
   if (Keygruen==1)			        /* wenn grÅnen      Key    */
   {
    Objekte[(SpKollision*4)+(Raum*19*4)]=0; 	/* Tor lîschen             */
    SpriteDeactiv(SpKollision);
    Keygruen=0;
    Key();
    goto Reak1;
   }
   KunoX=KunoXalt;		  /* alten Horizontalwert zurÅcklesen	   */
   if (KSprung!=0) KSprung=0;     /* SprÅnge beenden		           */
   if (KPult  !=0) KPult  =0;
   if (KFall==0)  KunoY=KunoYalt; /* wenn kein Fall vorliegt, dann auch    */
				  /* alten Vertikalwert zurÅcklesen        */
   goto Reak1;
  }
  if (Objekt==28)				/* Tor blau                */
  {
   if (Keyblau==1)			        /* wenn blauen      Key    */
   {
    Objekte[(SpKollision*4)+(Raum*19*4)]=0; 	/* Tor lîschen             */
    SpriteDeactiv(SpKollision);
    Keyblau=0;
    Key();
    goto Reak1;
   }
   KunoX=KunoXalt;		  /* alten Horizontalwert zurÅcklesen	   */
   if (KSprung!=0) KSprung=0;     /* SprÅnge beenden		           */
   if (KPult  !=0) KPult  =0;
   if (KFall==0)  KunoY=KunoYalt; /* wenn kein Fall vorliegt, dann auch    */
				  /* alten Vertikalwert zurÅcklesen        */
   goto Reak1;
  }
  if (Objekt==29)				/* Tor gelb                */
  {
   if (Keygelb==1)			        /* wenn gelben      Key    */
   {
    Objekte[(SpKollision*4)+(Raum*19*4)]=0; 	/* Tor lîschen             */
    SpriteDeactiv(SpKollision);
    Keygelb=0;
    Key();
    goto Reak1;
   }
   KunoX=KunoXalt;		  /* alten Horizontalwert zurÅcklesen	   */
   if (KSprung!=0) KSprung=0;     /* SprÅnge beenden		           */
   if (KPult  !=0) KPult  =0;
   if (KFall==0)  KunoY=KunoYalt; /* wenn kein Fall vorliegt, dann auch    */
				  /* alten Vertikalwert zurÅcklesen        */
   goto Reak1;
  }
  if (Objekt==30)				/* Tor rot                 */
  {
   if (Keyrot==1)			        /* wenn roten       Key    */
   {
    Objekte[(SpKollision*4)+(Raum*19*4)]=0; 	/* Tor lîschen             */
    SpriteDeactiv(SpKollision);
    Keyrot=0;
    Key();
    goto Reak1;
   }
   KunoX=KunoXalt;		  /* alten Horizontalwert zurÅcklesen	   */
   if (KSprung!=0) KSprung=0;     /* SprÅnge beenden		           */
   if (KPult  !=0) KPult  =0;
   if (KFall==0)  KunoY=KunoYalt; /* wenn kein Fall vorliegt, dann auch    */
				  /* alten Vertikalwert zurÅcklesen        */
   goto Reak1;
  }

/*-- Gecko ----------------------------------------------------------------*/
  if (Objekt==32)				/* Gecko    		   */
  {
   if (Schwert>0)                               /* Schwert vorhanden ?     */
   {
    Sword(-1);
    Objekte[(SpKollision*4)+(Raum*19*4+0)]=12;
    Objekte[(SpKollision*4)+(Raum*19*4+1)]=-1;
    Punkte(25);
    goto Reak1;
   }
   if (Wappen>0)				/* Wappen vorhanden ?      */
   {
    Wapp(-1);
    goto Reak1;
   }
   Power(-2);					/* Kuno war blank          */
   goto Reak1;
  }

/*-- Schalter -------------------------------------------------------------*/
  if (Objekt==33)				/* Schalter    		   */
  {
   int xz, xb;
   if ((Taste&0x40)==0x40)                      /* CTRL gedrÅckt ?         */
   {
    Objekte[(SpKollision*4)+(Raum*19*4+1)]=1;
    SpriteDaten(SpKollision,SCHALTER2);
    for (xz=1;xz<17;xz++)
    {
     xb=Objekte[(Raum*19*4)+((xz*4)+0)];
     if (xb==34)				/* geheime Mauer                  */
     {
      Objekte[(Raum*19*4)+((xz*4)+1)]=1;
     }
    }
   }
     goto Reak1;
  }
/*-- Wasser ---------------------------------------------------------------*/
  if (Objekt==36)				/* Wasser   		   */
  {
   if (Wappen>0)				/* Wappen vorhanden ?      */
   {
    Wapp(-1);
    goto Reak1;
   }
   Power(-2);					/* Kuno war blank          */
   goto Reak1;
  }
/*-- Exit-Schild-----------------------------------------------------------*/
  if (Objekt==22)				/* "EXIT"                  */
  {
   Taste=Taste&0x7F		       	       ;/* SPACE zurÅcksetzen      */
   Raum=0                                      ;/* Anfangsraum im neuen    */
						/* Level                   */
   Level=Level++			       ;
   Levelladen(Level,LevelDaten,Objekt0)	       ;/* neues Level laden       */
   for (int xz=0;xz<19*10*4;xz++)               /* Level-Objekte in den    */
   {						/* Spiel-Speicher holen    */
    Objekte[(xz)]= Objekt0[(xz)];
   }

   goto Wechsel1			       ;

  }

Reak1:
 }
 SpKollision=SpKollAlt;

/*-- TÅr zum nÑchsten Raum                                                 */
 if ((SpKollision&0x20)==0x20)	   	/* TÅr zum nÑchsten Raum   	   */
 {
  Taste=Taste&0x7F		       ;/* SPACE zurÅcksetzen		   */
  Raum=Tuerkollision();
  if (KunoX<8)				/* TÅr links		   	   */
  {
   KunoX=KunoX+220;
  }
  else
  {
   KunoX=KunoX-220;
  }
  KunoXStart=KunoX;				/* Position Kuno merken    */
  KunoYStart=KunoY;
Wechsel1:
  Level_zeigen();

/*-- Ausgabe der Level- und RaumNummer auf den Bildschirm -----------------*/
 *(char *)LevelNummer=(char)(Level+48);
 Text_Ausgabe (((unsigned char*)LevelNummer),1,2,LevelX,LevelY);
 *(char *)RaumNummer =(char)(Raum +48);
 Text_Ausgabe (((unsigned char*)RaumNummer ),1,2,RaumX ,RaumY );

  return;
 }

/*-- Spielrand                                                             */
 if ((SpKollision&0x40)==0x40)
 {
  Taste=Taste&0x7F		       ;/* SPACE zurÅcksetzen		   */
  KunoX=KunoXalt		       ;/* alten Horizontalwert zurÅcklesen*/
  if (KSprung!=0) KSprung=0	       ;/* SprÅnge beenden		   */
  if (KPult  !=0) KPult  =0	       ;
  if (KFall==0)  KunoY=KunoYalt	       ;/* wenn kein Fall vorliegt, 	   */
					/* dann auch alten Vertikalwert    */
					/* zurÅcklesen         		   */

  return;
 }
 return;
}


/***************************************************************************/
/* Band_links: Laufband nach links					   */
/*-------------------------------------------------------------------------*/
/* Eingabe : Nummer des Objektes                                           */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void Band_links(int xz)
{
 Objekte[(xz*4)+(Raum*19*4)+1]++;
 if (Objekte[(xz*4)+(Raum*19*4)+1]==4)
	Objekte[(xz*4)+(Raum*19*4)+1]=0;
 if (Objekte[(xz*4)+(Raum*19*4)+1]==0)
	SpriteDaten(xz,BAND1);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==1)
	SpriteDaten(xz,BAND2);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==2)
	SpriteDaten(xz,BAND3);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==3)
	SpriteDaten(xz,BAND4);
 return;
};

/***************************************************************************/
/* Band_rechts: Laufband nach links					   */
/*-------------------------------------------------------------------------*/
/* Eingabe : Nummer des Objektes                                           */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void Band_rechts(int xz)
{
 Objekte[(xz*4)+(Raum*19*4)+1]++;
 if (Objekte[(xz*4)+(Raum*19*4)+1]==4)
	Objekte[(xz*4)+(Raum*19*4)+1]=0;
 if (Objekte[(xz*4)+(Raum*19*4)+1]==0)
	SpriteDaten(xz,BAND4);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==1)
	SpriteDaten(xz,BAND3);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==2)
	SpriteDaten(xz,BAND2);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==3)
	SpriteDaten(xz,BAND1);
 return;
};
/***************************************************************************/
/* Blume : Funktion fÅr die einzelnen Phasen der Feuerblumen               */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine						     	   */
/* Ausgabe : keine							   */
/***************************************************************************/
void Blume (int xz)
{
 Objekte[(xz*4)+(Raum*19*4)+1]++;
 if (Objekte[(xz*4)+(Raum*19*4)+1]==4)
	Objekte[(xz*4)+(Raum*19*4)+1]=0;
 if (Objekte[(xz*4)+(Raum*19*4)+1]==0)
	SpriteDaten(xz,BLUME1);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==1)
	SpriteDaten(xz,BLUME2);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==2)
	SpriteDaten(xz,BLUME3);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==3)
	SpriteDaten(xz,BLUME4);
 return;
};

/***************************************************************************/
/* Fall : Funktion fÅr die einzelnen Phasen des Wasserfalls                */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine						     	   */
/* Ausgabe : keine							   */
/***************************************************************************/
void Fall (int xz)
{
 Objekte[(xz*4)+(Raum*19*4)+1]++;
 if (Objekte[(xz*4)+(Raum*19*4)+1]==3)
	Objekte[(xz*4)+(Raum*19*4)+1]=0;
 if (Objekte[(xz*4)+(Raum*19*4)+1]==0)
	SpriteDaten(xz,FALL1);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==1)
	SpriteDaten(xz,FALL2);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==2)
	SpriteDaten(xz,FALL3);
 return;
};
/***************************************************************************/
/* Wasser : Funktion fÅr die einzelnen Phasen des Wassers                  */
/*-------------------------------------------------------------------------*/
/* Eingabe : Spritenummer       				     	   */
/* Ausgabe : keine							   */
/***************************************************************************/
VOID Wasser (INT xz)
{
 Objekte[(xz*4)+(Raum*19*4)+1]++;
 if (Objekte[(xz*4)+(Raum*19*4)+1]==6)
	Objekte[(xz*4)+(Raum*19*4)+1]=0;
 if (Objekte[(xz*4)+(Raum*19*4)+1]==0)
	SpriteDaten(xz,WASSER1);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==1)
	SpriteDaten(xz,WASSER2);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==2)
	SpriteDaten(xz,WASSER3);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==3)
	SpriteDaten(xz,WASSER4);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==4)
	SpriteDaten(xz,WASSER5);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==5)
	SpriteDaten(xz,WASSER6);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==6)
	SpriteDaten(xz,WASSER7);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==7)
	SpriteDaten(xz,WASSER8);
 return;
};

/***************************************************************************/
/* Kata : Funktion fÅr die einzelnen Phasen des Katapultes                 */
/*-------------------------------------------------------------------------*/
/* Eingabe : xz = SpriteNummer					     	   */
/* Ausgabe : keine							   */
/***************************************************************************/
void Kata (int xz)
{
 Objekte[(xz*4)+(Raum*19*4)+1]++;
 if (Objekte[(xz*4)+(Raum*19*4)+1]==3)
	Objekte[(xz*4)+(Raum*19*4)+1]=0;
 if (Objekte[(xz*4)+(Raum*19*4)+1]==0)
	SpriteDaten(xz,KATAPULT1);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==1)
	SpriteDaten(xz,KATAPULT2);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==2)
	SpriteDaten(xz,KATAPULT3);
 return;
};

/***************************************************************************/
/* Diamant : Funktion fÅr die einzelnen Phasen des Diamantenglitzerns      */
/*-------------------------------------------------------------------------*/
/* Eingabe : xz = SpriteNummer					     	   */
/* Ausgabe : keine							   */
/***************************************************************************/
void Diaman (int xz)
{
 Objekte[(xz*4)+(Raum*19*4)+1]++;
 if (Objekte[(xz*4)+(Raum*19*4)+1]==20)
 {
  Objekte[(xz*4)+(Raum*19*4)+1]=0;
  SpriteDaten(xz,DIAMANT1);
 }
 if (Objekte[(xz*4)+(Raum*19*4)+1]==17)
	SpriteDaten(xz,DIAMANT2);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==18)
	SpriteDaten(xz,DIAMANT3);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==19)
	SpriteDaten(xz,DIAMANT4);
 return;
};
/***************************************************************************/
/* Bumms : Funktion fÅr die einzelnen Phasen des Explosionspilzes          */
/*-------------------------------------------------------------------------*/
/* Eingabe : xz = SpriteNummer					     	   */
/* Ausgabe : keine							   */
/***************************************************************************/
void Bumms (int xz)
{
 Objekte[(xz*4)+(Raum*19*4)+1]++;
 if (Objekte[(xz*4)+(Raum*19*4)+1]==6)
 {
  Objekte[(xz*4)+(Raum*19*4)+0]=0;
  SpriteDeactiv(xz);
 }
 if (Objekte[(xz*4)+(Raum*19*4)+1]==0)
	SpriteDaten(xz,BUMM1);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==2)
	SpriteDaten(xz,BUMM2);
 if (Objekte[(xz*4)+(Raum*19*4)+1]==4)
	SpriteDaten(xz,BUMM3);
 return;
};
/***************************************************************************/
/* WizRot : Funktion fÅr die einzelnen Phasen des Wiz-Rotators             */
/*-------------------------------------------------------------------------*/
/* Eingabe : xz = SpriteNummer					     	   */
/* Ausgabe : keine							   */
/***************************************************************************/
void WizRot(int xz)
{
 int x= Objekte[(xz*4)+(Raum*19*4)+2];	/* X-Koordinate			   */
 int y= Objekte[(xz*4)+(Raum*19*4)+3];	/* Y-Koordinate			   */
 int p= Objekte[(xz*4)+(Raum*19*4)+1];	/* Bewegungsphasen		   */

 if (p<50)				/* WizRot am Entstehen		   */
 {
  p++;
  if (p<43)   SpriteDaten(xz,LEER);
  if (p==49)  SpriteDaten(xz,GLITZER4);
  if (p==47)  SpriteDaten(xz,GLITZER3);
  if (p==45)  SpriteDaten(xz,GLITZER2);
  if (p==43)  SpriteDaten(xz,GLITZER1);
  Objekte[(xz*4)+(Raum*19*4)+1]=p;	/* neu Bewegungsphase		   */
  return;
 }

 int yalt=y;				/* alte X-Koordinate		   */

 if (p<54)				/* WizRot nach Oben     	   */
	y=y+2;
 else
	y=y-2;
 SpriteKoord(xz,x,y);
 SpUntergrund=SpriteUnt(xz);
 SpKollision =SpriteKoll(xz);
 if ((SpUntergrund!=0)||((SpKollision&0x40)==0x40)
	||((SpKollision&0x20)==0x20))
 {
  y=yalt;
  if (p<54)
	p=57;
  else
	p=50;
 }
 p=(SPhasenL[p-50]+50);
 SpriteDaten(xz,WIZROT[p-50]);
 SpriteKoord(xz,x,y);
 Objekte[(xz*4)+(Raum*19*4)+2]=x;	/* X-Koordinate			   */
 Objekte[(xz*4)+(Raum*19*4)+3]=y;	/* Y-Koordinate			   */
 Objekte[(xz*4)+(Raum*19*4)+1]=p;	/* Bewegungsphasen		   */

 return;
};

/***************************************************************************/
/* Slimer : Funktion fÅr die einzelnen Phasen von Slimer                   */
/*-------------------------------------------------------------------------*/
/* Eingabe : xz = Nummer des Sprites				     	   */
/* Ausgabe : keine							   */
/***************************************************************************/
void Slimer (int xz)
{
 int x= Objekte[(xz*4)+(Raum*19*4)+2];	/* X-Koordinate			   */
 int y= Objekte[(xz*4)+(Raum*19*4)+3];	/* Y-Koordinate			   */
 int p= Objekte[(xz*4)+(Raum*19*4)+1];	/* Bewegungsphasen		   */

 if (p<100)				/* Slimer am Entstehen		   */
 {
  p++;
  if (p<93)   SpriteDaten(xz,LEER);
  if (p==99)  SpriteDaten(xz,GLITZER4);
  if (p==97)  SpriteDaten(xz,GLITZER3);
  if (p==95)  SpriteDaten(xz,GLITZER2);
  if (p==93)  SpriteDaten(xz,GLITZER1);
  Objekte[(xz*4)+(Raum*19*4)+1]=p;	/* neu Bewegungsphase		   */
  return;
 }

 int xalt=x;				/* alte X-Koordinate		   */

 if (p<102)			/* Slimer nach links			   */
	x=x-4;
 else
	x=x+4;
 SpriteKoord(xz,x,y);
 SpUntergrund=SpriteUnt(xz);
 SpKollision =SpriteKoll(xz);
 if ((SpUntergrund==0)||((SpKollision&0x40)==0x40)
	||((SpKollision&0x20)==0x20))
 {
  x=xalt;
  if (p<102)
	p=102;
  else
	p=100;
 }
 p=(SPhasen[p-100]+100);
 SpriteKoord(xz,x,y);
 SpriteDaten(xz,SLIMER[p-100]);
 Objekte[(xz*4)+(Raum*19*4)+2]=x;	/* X-Koordinate			   */
 Objekte[(xz*4)+(Raum*19*4)+3]=y;	/* Y-Koordinate			   */
 Objekte[(xz*4)+(Raum*19*4)+1]=p;	/* Bewegungsphasen		   */

 return;
};

/***************************************************************************/
/* Gecko : Funktion fÅr die einzelnen Phasen von Gecko                     */
/*-------------------------------------------------------------------------*/
/* Eingabe : xz = Nummer des Sprites				     	   */
/* Ausgabe : keine							   */
/***************************************************************************/
void Gecko (int xz)
{
/*-- Daten holen							   */
 int x= Objekte[(xz*4)+(Raum*19*4)+2];	/* X-Koordinate			   */
 int y= Objekte[(xz*4)+(Raum*19*4)+3];	/* Y-Koordinate			   */
 int p= Objekte[(xz*4)+(Raum*19*4)+1];	/* Bewegungsphasen		   */
 int xalt=x;				/* alte X-Koordinate		   */

/*-- Phase kleiner als 100, dann Gecko am Einblenden			   */
 if (p<100)				/* Gecko am Entstehen		   */
 {
  p++;                                  /* Phase erhîhen		   */
  if (p<93)   SpriteDaten(xz,LEER);	/* keine Spielfigur		   */
  if (p==99)  SpriteDaten(xz,GLITZER4); /* mit Glitzern Einblenden         */
  if (p==97)  SpriteDaten(xz,GLITZER3);
  if (p==95)  SpriteDaten(xz,GLITZER2);
  if (p==93)  SpriteDaten(xz,GLITZER1);
  Objekte[(xz*4)+(Raum*19*4)+1]=p;	/* Bewegungsphase zurÅckschreiben  */
  return;
 }

 if (p<102)				/* Gecko nach links		   */
	x=x-4;
 else					/* Gecko nach rechts		   */
	x=x+4;

/*-- Spielfigur setzen							   */
 SpriteKoord(xz,x,y);

/*-- Umgebung abfragen							   */
 SpUntergrund=SpriteUnt(xz);
 SpKollision =SpriteKoll(xz);

/*-- Reaktion auf keinen Untergrund oder Spielrand oder RaumtÅr            */
 if ((SpUntergrund==0)||((SpKollision&0x40)==0x40)
	||((SpKollision&0x20)==0x20))
 {
  x=xalt;       			/* alte X-Koordinate zurÅckholen   */
  if (p<102)				/* Richtungswechsel		   */
	p=102;
  else
	p=100;
 }
 p=(SPhasen[(p-100)]+100);		/* neue Phase berechnen            */

/*-- Spielfigur setzen							   */
 SpriteKoord(xz,x,y);
 SpriteDaten(xz,GECKO[p-100]);

/*-- Daten zurÅckschreiben						   */
 Objekte[(xz*4)+(Raum*19*4)+2]=x;	/* X-Koordinate			   */
 Objekte[(xz*4)+(Raum*19*4)+3]=y;	/* Y-Koordinate			   */
 Objekte[(xz*4)+(Raum*19*4)+1]=p;	/* Bewegungsphasen		   */

 return;
};
/***************************************************************************/
/* Hexe : Funktion fÅr die einzelnen Phasen der Hexe                       */
/*-------------------------------------------------------------------------*/
/* Eingabe : xz = Nummer des Sprites				     	   */
/* Ausgabe : keine							   */
/***************************************************************************/
void Hexe (int xz)
{
/*-- Daten holen							   */
 int x= Objekte[(xz*4)+(Raum*19*4)+2];	/* X-Koordinate			   */
 int y= Objekte[(xz*4)+(Raum*19*4)+3];	/* Y-Koordinate			   */
 int p= Objekte[(xz*4)+(Raum*19*4)+1];	/* Bewegungsphasen		   */
 int xalt=x;				/* alte X-Koordinate		   */

/*-- Phase kleiner als 100, dann Gecko am Einblenden			   */
 if (p<150)				/* Hexe am Entstehen		   */
 {
  p++;                                  /* Phase erhîhen		   */
  if (p<143)  SpriteDaten(xz,LEER);	/* keine Spielfigur		   */
  if (p==149) SpriteDaten(xz,GLITZER4); /* mit Glitzern Einblenden         */
  if (p==147) SpriteDaten(xz,GLITZER3);
  if (p==145) SpriteDaten(xz,GLITZER2);
  if (p==143)  SpriteDaten(xz,GLITZER1);
  Objekte[(xz*4)+(Raum*19*4)+1]=p;	/* Bewegungsphase zurÅckschreiben  */
  return;
 }

 if (p<152)				/* Hexe nach links		   */
	x=x-4;
 else					/* Hexe nach rechts		   */
	x=x+4;

/*-- Spielfigur setzen							   */
 SpriteKoord(xz,x,y);

/*-- Umgebung abfragen							   */
 SpUntergrund=SpriteUnt(xz);
 SpKollision =SpriteKoll(xz);

/*-- Reaktion auf keinen Untergrund oder Spielrand oder RaumtÅr            */
 if ((SpUntergrund==0)||((SpKollision&0x40)==0x40)
	||((SpKollision&0x20)==0x20))
 {
  x=xalt;       			/* alte X-Koordinate zurÅckholen   */
  if (p<152)				/* Richtungswechsel		   */
	p=152;
  else
	p=150;
 }
 p=(SPhasen[(p-150)]+150);		/* neue Phase berechnen            */

/*-- Spielfigur setzen							   */
 SpriteKoord(xz,x,y);
 SpriteDaten(xz,HEXE[p-150]);

/*-- Daten zurÅckschreiben						   */
 Objekte[(xz*4)+(Raum*19*4)+2]=x;	/* X-Koordinate			   */
 Objekte[(xz*4)+(Raum*19*4)+3]=y;	/* Y-Koordinate			   */
 Objekte[(xz*4)+(Raum*19*4)+1]=p;	/* Bewegungsphasen		   */

 return;
};

/***************************************************************************/
/* Mauer : Funktion fÅr die geheimen Mauern                                */
/*-------------------------------------------------------------------------*/
/* Eingabe : xz = Nummer des Sprites				     	   */
/* Ausgabe : keine							   */
/***************************************************************************/
void Mauer (int xz)
{
/*-- Daten holen							   */
 int p= Objekte[(xz*4)+(Raum*19*4)+1];	/* Bewegungsphasen		   */

 if (p==0)				/* noch nicht geschaltet           */
 {
  SpriteDaten(xz,LEER);
  return;
 }

/*-- Mauer am Entstehen, nach BetÑtigung des Schalters			   */
 p++;                                   /* Phase erhîhen		   */
 if (p<5)   SpriteDaten(xz,LEER);	/* keine Spielfigur		   */
 if (p==11)  SpriteDaten(xz,GLITZER4);  /* mit Glitzern Einblenden         */
 if (p==9)  SpriteDaten(xz,GLITZER3);
 if (p==7)  SpriteDaten(xz,GLITZER2);
 if (p==5)  SpriteDaten(xz,GLITZER1);
 Objekte[(xz*4)+(Raum*19*4)+1]=p;	/* Bewegungsphase zurÅckschreiben  */
 if (p==13)
 {
  Objekte[(xz*4)+(Raum*19*4)+0]=26;	/* Mauer zeigen			   */
  SpriteDaten(xz,WAND3);
 }
 return;
};

/***************************************************************************/
/* Schalter : Funktion fÅr den Schalter der geheimen Mauern                */
/*-------------------------------------------------------------------------*/
/* Eingabe : xz = Nummer des Sprites				     	   */
/* Ausgabe : keine							   */
/***************************************************************************/
void Schalter (int xz)
{
/*-- Daten holen							   */
 int p= Objekte[(xz*4)+(Raum*19*4)+1];	/* Bewegungsphasen		   */

 SpriteDaten(xz,SCHALTER2);
 if (p==0)				/* noch nicht geschaltet           */
 {
  SpriteDaten(xz,SCHALTER1);
 }
 return;
};

/***************************************************************************/
/* Point : Funktion fÅr die einzelnen Phasen von beweglichen Punkten       */
/*-------------------------------------------------------------------------*/
/* Eingabe : xz = Nummer des Sprites				     	   */
/* Ausgabe : keine							   */
/***************************************************************************/
void Point (int xz)
{
 int x= Objekte[(xz*4)+(Raum*19*4)+2];	/* X-Koordinate			   */
 int y= Objekte[(xz*4)+(Raum*19*4)+3];	/* Y-Koordinate			   */
 int p= Objekte[(xz*4)+(Raum*19*4)+1];	/* Phase			   */

/*-- Y-Koordinate nach oben						   */
 y=y-1;

/*-- Phase erhîhen							   */
 p++;

/*-- Sprite setzen							   */
 SpriteKoord(xz,x,y);

/*-- ÅberprÅfen, ob schon au·erhalb des Bildschirmes oder schon 20x        */
/*   nach oben gestiegen						   */
 if ((y<-21)||(p>20))
 {
  Objekte[(xz*4)+(Raum*19*4)+0]=0;	/* wenn au·erhalb, dann ausblenden */
  SpriteDeactiv(xz);
 }

/*-- Koordinaten zurÅckschreiben					   */
 Objekte[(xz*4)+(Raum*19*4)+2]=x;	/* X-Koordinate			   */
 Objekte[(xz*4)+(Raum*19*4)+3]=y;	/* Y-Koordinate			   */
 Objekte[(xz*4)+(Raum*19*4)+1]=p;	/* Phasen			   */
 return;
};

/***************************************************************************/
/* Bild_Aufbau : baut das Spielfeld im Hintergrund auf und blendet es ein  */
/*-------------------------------------------------------------------------*/
/* Eingabe     : keine                                                     */
/* Ausgabe     : keine                                                     */
/***************************************************************************/
void Bild_Aufbau(void)
{
 int xz;
/*-- unbenutzten Hintergrund kopieren                                      */
 blockmove(Seite_hinten,Seite_schreiben,0,0,0,0,320,200);

/*-- Spiel-Objekte setzten ------------------------------------------------*/
 if (Gameover==2) goto Bild4;
 for (xz=0;xz<19;xz++)
 {
  Objekt=Objekte[(xz*4)+(Raum*19*4)];
  if (Objekt==1)  Blume(xz)      ;
  if (Objekt==2)  Band_links(xz) ;
  if (Objekt==3)  Band_rechts(xz);
  if (Objekt==7)  Slimer(xz)     ;
  if (Objekt==8)  Point(xz)      ;		/* bewegliche 100-Punkte   */
  if (Objekt==9)  Point(xz)      ;		/* bewegliche  20-Punkte   */
  if (Objekt==10) Point(xz)      ;		/* bewegliche  50-Punkte   */
  if (Objekt==11) Kata(xz)       ;
  if (Objekt==12) Bumms(xz)      ;		/* Explosion               */
  if (Objekt==15) Diaman(xz)     ;		/* Diamantenglitzern	   */
  if (Objekt==16) Point(xz)      ;		/* bewegliche  500-Punkte  */
  if (Objekt==21) Point(xz)      ;		/* beweglicher Key-Punkt   */
  if (Objekt==32) Gecko(xz)	 ;
  if (Objekt==33) Schalter(xz)	 ;		/* SchalterfÅr Mauer       */
  if (Objekt==34) Mauer(xz)	 ;		/* geheime Mauer	   */
  if (Objekt==35) Fall(xz)       ;              /* Wasserfall              */
  if (Objekt==36) Wasser(xz)     ;              /* Burggraben              */
  if (Objekt==37) WizRot(xz)     ;              /* Wiz-Rotator             */
  if (Objekt==38) Hexe(xz)       ;              /* Hexe laufend            */

 }


/*-- Verhalten bei GameOver						   */
Bild4:
 if (Gameover==1)
 {
  for (xz=0;xz<19;xz++)
  {
   SpriteDeactiv(xz);
  }
  SpriteDaten(1,OVER1);
  SpriteDaten(2,OVER2);
  SpriteDaten(3,OVER3);
  SpriteDaten(4,OVER4);
  SpriteKoord(1,70,70);
  SpriteKoord(2,94,70);
  SpriteKoord(3,118,70);
  SpriteKoord(4,142,70);
  SpriteActiv(1);
  SpriteActiv(2);
  SpriteActiv(3);
  SpriteActiv(4);
  Gameover=2;
  GameZaehler=15;
  GameoverZustand=0;
 }

 if (Gameover==2)
 {
  if (GameZaehler==0) goto Bild6;
  GameZaehler--;
  goto Bild5;
Bild6:
  if ((Taste&0x80)==0x80) Taste=0x20;  		/* SPACE-Druck abwarten   */
Bild5:
 }

/*-- ObjektSprites der Spielebene zeichnen				  */
 for (int z=1;z<13;z++)
 {
  Sprite(z);
 }

/*-- Start- und Endverhalten Kuno -----------------------------------------*/
 if (KEnde != 0)
 {
  if (KEnde==8 ) KZustand=14;
  if (KEnde==6 ) KZustand=15;
  if (KEnde==4 ) KZustand=16;
  if (KEnde==2 ) KZustand=17;
  if (KEnde==1 )
  {
   KZustand=18;
   KBeginn=8;
   KunoX=KunoXStart;
   KunoY=KunoYStart;
  }
  KEnde--;
  goto Bild3;

 }
 if (KBeginn!= 0)
 {
  if (KBeginn==8 ) KZustand=10;
  if (KBeginn==6 ) KZustand=11;
  if (KBeginn==4 ) KZustand=12;
  if (KBeginn==2 ) KZustand=13;
  KBeginn--;
  goto Bild3;
 }
/*-- Abfrage auf Untergrund von Kuno                                       */
 if (KSprung!=0) goto Bild1;			/* Sprung => keine Abfrage */
 if (KPult  !=0) goto Bild1;
 SpKollision =SpriteKoll(Kuno);
 SpUntergrund=SpriteUnt (Kuno);

 if ((SpUntergrund==0))			/* kein Untergrund=> fallen        */
 {
  KFall--;					/* FallZeiger -1           */
  if (KFall==0) KFall=-1;		/* Umgehen von "LuftsprÅngen"	   */
  KunoY=KunoY+4;                                      /* Kuno 2 tiefer 	   */
 }
 if ((SpUntergrund>0))				/* Untergrund da           */
 {
  if ((SpUntergrund&0x10)==0x10)	/* Untergrund ist ein Sprite	   */
  {
   Objekt=Objekte[(Spritekollision()*4)+(Raum*19*4)];
   if (Objekt==2)                       /* welches Sprite?                 */
   {
    KunoX=KunoX-4;			/* Untergrund=Band nach links      */
    if (KFall<=-10) Power(-2);          /* Falltiefe berechnen		   */
    KFall=0;				/* Fall beendet		           */
    KPult=0;				/* Katapultsprung beenden	   */
    goto Sprit;				/* Ende Abfrage			   */
   }
   if (Objekt==26)                      /* welches Sprite?                 */
   {
    KunoX=KunoX;			/* Untergrund=Geheime Mauer        */
    if (KFall<=-10) Power(-2);          /* Falltiefe berechnen		   */
    KFall=0;				/* Fall beendet		           */
    KPult=0;				/* Katapultsprung beenden	   */
    goto Sprit;				/* Ende Abfrage			   */
   }

   if (Objekt==3)
   {
    KunoX=KunoX+4;       		/* Untergrund=Band nach rechts     */
    if (KFall<=-10) Power(-2);		/* Falltiefe berechnen		   */
    KFall=0;				/* Fall beendet	           	   */
    KPult=0;                            /* Katapultsprung beenden          */
    goto Sprit;				/* Ende Abfrage			   */
   }
   if (Objekt==28)			/* Tor blau			   */
   {
    if (KFall<=-10) Power(-2);		/* Falltiefe berechnen		   */
    KFall=0;				/* Fall beendet	           	   */
    KPult=0;                            /* Katapultsprung beenden          */
    goto Sprit;				/* Ende Abfrage			   */
   }
   if (Objekt==29)			/* Tor gelb			   */
   {
    if (KFall<=-10) Power(-2);		/* Falltiefe berechnen		   */
    KFall=0;				/* Fall beendet	           	   */
    KPult=0;                            /* Katapultsprung beenden          */
    goto Sprit;				/* Ende Abfrage			   */
   }
   if (Objekt==30)			/* Tor rot			   */
   {
    if (KFall<=-10) Power(-2);		/* Falltiefe berechnen		   */
    KFall=0;				/* Fall beendet	           	   */
    KPult=0;                            /* Katapultsprung beenden          */
    goto Sprit;				/* Ende Abfrage			   */
   }
   if (Objekt==31)			/* Tor grÅn			   */
   {
    if (KFall<=-10) Power(-2);		/* Falltiefe berechnen		   */
    KFall=0;				/* Fall beendet	           	   */
    KPult=0;                            /* Katapultsprung beenden          */
    goto Sprit;				/* Ende Abfrage			   */
   }
   if (Objekt==11)                      /* Untergrund=Katapult             */
   {
    if (KFall<=-10) Power(-2); 		/* Falltiefe berechnen		   */
    KFall=0;				/* Fall beendet	        	   */
    KPult=1;      			/* Untergrund=Katapult             */
    goto Sprit3;                        /* Ende Abfrage			   */
   }
			/*-- Untergrund-Sprite soll nicht beachtet werden  */
/*-- ist dort noch ein anderer Untergrund?                                 */
			/*-- Sprite-Abfrage raus, da schon abgefragt       */
   SpUntergrund=SpUntergrund&0xEF;
   if ((SpUntergrund==0))			/* kein Untergrund=> fallen        */
   {
    KFall--;					/* FallZeiger -1           */
    if (KFall==0) KFall=-1;		/* Umgehen von "LuftsprÅngen"	   */
    KunoY=KunoY+4;                                      /* Kuno 2 tiefer 	   */
   }

   goto Sprit2;
/*-- Ende der Untergrundabfrage						   */
Sprit:
  }
  KPult=0;				/* Katapultsprung beenden	   */
Sprit3:
  if (KFall<=-10) Power(-2);  	        /* Falltiefe berechnen		   */
  KFall=0;      			/* Fall beendet		           */
Sprit2:
 }

Bild1:
/*-- Abfrage Kuno-Steuerung -----------------------------------------------*/
 Kuno_Steuerung();


Bild2:
/*-- setzen der neuen Werte und Umgebung holen ----------------------------*/
 SpriteKoord(Kuno,KunoX,KunoY);			/* Setzen der neuen Koord  */
 SpUntergrund=SpriteUnt(Kuno);
 SpKollision =SpriteKoll(Kuno);

/*-- Abfrage Kollision Kuno -----------------------------------------------*/
 if (SpKollision!=0) Kuno_Reak();

/*-- Reaktion auf Kollision                                                */

Bild3:
/*-- Zeichnen des Sprite --------------------------------------------------*/
 if (KunoY<0) KunoY=KunoYalt;			/* verhindert das Ver-     */
 if (KunoX<-4) KunoX=KunoXalt;			/*  schwinden von Kuno aus */
						/*  dem Bildschirm         */
 SpriteKoord(Kuno,KunoX,KunoY);			/* endgÅltige Koordinaten  */
 SpriteDaten(Kuno,KPhasen[KZustand]);
 Sprite(Kuno);

/*-- ObjektSprites der Vordergrundebene zeichnen			   */
 for (z=13;z<19;z++)
 {
  Sprite(z);
 }

 return;
}

/***************************************************************************/
/* Spiel_laden : stellt ein altes Spiel wieder her                         */
/*-------------------------------------------------------------------------*/
/* Eingabe     : keine                                                     */
/* Ausgabe     : keine                                                     */
/***************************************************************************/
void Spiel_laden( void )
{
 return;
}

/***************************************************************************/
/* Spiel_Anfang : initialisiert ein neues Spiel                            */
/*-------------------------------------------------------------------------*/
/* Eingabe      : keine                                                    */
/* Ausgabe      : keine                                                    */
/***************************************************************************/
void Spiel_Anfang( void )
{
 Level=0;
 Raum =0;
 KunoX=220;     		/* Startposition von Kuno		   */
 KunoY=123;
 KunoXStart=KunoX;
 KunoYStart=KunoY;
 KZustand=0;			/* Kuno blickt nach links                  */
 KBeginn=8;			/* Kuno soll eingeblendet werden	   */
 KEnde=0;			/* Ende zurÅcksetzen			   */
 Kraft=45;
 KPult=0;
 Wappen =0;
 Schwert=0;
 Diamanten=0;
 Keyrot=0;
 Keygruen=0;
 Keyblau=0;
 Keygelb=0;
 Leben  ="5";
 PUNKTE=0;
 Key();
 Diamant(0);			/* Anzeiger der Diamanten		   */
 Punkte(0 );			/* Anzeige der Punkte			   */
 Power(0);			/* Anzeige Kraft			   */
 Sword(0);			/* Anzeige der Anzahl der Schwerter 	   */
 Wapp(0);			/* Anzeige der Anzahl der Wappen	   */
/*-- kopieren der Levelobjekte in das Objektspielfeld                      */
 Levelladen(Level,LevelDaten,Objekt0);
 for (int xz=0;xz<19*10*4;xz++)
 {
  Objekte[(xz)]= Objekt0[(xz)];
/*  Objekte[(xz)]= ObjektNr[Level][(xz)];*/
 }
 SpriteDeactiv(Kuno);		/* sicher ist sicher			   */
 SpriteDaten(Kuno,KPhasen[KZustand]);
 SpriteKoord(Kuno,KunoX,KunoY);
 SpriteActiv(Kuno);
 return;
}

/***************************************************************************/
/* Tab_Zeichen : wandelt den BS-Code in einen Levelcode                    */
/*-------------------------------------------------------------------------*/
/* Eingabe : Zeichen = BS-Code (zB FF oder 0A usw)                         */
/* Ausgabe : Levelcode                                                     */
/***************************************************************************/
unsigned char Tab_Zeichen (unsigned char Zeichen)
{
/*-- Vergleich mit der Umrechnungstabelle                                  */
 return (Tabchar[Zeichen]);
}



/***************************************************************************/
/* Spiel   : Eigentliches Spielprogramm                                    */
/*-------------------------------------------------------------------------*/
/* Eingabe : Fortfahren = altes Spiel weiterspielen (0 - nein, 1 - ja)     */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void Spiel(unsigned char Fortfahren)
{
   int X;


/*-- Setzen der Bildschirm-Variablen auf Anfang                            */
 Seite_zeigen   =0;
 Seite_schreiben=1;

/*-- Hintergrund laden und anzeigen                                        */
 setpage(Seite_hinten);
 TGA_laden("rg4.tga");
 showpage(Seite_hinten);
 Bild_aufblenden();
 setpage(Seite_schreiben);


/*-- Auswertung der Parameter                                              */
 if (Fortfahren==1)
 {
  Spiel_laden();			/* altes Spiel wiederherstellen    */
 }
 else
 {
  Spiel_Anfang();			/* neues Spiel beginnen            */
 }
/*-- Level anzeigen							   */
 Level_zeigen();

/*-- Spielschleife --------------------------------------------------------*/
weiter:
 KunoXalt=KunoX;			/* Position merken		   */
 KunoYalt=KunoY;
 Bild_Aufbau();
 Bild_zeigen();

/*-- Abfrage, ob die Taste ESC gedrÅckt wurde				   */
 if ((Taste&0x20) != 0x20)
	goto weiter;			/* ESC nicht gedrÅckt		   */


Ende:

 setpage(Seite_hinten);
 Bild_ausblenden();
 return;
}

/***************************************************************************/
/* Auswahl : Auswahlprogramm fÅr MenÅsteuerung                             */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine                                                         */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void Auswahl(void)
{

/*-- Setzen der Bildschirm-Variablen auf Anfang                            */
 Seite_zeigen   =0;
 Seite_schreiben=1;

/*-- Hintergrund laden und anzeigen                                        */
Beginn:
 setpage(Seite_Reserve);
 TGA_laden("rg3.tga");
 showpage(Seite_Reserve);
 Bild_aufblenden();
 setpage(Seite_schreiben);


 KunoX=40;     		/* Startposition von Kuno		   */
 KunoY=53;
 KunoYStart=KunoY;
 SpriteDeactiv(Kuno);		/* sicher ist sicher			   */
 SpriteDaten(Kuno,KRG1);
 SpriteKoord(Kuno,KunoX,KunoY);
 SpriteActiv(Kuno);

weiter:
 SpriteKoord(Kuno,KunoX,KunoY);
 blockmove(Seite_Reserve,Seite_schreiben,0,0,0,0,320,200);
 Sprite(Kuno);


/*-- Abfrage, ob die Taste ESC gedrÅckt wurde				   */

 Bild_zeigen();
 if ((Taste&0x20) == 0x20)
 {	goto Ende ;			/* ESC gedrÅckt			   */
 }
 if ((Taste&0x08) == 0x08)		/* Taste oben			   */
 {
	KunoY=KunoY-30;
	if (KunoY<KunoYStart) KunoY=KunoYStart+90;
 }
 if ((Taste&0x04) == 0x04)		/* Taste unten 			   */
 {
	KunoY=KunoY+30;
	if (KunoY>KunoYStart+90) KunoY=KunoYStart;
 }

 if ((Taste&0x80) == 0x80)
 {
    if (KunoY==53)
    {
	Spiel(0);
	goto Beginn;

    }
    if (KunoY==143) goto Ende;

 }
 goto weiter;


 Ende:

 return;
}

/***************************************************************************/
/* SpeicherRese : Reservierung von Speicher fÅr Sprites usw.               */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine                                                         */
/* Ausgabe : Erfolg = 0 - nicht genÅgend Speicher vorhanden                */
/*                  > 0 - Erfolgreiche Reservierung                        */
/***************************************************************************/
BYTE SpeicherRese(VOID)
{
  ULONG Erfolg=1;
/*-- Sprite-Speicher reservieren -------------------------------------------*/
  SpriteBeginn=(PCHAR) farmalloc(24*21*30);
  if(SpriteBeginn==NULL)
  {
   Erfolg=0;
   goto Misserfolg;
  }

/*-- Sprite-Speicher den einzelnen Sprites zuordnen ------------------------*/
 WIZROT1=SpriteBeginn+(24*21*0);
 WIZROT2=SpriteBeginn+(24*21*1);
 WIZROT3=SpriteBeginn+(24*21*2);
 WIZROT4=SpriteBeginn+(24*21*3);
 HEXERE1=SpriteBeginn+(24*21*4);
 HEXERE2=SpriteBeginn+(24*21*5);
 HEXELI1=SpriteBeginn+(24*21*6);
 HEXELI2=SpriteBeginn+(24*21*7);
 KLINKS1=SpriteBeginn+(24*21*8);
 KLINKS2=SpriteBeginn+(24*21*9);
 KRECHTS1=SpriteBeginn+(24*21*10);
 KRECHTS2=SpriteBeginn+(24*21*11);
 KLINKSSPR=SpriteBeginn+(24*21*12);
 KLINKSST=SpriteBeginn+(24*21*13);
 KRECHTSSPR=SpriteBeginn+(24*21*14);
 KRECHTSST=SpriteBeginn+(24*21*15);
 KLEITER1=SpriteBeginn+(24*21*16);
 KLEITER2=SpriteBeginn+(24*21*17);
 KBEGINN1=SpriteBeginn+(24*21*18);
 KBEGINN2=SpriteBeginn+(24*21*19);
 KBEGINN3=SpriteBeginn+(24*21*20);
 KBEGINN4=SpriteBeginn+(24*21*21);
 SKELETT1=SpriteBeginn+(24*21*22);
 SKELETT2=SpriteBeginn+(24*21*23);
 SKELETT3=SpriteBeginn+(24*21*24);
 KRG1=SpriteBeginn+(24*21*25);
 BAND1=SpriteBeginn+(24*21*26);
 BAND2=SpriteBeginn+(24*21*27);
 BAND3=SpriteBeginn+(24*21*28);
 BAND4=SpriteBeginn+(24*21*29);

 ObjTab[0]=(PCHAR) KLINKS1;             /* Objekt-Tabelle zuordnen          */
 ObjTab[2]=(PCHAR) BAND1;
 ObjTab[3]=(PCHAR) BAND1;
 ObjTab[37]=(PCHAR) WIZROT1;
 ObjTab[38]=(PCHAR) HEXERE1;

 KPhasen[0]= (PCHAR) KLINKS1;		/* Kunos BewegungsPhasen zuordnen   */
 KPhasen[1]= (PCHAR) KLINKS2;
 KPhasen[2]= (PCHAR) KRECHTS1;
 KPhasen[3]= (PCHAR) KRECHTS2;
 KPhasen[4]= (PCHAR) KLINKSSPR;
 KPhasen[5]= (PCHAR) KRECHTSSPR;
 KPhasen[6]= (PCHAR) KLINKSST;
 KPhasen[7]= (PCHAR) KRECHTSST;
 KPhasen[8]= (PCHAR) KLEITER1;
 KPhasen[9]= (PCHAR) KLEITER2;
 KPhasen[10]= (PCHAR) KBEGINN1;
 KPhasen[11]= (PCHAR) KBEGINN2;
 KPhasen[12]= (PCHAR) KBEGINN3;
 KPhasen[13]= (PCHAR) KBEGINN4;
 KPhasen[14]= (PCHAR) KBEGINN4;
 KPhasen[15]= (PCHAR) SKELETT1;
 KPhasen[16]= (PCHAR) SKELETT2;
 KPhasen[17]= (PCHAR) SKELETT3;
 KPhasen[18]= LEER;

 WIZROT[0]=(PCHAR)WIZROT1;
 WIZROT[1]=(PCHAR)WIZROT2;
 WIZROT[2]=(PCHAR)WIZROT3;
 WIZROT[3]=(PCHAR)WIZROT4;
 WIZROT[4]=(PCHAR)WIZROT4;
 WIZROT[5]=(PCHAR)WIZROT3;
 WIZROT[6]=(PCHAR)WIZROT2;
 WIZROT[7]=(PCHAR)WIZROT1;

 HEXE[2]=(PCHAR) HEXERE1;		/* Hexe-Phasen zurodnen            */
 HEXE[3]=(PCHAR) HEXERE2;
 HEXE[0]=(PCHAR) HEXELI1;
 HEXE[1]=(PCHAR) HEXELI2;

Misserfolg:
  return(Erfolg);
}

/***************************************************************************/
/* SpeicherFrei : Freigabe des reservierten Speichers                      */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine                                                         */
/* Ausgabe : keine                                                         */
/***************************************************************************/
VOID SpeicherFrei(VOID)
{
 farfree(SpriteBeginn);
 return;
}

/***************************************************************************/
/* LoadSpielSprites : lÑdt die Sprites und Texturen			   */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine						           */
/* Ausgabe : keine							   */
/***************************************************************************/
void LoadSpielSprites (void)
{
/*-- Textur laden      							   */
 Charladen("level_z.tga",Levelz,37);

/*-- ZeichensÑtze laden							   */
 Zeichenladen();


/*-- Sprites laden							   */
 Spriteladen("klinks1 .tga",KLINKS1   );
 Spriteladen("klinks2 .tga",KLINKS2   );
 Spriteladen("krechts1.tga",KRECHTS1  );
 Spriteladen("krechts2.tga",KRECHTS2  );
 Spriteladen("kbeginn1.tga",KBEGINN1  );
 Spriteladen("kbeginn2.tga",KBEGINN2  );
 Spriteladen("kbeginn3.tga",KBEGINN3  );
 Spriteladen("kbeginn4.tga",KBEGINN4  );
 Spriteladen("klist   .tga",KLINKSST  );
 Spriteladen("krest   .tga",KRECHTSST );
 Spriteladen("klispr  .tga",KLINKSSPR );
 Spriteladen("krespr  .tga",KRECHTSSPR);
 Spriteladen("kleiter1.tga",KLEITER1  );
 Spriteladen("kleiter2.tga",KLEITER2  );
 Spriteladen("band1   .tga",BAND1     );
 Spriteladen("band2   .tga",BAND2     );
 Spriteladen("band3   .tga",BAND3     );
 Spriteladen("band4   .tga",BAND4     );
 Spriteladen("slimer1 .tga",SLIMER1   );
 Spriteladen("slimer2 .tga",SLIMER2   );
 Spriteladen("slimer3 .tga",SLIMER3   );
 Spriteladen("slimer4 .tga",SLIMER4   );
 Spriteladen("blume1  .tga",BLUME1    );
 Spriteladen("blume2  .tga",BLUME2    );
 Spriteladen("blume3  .tga",BLUME3    );
 Spriteladen("blume4  .tga",BLUME4    );
 Spriteladen("katap1  .tga",KATAPULT1 );
 Spriteladen("katap2  .tga",KATAPULT2 );
 Spriteladen("katap3  .tga",KATAPULT3 );
 Spriteladen("schwert .tga",SCHWERT   );
 Spriteladen("sack    .tga",SACK      );
 Spriteladen("wappen  .tga",WAPPEN    );
 Spriteladen("punkt100.tga",POINT100  );
 Spriteladen("punkt50 .tga",POINT50   );
 Spriteladen("punkt20 .tga",POINT20   );
 Spriteladen("over1   .tga",OVER1     );
 Spriteladen("over2   .tga",OVER2     );
 Spriteladen("over3   .tga",OVER3     );
 Spriteladen("over4   .tga",OVER4     );
 Spriteladen("bumm1   .tga",BUMM1     );
 Spriteladen("bumm2   .tga",BUMM2     );
 Spriteladen("bumm3   .tga",BUMM3     );
 Spriteladen("ralf    .tga",RALF      );
 Spriteladen("health  .tga",HEALTH    );
 Spriteladen("disk    .tga",DISK      );
 Spriteladen("diamant1.tga",DIAMANT1  );
 Spriteladen("diamant2.tga",DIAMANT2  );
 Spriteladen("diamant3.tga",DIAMANT3  );
 Spriteladen("diamant4.tga",DIAMANT4  );
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
 Spriteladen("gecko2  .tga",GECKO2    );
 Spriteladen("gecko3  .tga",GECKO3    );
 Spriteladen("gecko4  .tga",GECKO4    );
 Spriteladen("glitzer1.tga",GLITZER1  );
 Spriteladen("glitzer2.tga",GLITZER2  );
 Spriteladen("glitzer3.tga",GLITZER3  );
 Spriteladen("glitzer4.tga",GLITZER4  );
 Spriteladen("leer    .tga",LEER      );
 Spriteladen("schalt1 .tga",SCHALTER1 );
 Spriteladen("schalt2 .tga",SCHALTER2 );
 Spriteladen("fall1   .tga",FALL1     );
 Spriteladen("fall2   .tga",FALL2     );
 Spriteladen("fall3   .tga",FALL3     );
 Spriteladen("wasser1 .tga",WASSER1   );
 Spriteladen("wasser2 .tga",WASSER2   );
 Spriteladen("wasser3 .tga",WASSER3   );
 Spriteladen("wasser4 .tga",WASSER4   );
 Spriteladen("wasser5 .tga",WASSER5   );
 Spriteladen("wasser6 .tga",WASSER6   );
 Spriteladen("wasser7 .tga",WASSER7   );
 Spriteladen("wasser8 .tga",WASSER8   );
 Spriteladen("skel-1  .tga",SKELETT1  );
 Spriteladen("skel-2  .tga",SKELETT2  );
 Spriteladen("skel-3  .tga",SKELETT3  );
 Spriteladen("wizrot1 .tga",WIZROT1   );
 Spriteladen("wizrot2 .tga",WIZROT2   );
 Spriteladen("wizrot3 .tga",WIZROT3   );
 Spriteladen("wizrot4 .tga",WIZROT4   );
 Spriteladen("krg1    .tga",KRG1      );
 Spriteladen("hexeli1 .tga",HEXELI1   );
 Spriteladen("hexeli2 .tga",HEXELI2   );
 Spriteladen("hexere1 .tga",HEXERE1   );
 Spriteladen("hexere2 .tga",HEXERE2   );

 return;
}