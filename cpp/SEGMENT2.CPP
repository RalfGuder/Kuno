/*-- INCLUDE-Dateien ------------------------------------------------------*/
#include <conio.h>
#include <dos.h>
#include <alloc.h>
#include "types.h"


PCHAR SEGMENT2;

PCHAR Segment2(VOID)
{
/*-- Sprite-Speicher reservieren -------------------------------------------*/
//  SEGMENT1=(PCHAR) farmalloc(24*21*26);

    SEGMENT2 = (PCHAR) farmalloc(65000);
    return(SEGMENT2);
}

