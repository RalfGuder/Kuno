/***************************************************************************/
/* L E V E L . C P P   => Spiel.CPP                                        */
/*-------------------------------------------------------------------------*/
/* Aufgabe       :  Datencode fÅr alle Level fÅr                           */
/*                  K U N O   -   D E R   R I T T E R			   */
/*-------------------------------------------------------------------------*/
/* Autor         :  Ralf Guder						   */
/* Arbeitsbeginn :  30.01.96						   */
/* Arbeitsende   :                                                         */
/* letztes Update:  22.03.99                                               */
/*-------------------------------------------------------------------------*/
/* Info:                                               			   */
/* -----     								   */
/* Grî·e eines Levels 31x19						   */
/***************************************************************************/

/***************************************************************************/
/* Tabelle zur Umrechnung der Levelzeichen in BS-Zeichen                   */
/***************************************************************************/

unsigned char Tabchar[]={32,48,49,50,51,52,53,54,55,56,57,65,66,67,
			 68,69,70,71,72,73,74,75,76,77,78,79,80,81,
			 82,83,84,85,86,87,88,89,90};

unsigned char Tabattr[]={0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
			 0x20,0x20,0x48,0x08,0x80,
			 0x80,0x00,0x00,71,72,73,74,75,0x08,0x00,0x00,0x00,
			 0x00,0x00,
			 82,83,84,85,86,87,88,89,90};

/***************************************************************************/
/* Liste der beweglichen Objekte Level0					   */
/***************************************************************************/
unsigned int Objekt0[]=
{
/*-- Level0, Objekte Raum0                                                 */
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,

/*-- Level0, Objekte Raum1						   */
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,

/*-- Level0, Objekte Raum2						   */
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
/*-- Level0, Objekte Raum3						   */
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
/*-- Level0, Objekte Raum4						   */
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
/*-- Level0, Objekte Raum5						   */
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
/*-- Level0, Objekte Raum6						   */
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
/*-- Level0, Objekte Raum7						   */
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
/*-- Level0, Objekte Raum8						   */
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
/*-- Level0, Objekte Raum9						   */
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,
 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0
};


/***************************************************************************/
/* Level-Nummer 0  (Grund- und Demolevel)                                  */
/***************************************************************************/
unsigned char Level0[]=
{
/*-- Raum 0 ---------------------------------------------------------------*/
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
"A                             A"
"1                             2"
"1                             2"
"1                             2"
"ABBBBB   BBBBADCABBBB   BBBBBBA"
"A            ADCA             A"
"A            ADCA             A"
"A            ADCA             A"
"A            ADCA             A"
"A            ADCA             3"
"A            ADCA             3"
"A            ADCA             3"
"A  AAAAAAAAAAADCAAAA   AAAAAAAA"
"1             DC              A"
"1             DC              A"
"1             DC              A"
"AAAA         AAAA         AA  A"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"

/*-- Level0 Raum 1							   */
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
"A                             A"
"A                             0"
"A                             0"
"A                             0"
"A     AAAAAAAAAAAAA        BBBA"
"A                             A"
"A                             A"
"A                       BBB   A"
"A                             A"
"A                             A"
"A                          BBBA"
"A                             A"
"A                             A"
"A                       BBB   0"
"A                             0"
"A                             0"
"AAAAAA                     AAAA"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"

/*-- Level0 Raum 2                                                         */
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
"A                             A"
"0                             A"
"0                             A"
"0                             A"
"AAAAAAAAAAAAAAAAAAABBBB       A"
"A                      BBB    A"
"A                             A"
"A                  BBB        A"
"A                             A"
"A              BBB            A"
"A             B   B           A"
"A            B     B          A"
"A                  B          9"
"A                 B           9"
"A               BB            9"
"A             BB              A"
"A            B                A"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"

/*-- Raum 3								   */
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"

/*-- Raum 4								   */
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"

/*-- Raum 5								   */
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"

/*-- Raum 6								   */
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"

/*-- Raum 7								   */
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"

/*-- Raum 8								   */
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"

/*-- Raum 9								   */
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"A                             A"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"

};


/***************************************************************************/
/* Tabelle zur Umrechnung der Levelnummern                                 */
/***************************************************************************/
unsigned char *LevelNr[]={ Level0 };

unsigned int *ObjektNr[]={ Objekt0 };