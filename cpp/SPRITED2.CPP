
/***************************************************************************/
/* Farbcode und Bewegungsphasen der HEXE 1                                */
/***************************************************************************/
/*-- Bewegung nach Links                                                   */
unsigned char SHEXELI1[24*21];
unsigned char SHEXELI2[24*21];
/*-- Bewegung nach Rechts                                                  */
unsigned char HEXERE1[24*21];
unsigned char HEXERE2[24*21];

unsigned char *Spritewo(int Nummer)
{
 unsigned char *Beginn=SHEXELI1+(24*21*(Nummer-1));
 return (Beginn);
}
