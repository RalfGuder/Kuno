/*-- INCLUDE-Dateien ------------------------------------------------------*/
#include <conio.h>
#include <dos.h>
#include <alloc.h>
#include "types.h"


PCHAR SEGMENT1;

PCHAR Segment1(VOID)
{
/*-- Sprite-Speicher reservieren -------------------------------------------*/
//  SEGMENT1=(PCHAR) farmalloc(24*21*26);

    SEGMENT1 = (PCHAR) farmalloc(64000);
    return(SEGMENT1);
}

