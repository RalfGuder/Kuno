/***************************************************************************/
/*                  K U N O   -   D E R   R I T T E R                      */
/*-------------------------------------------------------------------------*/
/*                Jump'n Run - Spiel                                       */
/*-------------------------------------------------------------------------*/
/* Autor         : Ralf Guder                                              */
/* Arbeitsbeginn : 31.01.96                                                */
/* Arbeitsende   :                                                         */
/* letztes Update: 18.04.99                                                */
/* PC-System     : 486SX-25                                                */
/*                 VGA (320x200) 256                                       */
/*-------------------------------------------------------------------------*/
/* Speichermodell: HUGE                                                    */
/*-------------------------------------------------------------------------*/
/* compiliert mit Borland Turbo C++                                        */
/* Projekt       : KUNO    .PRJ						   */
/* Programmmodule: MAINFILE.CPP                                            */
/*                 FILEWORK.CPP - enth�lt Routinen zur Dateiarbeit         */
/*                 VGAWORK .CPP - enth�lt Routinen zur Bildschirmarbeit    */
/*                 SPIEL   .CPP - enth�lt Routinen f�r das Spiel           */
/*		   SPRITE  .CPP - enth�lt Routinen bewegliche Objekte      */
/*-------------------------------------------------------------------------*/
/* Funktionen    : ERROR ()	- Fehlerroutine                            */
/*                 GRAFIKTEST() - Test auf VGA-Karte                       */
/*                 MAIN()       - Hauptprogramm                            */
/***************************************************************************/

/*== 1.   Voreinstellungen ================================================*/

/*-- 1.1  INCLUDE-Dateien -------------------------------------------------*/
#define kuno_c

#include <graphics.h>
#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <process.h>
#include <iostream.h>
#include <alloc.h>
#include "types.h"
#include "SOUND.CPP"		      /* enth�lt die Sounddaten            */
#include "KUNO.H"




/*-- Definitionen f�r den Tastatur-Handler --------------------------------*/
#ifdef __cplusplus
    #define __CPPARGS ...
#else
    #define __CPPARGS
#endif

/*-- 1.2  GLOBAL-Variablen ------------------------------------------------*/


PCHAR RG1;				  /* Zeiger auf IntroBild          */
CBYTE *message;  		          /* Zeiger f�r Fehlermeldungen    */
char VGAalt        ;			  /* Variable f�r alten Videomodus */
VOID  interrupt(*oldhandler)(__CPPARGS);/* enth�lt alten Tastatur-Vektor   */
VOID  interrupt(*oldtimehdl)(__CPPARGS);/* enth�lt alten Timer-Vektor      */


/*-- 1.3  externe Routinen, die ben�tigt werden ---------------------------*/
void set_rate (int);

/*        Modul "VGAWORK.CPP"                                              */
void init320200( void )   ;	          /* Initialisierung VGA-Modus     */
void setpix( int, int, unsigned char );   /* setzt einen Pixel             */
unsigned char getpix( int, int );         /* liefert die Farbe eines Pixels*/
void setpage( unsigned char );		  /* setzt Zugriffsseite           */
void showpage( unsigned char );           /* zeigt Seite auf Bildschirm    */

/*        Modul "FILEWORK.CPP"					           */
void TGA_laden(char *);                   /* l�dt ein TGA-Bild in den VGA  */
void Bild_aufblenden(void);
void Bild_ausblenden(void);

/*        Modul "SPIEL.CPP"                                                */
BYTE GetTaste(VOID);                      /* liefert den Tastaturwert      */
void LoadSpielSprites(void);		  /* l�dt die Sprites in das Spiel */
void Spiel(unsigned char);                /* Startet das eigentliche Spiel */
void Auswahl(VOID);			  /* Men�auswahl - Anfang          */
void interrupt timehdl(__CPPARGS);
void interrupt handler(__CPPARGS);
BYTE SpeicherRese(VOID);
BYTE SpeicherFrei(VOID);



/*   Modul "ADLIB.CPP"							   */
void Adlibvoice(unsigned int,   	/* initialisiert eine Stimme       */
		unsigned char*);
void Adlibreset(void);			/* zur�cksetzen der Karte          */
void StimmeDaten(unsigned char,
		 unsigned int *);	/* �bergibt Stimmdaten  	   */
void StimmeActiv(unsigned char);
void LiedDaten(unsigned char,
		 unsigned int *);	/* �bergibt Lieddaten  	   	   */
void LiedActiv(unsigned char);
void StimmeDeactiv(unsigned char);
void Stimme (unsigned char);

int Adlibtest(void);			  /* testet, ob eine Adlibkarte da */


/*== 2.   SUB-Routinen ====================================================*/

/***************************************************************************/
/* ERROR   : Fehler-Routine mit Programm-Abruch                            */
/*-------------------------------------------------------------------------*/
/* Eingabe :	MESSAGE	= enth�lt einen String, der als Fehlermeldung      */
/*			  ausgegeben wird 				   */
/***************************************************************************/

void huge error( const char *message )
{
 cout << ("\n\nError: %s \n\n", message);	      /* Meldung ausgeben  */
 printf  ("\nTaste dr�cken!");			           /* (<STDIO.H>)  */
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
 int x;    			/* R�cksprungvariable, enth�lt Fehlercode  */

 x=1;				     /* Annahme, da� kein VGA installiert  */
 asm mov ax,0x1A00;		       /* Funktion 1Ah, Unterfunktion 00h  */
 asm int 0x10; 					     /* VGA-BIOS aufrufen  */
 if (_AL==0x1A)   				 /* wenn VGA installiert,  */
 {
  x=0; 						  /* dann kein Fehlercode  */
 }
 return (x);
}



/***************************************************************************/
/* MAIN    :  Hauptprogramm von  K U N O				   */
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

/* Meldungen ausgeben							   */
 clrscr();
 textcolor(7);
 textbackground(1);
 cprintf("                    "
	 "K U N O - Der Ritter"
	 "                    ");
 printf("\nGudchensoft (c) 1996; 1999\n\n");

/* Grafikkarte testen (VGA ist Voraussetzung)      			   */
 printf("Initialisiere Grafikkarte: ");
 if (Grafiktest()==1)
 {
  error("keine VGA-Karte");
 }
 printf("erfolgreich!\n");

/* Soundkarte testen							   */
 printf("Initialisiere Sound      : ");
 if (Adlibtest()==0)
 {
  printf("keine Soundkarte!\n");
 }
 else
 {
  printf("erfolgreich!\n");
 }
/* Soundkarte initialisieren						   */
 Adlibreset();
/* Sprites laden							   */
 printf("Initialisiere Sprites    : ");

// RG1=(PCHAR) farmalloc(3*256+64005);
// if (RG1==NULL)
// {
//  printf("nicht genug Speicher!\n");
// }

 if (SpeicherRese()==0)
 {
  printf("nicht genug Speicher!\n");
 }
 LoadSpielSprites();
 printf("erfolgreich!\n");




/*-- Globale Variablen setzen ---------------------------------------------*/
 Seite_hinten=2;
 Seite_Reserve=3;
 GAMETIME=0;
 LevelDaten=Level1;


/*-- PC-System f�r Spiel einstellen ---------------------------------------*/
/*-- Tastatur-Vektor retten                                                */
   oldhandler = getvect(0x09);

/*-- neuen Tastatur-Vektor setzen                                          */
   setvect(0x09, handler);

/*-- Timer-Vektor retten                                                   */
   set_rate(8);
   oldtimehdl = getvect(0x1C);

/*-- neuen Timer   -Vektor setzen                                          */
   setvect(0x1C, timehdl);

/*-- Musikstimmen setzen					  	   */
 Adlibreset();
 LiedDaten(0,(unsigned int *)Stimme0);
 LiedDaten(1,(unsigned int *)Stimme1);
 LiedDaten(2,(unsigned int *)Stimme2);
 LiedDaten(3,(unsigned int *)Stimme3);
 LiedDaten(4,(unsigned int *)Stimme4);
 LiedDaten(5,(unsigned int *)Stimme5);
 LiedDaten(6,(unsigned int *)Stimme6);
 LiedDaten(7,(unsigned int *)Stimme7);
 LiedActiv(0);
 LiedActiv(1);
 LiedActiv(2);
 LiedActiv(3);
 LiedActiv(4);
 LiedActiv(5);
 LiedActiv(6);
 LiedActiv(7);
/* Grafikmodus einschalten                                                 */
 init320200();				      /* Initialisierung VGA-Modus */
 setpage(0);                                  /* Seite #0 als aktive Seite */
 showpage(1);

/* Optisches Zeichen f�r den Lademodus					   */
 TGA_laden("disk.tga");
 showpage(0);
 Bild_aufblenden();
 setpage(1);
 TGA_laden("rg.tga");
 setpage(2);
/* TGA_laden("rg3.tga");*/
 setpage(0);
 Bild_ausblenden();
 setpage(1);
 showpage(1);
 Bild_aufblenden();
 delay(1000);
 Bild_ausblenden();
/* setpage(2);*/
/* showpage(2);*/
/* Bild_aufblenden();*/
 Auswahl();
 Bild_ausblenden();

/* Spiel(0);					/* neues Spiel starten     */
/*-- Alte Einstellungen wieder herstellen ---------------------------------*/

/*-- alten Tastaturvektor zur�cksetzen                                     */
 setvect(0x09, oldhandler);

/*-- alten Timervektor zur�cksetzen                                        */
 setvect(0x1C, oldtimehdl);
 set_rate(0);
/* Soundkarte zur�cksetzen						   */
 Adlibreset();
 SpeicherFrei();
 farfree(RG1);
/* alten Videomodus herstellen 						   */
 closegraph();
 _AL=VGAalt;
 _AH=0;
 geninterrupt(0x10);

return;
}

