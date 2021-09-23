#include <dos.h>

/* Typendeklaration */
typedef unsigned char BYTE;
typedef unsigned int  WORD;


/* Modulvariablen */

BYTE oszoffs[]={0x00,0x01,0x02,0x03,0x04,
		0x05,0x08,0x09,0x0a,0x0b,
		0x0c,0x0d,0x10,0x11,0x12,
		0x13,0x14,0x15};

WORD voioffs[]={0x0300,0x0401,0x0502,
		0x0906,0x0a07,0x0b08,
		0x0f0c,0x100d,0x110e};

BYTE sound[]={	0x10,0x00,0xf2,0x84,0x00,
		0x11,0x0f,0xf2,0x84,0x00,
		0x00};

BYTE melodie[]={0x0f60,3,0x1240,3,0x0f60,3,0x1287,3,
		0x0f60,3,0x1201,3,0x1240,3,
		0x0f60,3,0x12ad,3,0x0f60,3,0x1301,3,
		0x0f60,3,0x1287,3,0x12ad,3,
		0x0f60,3,0x1240,3,0x0f60,3,0x1287,3,
		0x0f60,3,0x1201,3,0x1240,3,
		0x0f60,3,0x12ad,3,0x0f60,3,0x1301,3,
		0x0f60,3,0x1287,3,0x12ad,3,
		0x0f60,3,0x1287,3,
		0x8000};

WORD time;



;
;-----------------------------------------------------------
; Programmiert einen Oszillator
;-----------------------------------------------------------
;

oszprog:
                push    ax
                push    bx

                mov     ah, 20h
                add     ah, oszoffs[bx]
oszprog01:
                lodsb
                call    adlprog

                add     ah, 20h
                cmp     ah, 0a0h
                jc      oszprog01

                add     ah, 40h
                lodsb
                call    adlprog

                pop     bx
                pop     ax
                ret

;
;-----------------------------------------------------------
; Programmiert die Verbindung zwischen zwei Oszillatoren
;-----------------------------------------------------------
;

conprog:
                push    ax

                add     al, 0c0h
                mov     ah, al
                lodsb
                call    adlprog

                pop     ax
                ret

/*;
;-----------------------------------------------------------
; Programmiert ein Adlib-Register
;-----------------------------------------------------------
;*/

void adlprog(void)
{
asm		xchg    al, ah;
asm		mov     dx, 388h
                out     dx, al

		mov     cx, 6
adlprog01:
                in      al, dx
                loop    adlprog01

                inc     dx
                mov     al, ah
		out     dx, al

                dec     dx
                mov     cx, 35
adlprog02:
                in      al, dx
		loop    adlprog02

                pop     dx
		pop     cx
                pop     ax
                ret



/*;
;-----------------------------------------------------------
; Programmiert einen Operator (Oszillator-Paar)
;-----------------------------------------------------------
;*/

void voiprog(void)
{
asm		push    ax;
asm		push    bx;

asm		mov     al, bl;
asm		shl     bx, 1;
asm		mov     bx, voioffs[bx];
asm		mov     ah, bh;

asm		mov     bh, 0;
		oszprog();

asm		mov     bl, ah;
		oszprog();

		conprog();

asm		pop     bx;
asm		pop     ax;
return;
}



void main (void)
{

asm		mov     ax, 0x0120;
		adlprog();


asm		mov     ax, 0x0800;
asm		adlprog();


asm		mov     ax, 0xbd00;
		adlprog();


asm		mov     si, offset sound;
asm		mov     bx, 0;
		voiprog();


adlib03:
;
asm		mov     ah, 0;
asm		int     0x1a;
asm		mov     time, dx;
adlib04:
;
asm		mov     ah, 0;
asm		int     0x1a;
asm		cmp     dx, time;
asm		je      adlib04;


adlib05:
;
asm		mov     si, offset melodie;


adlib06:
;
asm		mov     ah, 1;
asm		int     0x16;
asm		jz      adlib07;
asm		jmp     adlib11;


adlib07:
;
asm		mov     ax, 0xb000;
		adlprog();


asm		lodsw;
asm		mov     bx, ax;


asm		cmp     bx, 0x8000;
asm		jne     adlib08;
asm		jmp     short adlib05;


adlib08:
;
asm		or      bx, 0x2000;

asm		mov     ah, 0xa0;
asm		mov     al, bl;
		adlprog();

asm		add     ah, 0x10;
asm		mov     al, bh;
		adlprog();


asm		mov     ah, 0;
asm		int     0x1a;
asm		lodsw;
asm		add     dx, ax;
asm		mov     time, dx;
adlib10:
;
asm		mov     ah, 0;
asm		int     0x1a;
asm		cmp     dx, time;
asm		jne     adlib10;

asm		jmp     adlib06;

;
; Tastendruck
; aus Buffer holen
;

adlib11:
asm		mov     ah, 0;
asm		int     0x16;

;
; Routine
; verlassen
;

return;
}