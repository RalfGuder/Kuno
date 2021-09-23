/***NOTE:
    This is an interrupt service routine.  You can NOT compile this
    program with Test Stack Overflow turned on and get an executable
    file which will operate correctly. */

#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <iostream.h>

#define INTR 0X09    /* The clock tick interrupt */

#ifdef __cplusplus
    #define __CPPARGS ...
#else
    #define __CPPARGS
#endif

void interrupt ( *oldhandler)(__CPPARGS);

unsigned char count=0;                       // KunosteuerungsZ臧ler
unsigned char scan;
unsigned char scan1;

void interrupt handler(__CPPARGS)
{
   scan =inportb(0x60);
   if (scan<128) goto Make;				//Taste gedr…kt
   scan=scan-128;
   if (scan==1 ) count=count^0x20;			// ESC los
   if (scan==57) count=count^0x80; //goto SEnde;		// SPACE los
   if (scan==29) count=count^0x40;			// CTRL los
   if (scan==28) count=count^0x10;			// ENTER los
   if (scan==75) count=count^0x01;			// Richtungen los
   if (scan==72) count=count^0x08;			// Richtungen los
   if (scan==77) count=count^0x02;			// Richtungen los
   if (scan==80) count=count^0x04;			// Richtungen los
   goto SEnde;

Make:
   if (scan==1 ) count=count|0x20;			// ESC gedr…kt
   if (scan==57) count=count|0x80;			// SPACE gedr…kt
   if (scan==29) count=count|0x40;			// CTRL gedr…kt
   if (scan==28) count=count|0x10;			// Enter gedr…kt
   if (scan==75) count=count|0x01;			// Links gedr…kt
   if (scan==72) count=count|0x08;			// Hoch gedr…kt
   if (scan==77) count=count|0x02;			// Rechts gedr…kt
   if (scan==80) count=count|0x04;			// Tief gedr…kt

SEnde:
   outportb (0x20,0x20);
}

int main(void)
{
/* save the old interrupt vector */
   oldhandler = getvect(INTR);

/* install the new interrupt handler */
   setvect(INTR, handler);

   int Hex;
/* loop until the counter exceeds 20 */
   while ((count&0x20) != 0x20)
	 {
	 printf("count is %i  ",scan);
	 Hex=count&0x80;
	 asm SHR Hex,7;
	 printf(" = %X",Hex);
	 Hex=count&0x40;
	 asm SHR Hex,6;
	 printf("%X",Hex);
	 Hex=count&0x20;
	 asm SHR Hex,5;
	 printf("%X",Hex);
	 Hex=count&0x10;
	 asm SHR Hex,4;
	 printf("%X ",Hex);
	 Hex=count&0x08;
	 asm SHR Hex,3;
	 printf("%X",Hex);
	 Hex=count&0x04;
	 asm SHR Hex,2;
	 printf("%X",Hex);
	 Hex=count&0x02;
	 asm SHR Hex,1;
	 printf("%X",Hex);
	 Hex=count&0x01;
	 printf("%X \n",Hex);
	 }

/* reset the old interrupt handler */
   setvect(INTR, oldhandler);

   return 0;
}

