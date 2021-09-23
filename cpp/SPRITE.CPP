/***************************************************************************/
/* S P R I T E . C P P                                                     */
/*-------------------------------------------------------------------------*/
/* Aufgabe       :  Quellcode fÅr die Sprite-Routinen fÅr                  */
/*                  K U N O   -   D E R   R I T T E R			   */
/*-------------------------------------------------------------------------*/
/* Autor         :  Ralf Guder						   */
/* Arbeitsbeginn :  30.01.96						   */
/* Arbeitsende   :                                                         */
/* letztes Update:  20.02.96                                               */
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
#include <dos.h>
#include "kuno.h"

/*-- externe Funktionen ---------------------------------------------------*/

/*   Modul "SPIEL.CPP"							   */
unsigned char Tab_Zeichen (unsigned char);	/* liefert Levelcode       */

/*-- Module-Variablen -----------------------------------------------------*/
unsigned char Sprkoll;				/* Spritekollision         */
unsigned char Tuerkoll;				/* Kollision mit TÅr       */
unsigned char Kollision; 			/* _ _ _ _  _ _ _ _        */
						/* L R T S  M              */
						/* E A ö P  A              */
						/* I N R R  U              */
						/* T D   I  E              */
						/* E     T  R              */
						/* R     E                 */
unsigned char Untergrund;           		/* _ _ _ _  _ _ _ _        */
						/* L R   S  M              */
						/* E A   P  A              */
						/* I N   R  U              */
						/* T D   I  E              */
						/* E     T  R              */
						/* R     E                 */

const char SpYL=21;				/* Hîhe eines Sprite       */
const char SpXL=24;				/* Breite eines Sprite     */
const char SpZ =20;				/* max. Anzahl Sprites     */


unsigned char *SpZeiger[SpZ];			/* Spritepuffer            */
int SpX[SpZ];
int SpY[SpZ];
int SpXa[SpZ];
int SpYa[SpZ];
int SpActiv[SpZ];


/***************************************************************************/
/* SpriteKoord : Sprite-Koordinaten Åbernehmen 				   */
/*-------------------------------------------------------------------------*/
/* Eingabe     : SpriteNr = Nummer des Sprites                             */
/*               SpriteX  = X-Koordinate des Sprites                       */
/*               SpriteY  = Y-Koordinate des Sprites                       */
/* Ausgabe     : keine                                                     */
/***************************************************************************/
void SpriteKoord (int SpriteNr, int SpriteX, int SpriteY)
{
  SpX[SpriteNr]=SpriteX;
  SpY[SpriteNr]=SpriteY;
  return;
}

/***************************************************************************/
/* SpriteFarbe : legt die 4 Farben des gewÑhlten Sprites fest              */
/*-------------------------------------------------------------------------*/
/* Eingabe : SpriteNr = Nummer des gewÑhlten Sprites                       */
/*           Farbe    = Zeiger auf Farbarray                               */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void SpriteFarbe (int SpriteNr, unsigned char *Farbe)
{
 return;
}

/***************************************************************************/
/* SpriteDaten : Sprite-Bilddaten Åbernehmen				   */
/*-------------------------------------------------------------------------*/
/* Eingabe : SpriteNr     = Nummer des Sprites                             */
/*           SpriteZeiger = Zeiger auf das Datenarray des Sprites          */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void SpriteDaten (int SpriteNr, unsigned char *SpriteZeiger)
{
  SpZeiger[SpriteNr] = SpriteZeiger;
  return;
}

/***************************************************************************/
/* SpriteActiv : Sprite-Aktivierung                                        */
/*-------------------------------------------------------------------------*/
/* Eingabe : SpriteNr = Nummer des Sprite, das aktiviert werden soll       */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void SpriteActiv (int SpriteNr)
{
  SpActiv[SpriteNr]=1;
  SpYa[SpriteNr]=SpY[SpriteNr];
  SpXa[SpriteNr]=SpX[SpriteNr];
  return;
}

/***************************************************************************/
/* SpriteDeactiv : Sprite-Deaktivierung                                    */
/*-------------------------------------------------------------------------*/
/* Eingabe : SpriteNr = Nummer des Sprite, das deaktiviert werden soll     */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void SpriteDeactiv(int SpriteNr)
{
  SpActiv[SpriteNr]=0;
  return;
}


/***************************************************************************/
/* Sprite : Zeichnen aller aktiven Sprites in die Arbeitsseite             */
/*-------------------------------------------------------------------------*/
/* Eingabe : SpriteNr = Nummer des Sprite, das gezeichnet werden soll      */
/* Ausgabe : keine                                                         */
/***************************************************************************/
void Sprite (int SpriteNr)
{
/*-- Benîtigte Variablen						   */
 int  c;
 int  x;
 int  y;
 int  z=SpriteNr;
 unsigned char Punkt;
 unsigned char* Sprites;

 Sprites=SpZeiger[SpriteNr];
/*-- Sprite abfragen und bei Bedarf Zeichnen				   */
 if (SpActiv[z]==1)				/* Sprite aktiv ?          */
 {
  SpXa[z]=SpX[z];
  SpYa[z]=SpY[z];

// Sprite zeichnen
  c=0;
  for (y=SpY[z]; y<SpY[z]+SpYL; y++)
   for (x=SpX[z]; x<SpX[z]+SpXL; x++)
   {
    if (y>=000)
     if (y<=199)
      if (x>=000)
       if(x<=319)
       {
	Punkt=Sprites[c];
	if (Punkt < 0xff)
	{
	 if (Punkt==0) Punkt=SpriteNr+128;		/* SpriteNr-Farbe einbinden*/
	 setpix(x,y,Punkt);
	}
       }
    c++;
   }
  }

  return;
}

/***************************************************************************/
/* Tuerkollision : liefert bei Bedarf die berÅhrte TÅr                     */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine						           */
/* Ausgabe : berÅhrte TÅr						   */
/***************************************************************************/
int Tuerkollision (void)
{
 return(Tuerkoll);
}

/***************************************************************************/
/* Spritekollision : liefert bei Bedarf das berÅhrte Sprite                */
/*-------------------------------------------------------------------------*/
/* Eingabe : keine                                                         */
/* Ausgabe : berÅhrtes Sprite                                              */
/***************************************************************************/
int Spritekollision(void)
{
 return(Sprkoll);
}

/***************************************************************************/
/* KoordTest: Testet die Koordinaten auf BS-Tauglichkeit                   */
/*-------------------------------------------------------------------------*/
/* Eingabe : X,Y = Koordinaten                                             */
/* Ausgabe : TRUE oder FALSE                                               */
/***************************************************************************/
 BYTE KoordTest(INT x,INT y)
 {
 if (((y>=000)&&(y<=199))&&((x>=000)&&(x<=319))) return(TRUE);
 return(FALSE);
 }

/***************************************************************************/
/* SpriteKoll : liefert den Kollisionscode des Sprites                     */
/*-------------------------------------------------------------------------*/
/* Eingabe : SpriteNr = Nummer des Sprite, das gezeichnet werden soll      */
/* Ausgabe : Kollisionscode                                                */
/***************************************************************************/
int SpriteKoll (int SpriteNr)
{
/*-- Benîtigte Variablen						   */
 int  c;
 int  x;
 int  y;
 int  z=SpriteNr;
 unsigned char Punkt;
 BYTE LevPkt;
 unsigned char *Sprites;
 Sprites=SpZeiger[SpriteNr];
 Tuerkoll=0;				/* Kollision mit TÅr		   */
 Sprkoll=0;				/* Kollision mit Sprite            */
 Kollision =0;				/* Kollision generell 0000 0000    */

/*-- Sprite abfragen                     				   */
 if (SpActiv[z]==1)				/* Sprite aktiv ?          */
 {

/*-- Bildschirm abfragen						   */
  c=0;
  for (y=SpY[z]; y<SpY[z]+SpYL; y++)
  {
   for (x=SpX[z]; x<SpX[z]+SpXL; x++)
   {
    if (KoordTest(x,y)==TRUE)
    {
	Punkt=Sprites[c];
	if (Punkt < 0xff)			/* nur wo Sprite gezeichnet*/
	{
	 Punkt=getpix(x,y);
	 if ((Punkt>128)&&(Punkt<148))		/* Sprite                  */
	 {
	  Sprkoll=Punkt-128;
	  Kollision=Kollision|0x10;
	 }
	 LevPkt=LevelChar(x,y);
	 Punkt=Level_Attribut(LevPkt);
	 Kollision=Kollision|Punkt;

	 if ((Punkt&0x20)==0x20)         	/* TÅr			   */
	 {
	  Tuerkoll=LevPkt-48      ;                /* welche TÅr?             */
	 }



     }
    }
    c++;
   }
  }
 }
  return (Kollision);
}


/***************************************************************************/
/* LevelChar: liefert den LevelCode der den Koordinaten entspricht         */
/*-------------------------------------------------------------------------*/
/* Eingabe : X,Y = Koordinaten auf dem BS                                  */
/* Ausgabe : LevelCode                                                     */
/***************************************************************************/
BYTE LevelChar (INT X, INT Y)
{
 BYTE LevelCode=0;                      /* RÅckgabewert initialisieren     */
 WORD XT;                               /* Zwischenvariable*/

 XT=Raum*31*19;                         /* Raumanfang  */
 X=X/8;
 Y=Y/8;
 LevelCode=LevelDaten[XT+(Y*31+X)];

 return(LevelCode);
}


/***************************************************************************/
/* SpriteUnt : liefert den Untergrundcode des Sprites                      */
/*-------------------------------------------------------------------------*/
/* Eingabe : SpriteNr = Nummer des Sprite, das gezeichnet werden soll      */
/* Ausgabe : Kollisionscode                                                */
/***************************************************************************/
int SpriteUnt (int SpriteNr)
{
/*-- Benîtigte Variablen						   */
 int  c;
 int  x;
 int  y;
 int  z=SpriteNr;
 INT  DUMMY; //Zwischenspeicher
 unsigned char Punkt;
 unsigned char* Sprites;
 Sprites=SpZeiger[SpriteNr];
 Untergrund=0;					/* Untergrund Reset        */

/*-- Sprite abfragen                     				   */
 if (SpActiv[z]==1)				/* Sprite aktiv ?          */
 {
/*-- Bildschirm abfragen						   */
  c=0;
  y=SpY[z]+SpYL;
  for (x=SpX[z]; x<SpX[z]+SpXL; x++)
  {
    if (KoordTest(x,y)==TRUE)
    {
     if(Sprites[((SpXL*(SpYL-1))+c)]<0xFF)
     {
      Punkt=getpix(x,y);
      if ((Punkt>127)&&(Punkt<148))		/* Sprite                  */
      {
       Sprkoll=Punkt-128;                       // SpriteNr holen
       Untergrund=Untergrund|0x10;              // KollisionsbestÑtigung
      }
      Punkt=LevelChar(x,y);
      Punkt=Level_Attribut(Punkt);
      Untergrund=Untergrund|Punkt;              /* VerkÅpfen               */
      if ((Punkt&0x08)==0x08)		        /* bin ich auf der Mauer?  */
      {
       DUMMY=(y/8)*8;		// ermitteln Position Levelcode und damit
       if (y!=DUMMY)            // Restpixel entfernen
       {
	   Untergrund=Untergrund&0xF7;   // nicht ganz on the Top
       }
     }
     }
    }
    c++;
  }
 }
 return (Untergrund);
}



