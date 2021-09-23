/**********************************************************************/
/*                     F M . C                                        */
/*--------------------------------------------------------------------*/
/* Aufagbe: Demonstriert den Zugriff auf die FM-Synthese der          */
/*          SoundBlaster-Karte.                                       */
/*--------------------------------------------------------------------*/
/* Autor          : Michael Tischer / Bruno Jennrich                  */
/* entwickelt am  : 06.02.1994                                        */
/* letztes Update : 31.03.1994                                        */
/* Achtung: MASTER un MIDI LautstÑrken entsprechend setzen            */
/* COMPILER: Borland C++ 3.1, Microsoft Visual C++ 1.5                */
/**********************************************************************/

/*--- Include-Dateien einbinden --------------------------------------*/

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "kbd.h"
#include "win.h"
#include "sbutil.h"
#include "fmutil.h"
#include "dsputil.h"
#include "args.h"

/*- Folgende Zeilen innerhalb eines Projektes entfernen: -------------*/
#include "sbutil.c"
#include "fmutil.c"

#define DSP_VERSIONONLY             /* Nur Versionskontrolle Benutzen */
#include "dsputil.c"
#include "win.c"
#include "args.c"

/*--- Makros und Konstanten ------------------------------------------*/
typedef struct tagTASTE
{
  CHAR cKey;                                    /* Taste auf Keyboard */
  INT  iNote;                                    /* zu spielende Note */
  BYTE bOct;
} TASTE;

SBBASE SBB;

TASTE QWERTZ[]= { { 'q', _c, 0 },
                  { 'w', _d, 0 },
                  { 'e', _e, 0 },
                  { 'r', _f, 0 },
                  { 't', _g, 0 },
                  { 'z', _a, 0 },
                  { 'u', _h, 0 },

                  { 'a', _c, 1 },
                  { 's', _d, 1 },
                  { 'd', _e, 1 },
                  { 'f', _f, 1 },
                  { 'g', _g, 1 },
                  { 'h', _a, 1 },
                  { 'j', _h, 1 },
                };

PBYTE screenbuf;
WINDOW win, tast, screen;
INTDATA Ints[ 15 ];
BOOLDATA Bools[ 6 ];
OBJECT Objects[ 20 ];

INT OPLOBJ,                    /* "Konstanten" um auf Eingaben direkt */
    MODEOBJ,                   /* reagieren zu kînnen.                */
    MODEINT,
    MODEBOOL,
    MODEY,                                                                                 
    OPL3_4OPOBJ,
    CHANNELOBJ,
    OSZILATOROBJ,
    FRQOBJ,
    FRQINT,
    OCTAVEOBJ,
    OCTAVEINT,
    MAXOBJ;

typedef struct tagCHDATA
{
  INT iOctave, iFrequency, iFM, iFeedBack;
} CHDATA;

typedef struct tagOSZIDATA
{
  INT iAttack , iDecay, iSustain, iRelease;
  INT iShortADSR;
  INT iContADSR;
  INT iVibrato;
  INT iTremolo;
  INT iMute, iHiMute;
  INT iFRQ, iWave;
} OSZIDATA;

OSZIDATA OD;
OSZIDATA ALLOSZ[ 2 ][ 18 ];

CHDATA CH;
CHDATA ALLCH[ 2 ][ 9 ];

INT iOpl;                                      /* Aktueller Oszilator */
INT iChannel;                                      /* Aktueller Kanal */
INT iOszilator;
INT iOPL3;                                /* liegt ein OPL3-Chip vor? */
INT iOPL3_4OP;                                      /* 4 Operatoren ? */

/**********************************************************************/
/* Func : Eigene Benachritigungsfunktion                              */
/**------------------------------------------------------------------**/
/* Eingabe : iAct   - OBJECT auf das sich Nachricht bezieht           */
/*           iMsg   - Nachrichtentyp                                  */
/*           iParam - Integer-Parameter                               */
/*           lParam - Long-Parameter                                  */
/**********************************************************************/
VOID Func( INT iAct, INT iMsg, INT iParam, LONG lParam )
{
  if( iMsg == MSG_CHANGED )
  {
    fm_SetOszilator( iOpl, iOszilator,                   /* Modulator */
                     ( BYTE )OD.iAttack, ( BYTE )OD.iDecay,
                     ( BYTE )OD.iSustain, ( BYTE )OD.iRelease,
                     ( BYTE )OD.iShortADSR, ( BYTE )OD.iContADSR,
                     ( BYTE )OD.iTremolo, ( BYTE )OD.iVibrato,
                     ( BYTE )OD.iMute, ( BYTE )OD.iHiMute,
                     ( BYTE )OD.iFRQ, ( BYTE )OD.iWave );

    if( iAct == OPLOBJ )                               /* OPL wechsel */
    {
      ALLCH[ ( INT )lParam ][ iChannel] = CH;/* aktuelle Daten retten */
      ALLOSZ[ ( INT )lParam ][ iOszilator ] = OD;

      CH = ALLCH[ iOpl ][ iChannel ];             /* neue Daten laden */
      iOszilator = fm_GetModulator( iChannel );
      OD = ALLOSZ[ iOpl ][ fm_GetModulator( iOszilator ) ];

      win_OBJECTPrintArray( &win, Objects, MAXOBJ, iAct );
    }

    if( iAct == OPL3_4OPOBJ )                /* Anzahl der Operatoren */
    {
      if( iOPL3_4OP )
      {
        CH.iFM = 0;
        win_OBJECTInitINT( &Objects[MODEOBJ],
                           &Ints[MODEINT], 0, MODEY, 100, 1,
                           "ZellverknÅpfung     :", 0, 3, &CH.iFM );
        fm_QuadroChannel( FM_FIRSTOPL2, TRUE, TRUE, TRUE );
        fm_QuadroChannel( FM_SECNDOPL2, TRUE, TRUE, TRUE );
      }
      else
      {
        CH.iFM = TRUE;
        win_OBJECTInitBOOL( &Objects[MODEOBJ],
                            &Bools[MODEBOOL], 0, MODEY, 100, 1,
                            "FM-Synthese         : ",
                            DT_YESNO, &CH.iFM );
        fm_QuadroChannel( FM_FIRSTOPL2, FALSE, FALSE, FALSE );
        fm_QuadroChannel( FM_SECNDOPL2, FALSE, FALSE, FALSE );
      }

      win_OBJECTPrintArray( &win, Objects, MAXOBJ, iAct );
    }

    if( iAct == OSZILATOROBJ )                    /* Oszilatorwechsel */
    {
      ALLOSZ[ iOpl ][ ( INT )lParam ] = OD;
      OD = ALLOSZ[ iOpl ][ iOszilator ];
      CH = ALLCH[ iOpl ][ fm_GetChannel( iOszilator ) ];
      iChannel = fm_GetChannel( iOszilator );

      win_OBJECTPrintArray( &win, Objects, MAXOBJ, iAct );
    }

    if( iAct == CHANNELOBJ )                          /* Kanalwechsel */
    {
      ALLCH[ iOpl ][  ( INT )lParam ] = CH;

      CH = ALLCH[ iOpl ][ iChannel ];
      iOszilator = fm_GetModulator( iChannel );
      OD = ALLOSZ[ iOpl ][ fm_GetModulator( iOszilator ) ];

      win_OBJECTPrintArray( &win, Objects, MAXOBJ, iAct );
    }
    fm_SetOszilator( iOpl, iOszilator,
                     ( BYTE )OD.iAttack, ( BYTE )OD.iDecay,
                     ( BYTE )OD.iSustain, ( BYTE )OD.iRelease,
                     ( BYTE )OD.iShortADSR, ( BYTE )OD.iContADSR,
                     ( BYTE )OD.iTremolo, ( BYTE )OD.iVibrato,
                     ( BYTE )OD.iMute, ( BYTE )OD.iHiMute,
                     ( BYTE )OD.iFRQ, ( BYTE )OD.iWave );
                   /* aktuellen Zustand von Kanal/Oszilator speichern */
   ALLOSZ[ iOpl ][ iOszilator ] = OD;
   ALLCH[ iOpl ][ iChannel ] = CH;
  }

  if( iMsg == MSG_KEY )                                /* Tastendruck */
  {
    fm_SetChannel( iOpl, iChannel,             /* Kanalnummer */
                 ( BYTE )( ALLCH[ iOpl ][ iChannel].iOctave ),
                 ALLCH[ iOpl ][ iChannel].iFrequency, 
                 ( BYTE ) ALLCH[ iOpl ][ iChannel].iFM,
                 ( BYTE ) ALLCH[ iOpl ][iChannel].iFeedBack );
    switch( iParam )
    {
      case KBD_F1:
        fm_PlayChannel( iOpl, 6, FALSE );
        fm_PlayBassDrum( iOpl, FALSE ); 
        fm_PlayBassDrum( iOpl, TRUE );
      break;
      case KBD_F2:
        fm_PlayChannel( iOpl, 7, FALSE );
        fm_PlayHiHat( iOpl, FALSE ); 
        fm_PlayHiHat( iOpl, TRUE );
      break;
      case KBD_F3:
        fm_PlayChannel( iOpl, 8, FALSE );
        fm_PlayTomTom( iOpl, FALSE ); 
        fm_PlayTomTom( iOpl, TRUE );
      break;
      case KBD_F4:
        fm_PlayChannel( iOpl, 7, FALSE );
        fm_PlaySnareDrum( iOpl, FALSE ); 
        fm_PlaySnareDrum( iOpl, TRUE );
      break;
      case KBD_F5:
        fm_PlayChannel( iOpl, 8, FALSE );
        fm_PlayTopCymbal( iOpl, FALSE ); 
        fm_PlayTopCymbal( iOpl, TRUE );
      break;

      default:
      { int i;

        fm_PlayChannel( iOpl, iChannel, ( BYTE )FALSE );
        for( i = 0; i < sizeof( QWERTZ ) / sizeof( TASTE ); i++ )
          if( tolower( iParam ) == QWERTZ[ i ].cKey )
          { INT old;
            fm_SetChannel( iOpl, iChannel,             /* Kanalnummer */
                 ( BYTE )( QWERTZ[ i ].bOct +               /* Oktave */
                           ALLCH[ iOpl ][ iChannel].iOctave ),
                 QWERTZ[ i ].iNote,
                 ( BYTE ) ALLCH[ iOpl ][ iChannel].iFM,
                 ( BYTE ) ALLCH[ iOpl ][iChannel].iFeedBack );
            fm_PlayChannel( iOpl, iChannel, ( BYTE )TRUE );

                                   /* Display an neue Werte anpassen! */
            win_LoVideo( &win );
            if( iAct == OCTAVEOBJ ) win_HiVideo( &win );
            old = *Ints[ OCTAVEINT ].pValue;
            *Ints[ OCTAVEINT ].pValue = QWERTZ[ i ].bOct + CH.iOctave;
            win_OBJECTPrint( &win, &Objects[ OCTAVEOBJ ] );
            *Ints[ OCTAVEINT ].pValue = old;
            win_LoVideo( &win );

            if( iAct == FRQOBJ ) win_HiVideo( &win );
            *Ints[ FRQINT ].pValue = QWERTZ[ i ].iNote;
            win_OBJECTPrint( &win, &Objects[ FRQOBJ ] );
          }
      }
    }
  }
}

/*--------------------------------------------------------------------*/
/*-- H A U P T P R O G R A M M                                      --*/
/*--------------------------------------------------------------------*/

VOID main( VOID )
{ INT i, j, y, o, ints, bools;

  if( sb_GetEnviron( &SBB, getenv( "BLASTER" ) ) == NO_ERROR )
    fm_SetBase( &SBB, TRUE );
  else
  if( fm_GetAdLib( &SBB ) != NO_ERROR )
  {
    printf("BLASTER-Umgebungsvariable nicht vorhanden\n");
    printf("und keine AdLib Kompatible Soundkarte gefunden!\n");
    exit(0);
  }

  iOPL3 = fm_QuadroOn( TRUE );
  iOPL3_4OP = FALSE;

  for( j = 0; j < 2; j++ )
  {
    for( i = 0; i < 9; i++ )
    {
      ALLCH[j][i].iOctave = 0;
      ALLCH[j][i].iFrequency = 0;
      ALLCH[j][i].iFM = TRUE;
      ALLCH[j][i].iFeedBack = 0;
      fm_SetChannel( j, i, ( BYTE )ALLCH[j][i].iOctave,
                                   ALLCH[j][i].iFrequency,
                           ( BYTE )ALLCH[j][i].iFM,
                           ( BYTE )ALLCH[j][i].iFeedBack );
                                     /* KanÑle den AusgÑngen zuweisen */
      if( iOPL3 ) fm_ChannelLR( j, i, j, !j );
    }
    for( i = 0; i < 18; i++)
    {
      ALLOSZ[j][i].iAttack = 10;
      ALLOSZ[j][i].iDecay = 3;
      ALLOSZ[j][i].iSustain = 15;
      ALLOSZ[j][i].iRelease = 6;
      ALLOSZ[j][i].iShortADSR = TRUE;
      ALLOSZ[j][i].iContADSR = FALSE;
      ALLOSZ[j][i].iTremolo = TRUE;
      ALLOSZ[j][i].iVibrato = TRUE;
      ALLOSZ[j][i].iMute = 0;
      ALLOSZ[j][i].iHiMute = 0;
      ALLOSZ[j][i].iFRQ = 7;
      ALLOSZ[j][i].iWave = 1;

      fm_SetOszilator( j, i,
        ( BYTE )ALLOSZ[j][i].iAttack,    ( BYTE )ALLOSZ[j][i].iDecay,
        ( BYTE )ALLOSZ[j][i].iSustain,   ( BYTE )ALLOSZ[j][i].iRelease,
        ( BYTE )ALLOSZ[j][i].iShortADSR, ( BYTE )ALLOSZ[j][i].iContADSR,
        ( BYTE )ALLOSZ[j][i].iTremolo,   ( BYTE )ALLOSZ[j][i].iVibrato,
        ( BYTE )ALLOSZ[j][i].iMute,      ( BYTE )ALLOSZ[j][i].iHiMute,
        ( BYTE )ALLOSZ[j][i].iFRQ,       ( BYTE )ALLOSZ[j][i].iWave );
    }
  }

  iOpl = 0;
  iChannel = 0;
  iOszilator = 0;

  OD = ALLOSZ[ iOpl ][ 0 ];
  CH = ALLCH[ iOpl ][ 0 ];

  o = 0;
  y = 0;
  ints = 0;
  bools = 0;

  OPLOBJ = -1;
  if( iOPL3 )
  {
    OPLOBJ = o;
    win_OBJECTInitINT( &Objects[o++], &Ints[ints++], 0, y++, 100, 1,
                             "OPL-Nr              :", 0, 1, &iOpl );

    OPL3_4OPOBJ = o;
    win_OBJECTInitBOOL( &Objects[o++], &Bools[bools++], 0, y++, 100, 1,
                             "4-Operatoren        : ", DT_YESNO,
                              &iOPL3_4OP );
    y++; /* Leerzeile */
  }

  CHANNELOBJ = o;
  win_OBJECTInitINT( &Objects[o++], &Ints[ints++], 0, y++, 100, 1,
                        "Channel        (0-8):", 0, 8, &iChannel );
  OCTAVEOBJ = o;
  OCTAVEINT = ints;
  win_OBJECTInitINT( &Objects[o++], &Ints[ints++], 0, y++, 100, 1,
                      "Oktave         (0-8):", 0, 8, &CH.iOctave );
  FRQOBJ = o;
  FRQINT = ints;
  win_OBJECTInitINT( &Objects[o++], &Ints[ints++], 0, y++, 100, 1,
                "Frequenz    (0-1023):", 0, 1023, &CH.iFrequency );

  MODEOBJ = o;
  MODEY = y;
  MODEINT = ints;
  MODEBOOL = bools;

  if( iOPL3_4OP )
   win_OBJECTInitINT( &Objects[o++], &Ints[ints], 0, y++, 100, 1,
                            "ZellverknÅpfung     :", 0, 3, &CH.iFM );
  else
    win_OBJECTInitBOOL( &Objects[o++], &Bools[bools], 0, y++, 100, 1,
                          "FM-Synthese         : ", DT_YESNO, &CH.iFM );
  ints++;
  bools++;

  win_OBJECTInitINT( &Objects[o++], &Ints[ints++], 0, y++, 100, 1,
                    "Feedback      (0- 3):", 0, 3, &CH.iFeedBack );
  y++; /* Leerzeile */

  OSZILATOROBJ = o;
  win_OBJECTInitINT( &Objects[o++], &Ints[ints++], 0, y++, 100, 1,
                     "Oszilator     (0-17):", 0, 17, &iOszilator );
  win_OBJECTInitINT( &Objects[o++], &Ints[ints++], 0, y++, 100, 1,
                     "Attack        (0-15):", 0, 15, &OD.iAttack );
  win_OBJECTInitINT( &Objects[o++], &Ints[ints++], 0, y++, 100, 1,
                      "Decay         (0-15):", 0, 15, &OD.iDecay );
  win_OBJECTInitINT( &Objects[o++], &Ints[ints++], 0, y++, 100, 1,
                    "Sustain       (0-15):", 0, 15, &OD.iSustain );
  win_OBJECTInitINT( &Objects[o++], &Ints[ints++], 0, y++, 100, 1,
                    "Release       (0-15):", 0, 15, &OD.iRelease );
  win_OBJECTInitBOOL( &Objects[o++], &Bools[bools++], 0, y++, 100, 1,
                "Short-ADSR          :  ", DT_ONOFF, &OD.iShortADSR );
  win_OBJECTInitBOOL( &Objects[o++], &Bools[bools++], 0, y++, 100, 1,
                 "Cont.-ADSR          :  ", DT_ONOFF, &OD.iContADSR );
  win_OBJECTInitBOOL( &Objects[o++], &Bools[bools++], 0, y++, 100, 1,
                  "Vibrato             :  ", DT_ONOFF, &OD.iVibrato );
  win_OBJECTInitBOOL( &Objects[o++], &Bools[bools++], 0, y++, 100, 1,
                  "Tremolo             :  ", DT_ONOFF, &OD.iTremolo );
  win_OBJECTInitINT( &Objects[o++], &Ints[ints++], 0, y++, 100, 1,
                       "Mute          (0-63):", 0, 63, &OD.iMute );
  win_OBJECTInitINT( &Objects[o++], &Ints[ints++], 0, y++, 100, 1,
                      "Hi-Mute       (0- 3):", 0, 3, &OD.iHiMute );
  win_OBJECTInitINT( &Objects[o++], &Ints[ints++], 0, y++, 100, 1,
                        "* Frq         (0-15):", 0, 15, &OD.iFRQ );
  if( iOPL3 )
    win_OBJECTInitINT( &Objects[o++], &Ints[ints++], 0, y++, 100, 1,
                          "Wave          (0- 7):", 0, 7, &OD.iWave );
  else
    win_OBJECTInitINT( &Objects[o++], &Ints[ints++], 0, y++, 100, 1,
                          "Wave          (0- 3):", 0, 3, &OD.iWave );

  MAXOBJ = o;
  win_GetScreenSettings( &screen );
  screenbuf = win_Save( &screen );
  win_Clr( &screen );

  win_Init( &win, 1, 1, 30, 22, 0x1F, 0x71, WIN_ACTIVE );
  win_Clr( &win );

  win_Init( &tast, 32, 1, 44, 22, 0x1F, 0x17, WIN_ACTIVE );
  win_Clr( &tast );


  win_printf(&tast, "    › › € › › › €   › › € › › › €\n", -1 );
  win_printf(&tast, "    › › € › › › €   › › € › › › €\n", -1 );
  win_printf(&tast, "    € € € € € € €   € € € € € € €\n", -1 );
  win_printf(&tast, "    € € € € € € €   € € € € € € €\n", -1 );
  win_printf(&tast, "    q w e r t z u   a s d e f g h\n", -1 );
  win_printf(&tast, "      1. Oktave       2. Oktave\n", -1 );
  win_printf(&tast, "\n", -1 );
  win_printf(&tast, "F1 - BassDrum   F2 - HiHat  F3 - TomTom\n", -1 );
  win_printf(&tast, "F4 - SnareDrum  F4 - TopCymbal\n\n", -1 );
  win_printf(&tast, "Navigation: \n", -1);
  win_printf(&tast, "* <CURSOR OBEN> & <CURSOR UNTEN>\n", -1);
  win_printf(&tast, "  Auswahl der Einstellung\n\n", -1 );
  win_printf(&tast, "* <+> & <->\n", -1);
  win_printf(&tast, "  anderer Wert\n\n", -1 );
  win_printf(&tast, "<ESC> fÅr Ende\n\n" );
  win_printf(&tast, "Micro-Piano (c)\n Michael Tischer &\n", -1 );
  win_printf(&tast, " Bruno Jennrich", -1);
                                        /* Percussionmodus aktivieren */
  fm_PercussionMode( FM_FIRSTOPL2, TRUE );
  fm_PercussionMode( FM_SECNDOPL2, TRUE );

  win_OBJECTProcessArray( &win, Objects, MAXOBJ, Func );

  fm_Reset();                                    /* komplette Stille */
  win_Restore( screenbuf, TRUE );
}

