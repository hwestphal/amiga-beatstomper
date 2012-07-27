/*
Copyright (c) 1993-2012, Harald Westphal
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <hardware/custom.h>
#include <hardware/cia.h>
#include <libraries/dosextens.h>
#include <graphics/gfx.h>
#include <graphics/sprite.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include "midi.h"

#define ANZ_GAD 35
#define TB (rp->TxBaseline)
#define MX (screen->MouseX)
#define MY (screen->MouseY)
#define GADGETON(g) (g)->Flags&=~GADGDISABLED
#define GADGETOFF(g) (g)->Flags|=GADGDISABLED
#define GNUMBER(g) (((struct StringInfo *)(g)->SpecialInfo)->LongInt)
#define GSTRING(g) (((struct StringInfo *)(g)->SpecialInfo)->Buffer)
#define PERIOD(r) (3579545/r)
#define RATE(p) (3579545/p)
#define custom (*(struct Custom *)0xdff000)
#define ciab (*(struct CIA *)0xbfd000)
#define PATTNR 'BSPT'
#define DKNR 'BSDK'
#define SONGNR 'BSSG'
#define LEFTDOWN (!((*(char *)0xbfe001)&0x40))
#define MIDDOWN (!((*(char *)0xdff016)&0x01))
#define RIGHTDOWN (!((*(char *)0xdff016)&0x04))
#define BOTHDOWN (LEFTDOWN&&RIGHTDOWN)
#define KEYBOARD (*(char *)0xbfec01)
#define LEDON (*(char *)0xbfe001=252)
#define LEDOFF (*(char *)0xbfe001=254)
#define SONGSIZE 105
#define CIATIME 5320343/3
#define DMAWAIT 100
#define MODE_NORMAL 0
#define MODE_NOP 1
#define MODE_PLUS 2
#define MODE_MINUS 3
#define MODE_FORCE 4

void InputInt(void);
void MidiInput(void);
void PlaySound(short);
void Record(short);
void LoadSample(void);
void Read8SVX(BPTR,ULONG,SHORT,UBYTE *);
void ReadRAW(BPTR,ULONG,SHORT,UBYTE *);
void LoadDrumkit(short,char *);
void SaveDrumkit(void);
void LoadSong(void);
void SaveSong(void);
void LoadPat(void);
void SavePat(void);
void PlayPat(void);
void StopPat(void);
void PausePat(void);
void GadgetsAus(void);
void GadgetsAn(void);
void PlaySong(void);
void PlaySongMidi(void);
void PlayPatInt(void);
void PlaySongInt(void);
void PlaySongIntMidi(void);
void EraseSong(void);
void EraseSample(short);
void EraseSmps(void);
void SetPat(short,short);
void SetSmpName(char *,short);
void SetLength(short);
void SetSpeed(short);
void SetQuant(short);
void SetWB(void);
void SetEvent(short,short);
void ClearEvent(short,short);
void Cut(void);
void Copy(void);
void Paste(void);
void Mix(void);
void RefreshThem(void);
short Str2Note(char *);
void Note2Str(short,char *);
char *ucase(char *);
struct Gadget *GetGadgAdr(short);
void ErrorMsg(char *);
char *FileBox(char *,char *,char *);
ULONG GetFileLength(UBYTE *);
void OpenAll(void);
void CloseAll(void);

struct SampleData
	{
	long length;
	short per;
	short vol;
	short note;
	short vmin;
	short vmax;
	short ach;
	short mch;
	};

UWORD display[5304] =
	{
	/* Plane 0 */
	0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,0,
	0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,0,
	0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,0,
	0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,0,
	0,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,0,
	0,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,0,
	0,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,0,
	1,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,63488,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1023,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1023,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1023,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1023,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1023,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1023,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1023,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1023,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1023,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1023,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2048,
	1,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,63488,
	0,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,0,
	0,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,3084,0,
	0,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,3072,0,
	0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,0,
	0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,3072,0,0,
	0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,0,
	0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,3072,0,0,0,0,
	/* Plane 1 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	24576,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	36864,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	61440,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	36864,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	36864,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	57344,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	36864,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	57344,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	36864,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	57344,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	28672,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	32768,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	32768,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	32768,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	28672,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	57344,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	36864,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	36864,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	36864,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	57344,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	28672,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	32768,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	57344,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	32768,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	28672,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	28672,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	32768,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	57344,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	32768,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	32768,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	28672,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	32768,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	45056,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	36864,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	28672,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	36864,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	36864,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	61440,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	36864,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	36864,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	28672,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	8192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	8192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	8192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	28672,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	61440,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	4096,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	4096,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	4096,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	57344,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	};

struct Image displayim = { 0,0,533,78,2,&display[0],0xff,0x0,NULL };

UWORD box[3][8] =
	{{ 0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,61440,61440,61440,61440 },
	{ 61440,61440,61440,61440,61440,61440,61440,61440 }};

struct Image boxim[3] =
	{{ 0,0,4,4,2,&box[0][0],0xff,0x0,NULL },
	{ 0,0,4,4,2,&box[1][0],0xff,0x0,NULL },
	{ 0,0,4,4,2,&box[2][0],0xff,0x0,NULL }};

UWORD metro1[168] =
	{
	/* Plane 0 */
	0,0,256,0,0,768,2047,~0,58112,2047,~0,58112,0,6144,768,0,6144,768,0,6144,768,
	0,6144,768,0,6144,768,0,6144,768,0,6144,768,0,6144,768,0,6144,768,0,6144,768,
	0,6144,768,0,6144,768,0,6144,768,0,6144,768,0,6144,768,0,6144,768,0,6144,768,
	0,6144,768,3,65472,768,3,65472,768,3,65472,768,3,65472,768,0,0,768,32767,~0,65280,
	/* Plane 1 */
	~0,~0,65024,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,
	49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,
	49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,
	49152,65280,0,49152,65280,0,49152,0,0,49152,0,0,32768,0,0
	};

UWORD metro2[168] =
	{
	/* Plane 0 */
	0,0,256,0,0,768,2047,~0,58112,2047,~0,58112,0,6144,768,0,12288,768,0,12288,768,
	0,24576,768,0,49152,768,1,32768,768,3,0,768,3,0,768,6,0,768,12,0,768,24,0,768,
	48,0,768,48,0,768,96,0,768,192,0,768,384,0,768,768,0,768,768,0,768,16368,0,768,
	16352,0,768,16320,0,768,16256,0,768,0,0,768,32767,~0,65280,
	/* Plane 1 */
	~0,~0,65024,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,
	49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,
	49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,
	65408,0,0,65280,0,0,49152,0,0,49152,0,0,32768,0,0
	};

UWORD metro3[168] =
	{
	/* Plane 0 */
	0,0,256,0,0,768,2047,~0,58112,2047,~0,58112,0,6144,768,0,3072,768,0,3072,768,
	0,1536,768,0,768,768,0,384,768,0,192,768,0,192,768,0,96,768,0,48,768,0,24,768,
	0,12,768,0,12,768,0,6,768,0,3,768,0,1,33536,0,0,49920,0,0,49920,0,15,65280,
	0,7,65280,0,3,65280,0,1,65280,0,0,768,32767,~0,65280,
	/* Plane 1 */
	~0,~0,65024,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,
	49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,
	49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,
	49152,1,64512,49152,0,64512,49152,0,0,49152,0,0,32768,0,0
	};

struct Image metroim[4] =
	{{ 0,0,40,28,2,&metro2[0],0xff,0x0,NULL },
	{ 0,0,40,28,2,&metro1[0],0xff,0x0,NULL },
	{ 0,0,40,28,2,&metro3[0],0xff,0x0,NULL },
	{ 0,0,40,28,2,&metro1[0],0xff,0x0,NULL }};

UWORD metrosnd[] =
  { 0x07D0,0x3C28,0x06ED,0x393F,0x3412,0xD5C2,0xB2B3,0xD9E2,0xFB5E,0x5380,0x4F2F,0xF9E8,0xB98D,0x8181,0x9DF4,0x224B,0x5C67,0x8080,0x6818,0xB9B0,0x818E,0x8190,0xC5F8,0x3159,0x7180,0x8062,0x25EB,0xAA9F,0x8498,0x9EE1,0xE529,0x4565,0x7341,0x35DF,0xE5AF,
  0xE3D0,0xE7F6,0xF123,0x1C57,0x3D12,0xE7B6,0xBFC3,0xECF7,0x141F,0x343E,0x4641,0xF915,0xDDAC,0xA89F,0xD7E4,0x4423,0x5726,0x3C2E,0x151E,0xE1B6,0x9CB8,0xCE01,0x1432,0x3547,0x412B,0x15E7,0xCBC4,0xBACC,0xE610,0x1C46,0x223E,0x3836,0x1BEE,0xD9AE,0xCCD4,0x0D0E,
  0x3D37,0x254F,0x1F30,0xF5EC,0xB0BE,0xB0D9,0x000D,0x3440,0x3556,0x3525,0xF4CC,0xB9BC,0xC3DF,0xF60A,0x3342,0x3D51,0x2E22,0x00E1,0xC2CD,0xC9D6,0xF00F,0x1D25,0x2B38,0x3B14,0x09D0,0xCAC4,0xC9DF,0xE315,0x132F,0x4734,0x2C11,0x15C3,0xE6B7,0xBDD8,0xD00E,0x1539,
  0x1F38,0x161A,0xFEE0,0xDAB1,0xC0CE,0xEE17,0x201F,0x302B,0x1F35,0xFCF4,0xC1BE,0xC0C2,0xF301,0x1D24,0x1635,0x3728,0x0AF6,0xC8D5,0xAFED,0xC6F9,0x0D18,0x3130,0x2118,0xF904,0xDAD8,0xBDD2,0xD1FF,0x162D,0x2324,0x0D0F,0x03FE,0xE4DA,0xCAC8,0xE2FE,0x192F,0x282F,
  0x160E,0x0FF0,0xF7D9,0xDFDB,0xECEF,0x1D28,0x2C28,0x0510,0xF607,0xE6EB,0xCAEA,0xF204,0x1912,0x1E17,0x120B,0x0010,0xFF01,0xD6C3,0xCADF,0x1330,0x0A1F,0x0118,0x1213,0x06CD,0xE3C7,0xF105,0x1712,0x0CFC,0x1904,0x090F,0xE706,0xF006,0xFAE8,0x0D00,0x1408,0xE902,
  0x0004,0x1C07,0x15EE,0xEE00,0x1520,0xF8F7,0xE2FE,0x14EF,0xF6EA,0xF1F5,0x120A,0x19FD,0xFA00,0x0605,0xF5E9,0xD4F5,0xFA07,0x0B14,0x0B29,0x1219,0x02F4,0x05EF,0x02E3,0xE0F6,0xFC17,0x1E0E,0x0E02,0xF30B,0xF808,0xF2E9,0xF2F5,0x0907,0x1614,0xFB0D,0x0AFD,0xFFFC,
  0xEFF9,0xFCF5,0xFA18,0x0A16,0x10E6,0x04E7,0x0AF4,0xF6E1,0xE0FE,0x0E1F,0x1223,0x0D13,0x0FFF,0x09DF,0xECDA,0xFDFF,0x010F,0x1515,0x0C20,0x040A,0xF4F1,0xE1E4,0xE8F8,0x080D,0x1F11,0x1111,0x0315,0x03F6,0xE3E1,0xE606,0xF514,0xFB17,0x0510,0x1505,0x08E5,0xEFE4,
  0xF2F8,0xFC11,0x070F,0x221F,0x1106,0xF4ED,0xF5E6,0xFEEF,0x0200,0x0411,0x1316,0x2106,0x0DEA,0xFDE6,0xFCFC,0xF3FE,0xFB0A,0x1111,0x1801,0x0DFE,0xFBF2,0xF8F2,0x0105,0xFF09,0x0617,0x130C,0x0CED,0xFB02,0xF10E,0xF200,0xFC07,0x1113,0x1B10,0x0105,0xF103,0xF208,
  0xFB02,0x0409,0x1809,0x0BFD,0x04FC,0x00F6,0xEDFC,0x0B0E,0x060D,0x0100,0x0C00,0xF6F9,0xF9F6,0xF4FF,0xFB0B,0xFD08,0x070A,0x0BFE,0xFDFF,0xFEEF,0x0103,0xFD11,0x1105,0x13F0,0x11FA,0x03FC,0xF7FE,0x02FF,0x0CFF,0x0A05,0xFB05,0x02FE,0xFAF8,0x0100,0xF406,0x040A,
  0x14FD,0x03F5,0x00FA,0xF8EC,0xF1E8,0xFEFD,0x1509,0x160A,0x0401,0xF306,0xF4F3,0xF7ED,0x0008,0x0415,0x020D,0x080E,0xFEFB,0xFAEE,0xED06,0xFAFC,0xF8FB,0x0C0A,0x1506,0x05FD,0xF6EF,0xF8FB,0xF801,0x000A,0x090C,0x060A,0x04FF,0xF5F6,0xFDEE,0x01FE,0xFEFE,0xFF01,
  0x0611,0x0D00,0xFEF3,0xFBF7,0x0901,0x06F1,0x0600,0x1516,0x14FD,0x00F1,0x020B,0x0404,0xF700,0x0212,0x0B0D,0x0A04,0xFCFB,0x00F9,0xFCFB,0xFFFC,0x0606,0x100A,0x0A00,0x0309,0xFD06,0x00F7,0xFEF8,0x040A,0x090A,0x020B,0x0609,0x0802,0xF501,0xFD06,0xFDFE,0x0301,
  0x1C08,0x0901,0x0206,0x1103,0x0100,0xF1F9,0x05FF,0x01FA,0x0105,0x0C00,0x0901,0x0A01,0x01FE,0xF0F7,0xF805,0x0109,0xF702,0x0408,0x0DFD,0x09F1,0xF9F7,0x0904,0xFC03,0xF008,0x040C,0x07F3,0x02EF,0x0C06,0x00FD,0xF7F6,0x0710,0xF80D,0xEA03,0xFC03,0x0FFD,0x01F6,
  0xFDF5,0x0105,0x0700,0xFBFD,0xF407,0x04FE,0xFFF5,0xF9F5,0x07FD,0x0DF3,0xF8F5,0xEE03,0x020A,0xFFF4,0xF6FB,0x0502,0xF902,0xF703,0xF6FE,0x0B07,0x0E06,0xFD05,0xFB05,0xFBFE,0xF1FB,0xF412,0x0509,0x08FC,0x0701,0x0202,0xF1F9,0xF406,0x020A,0x0306,0xFC01,0xFC07,
  0x02F9,0xFFF2,0xF7FF,0xFA09,0x0808,0x0CFB,0x09FC,0x04F6,0xF6F8,0xF906,0xFF03,0xF807,0x0611,0x0FFC,0xFBF7,0xFB08,0x090D,0x0B07,0xF605,0xFD0D,0x0804,0xFFF9,0xFBFB,0x0B06,0x0A01,0xFEFE,0x0DFC,0x04F4,0x0101,0xFA05,0xFF0C,0x010E,0xFAFD,0x0000,0xFFF8,0xF9FA,
  0xFF12,0x0719,0x090C,0x0501,0x05F3,0xFBF2,0xFDFC,0xF104,0x0C13,0x1D0A,0x14F5,0x01FC,0xF1FF,0xE5EE,0xF4FF,0x0713,0x1708,0x08FF,0x08FF,0xF4F8,0xF300,0xF8F5,0x0503,0x100C,0x0606,0xFFF2,0xFBFD,0xFAFC,0xFA04,0x080B,0x070A,0x0407,0xFAFA,0xF9F3,0xF9F5,0xF810,
  0x0616,0x090A,0x03FE,0x07EC,0xF2EC,0xF106,0xF608,0x040E,0x1A10,0x0FFD,0xF6F2,0xEEFB,0xF6F8,0xF6F5,0xFA0A,0x0E0A,0x0A04,0x0402,0xF6F7,0xF5F8,0xFD00,0x10FD,0x0700,0x050E,0x04FD,0x03FC,0xFFFC,0x0205,0x010E,0x08FF,0x00F9,0x0100,0x05FC,0x0603,0xF909,0x060E,
  0xFFFD,0xF9F6,0x06F1,0x00FA,0xFB02,0x0204,0xFFFC,0x0404,0x00FB,0xFAFA,0xF9FD,0x0508,0x0202,0x0508,0x1304,0x03FF,0xF801,0xF604,0xF40A,0x070D,0x1203,0x0909,0x0401,0x08EC,0xF8EE,0xF6FE,0x060F,0x0A10,0x0A05,0x04FF,0xFBF0,0xECF1,0xF501,0xF603,0x020C,0x0F0A,
  0x06FE,0xFAF3,0xF1F3,0xF0F4,0x0407,0x0D07,0x140D,0x0E07,0xFAFC,0xE9F2,0xF8FB,0x0105,0x0E0F,0x0E0E,0x0B0E,0xF2F1,0xEBEA,0xF4F1,0x0207,0x0512,0x1713,0x16FA,0xFBF1,0xECEF,0xEEF3,0xF2FC,0x0902,0x0E09,0x130C,0x07F0,0xF1ED,0xEBF5,0xFE01,0x0004,0x060F,0x1206,
  0x02FA,0xEEF3,0xF2F6,0x01FE,0x0D0F,0x0A0A,0x0707,0x00F9,0xF5EB,0xF3F5,0xFB09,0x0808,0x080F,0x0A10,0xFCF4,0xE6EB,0xEEF7,0xFC03,0x070C,0x1109,0x0D08,0xFEFA,0xF0F5,0xF7FB,0x0205,0x0E07,0x1205,0x0804,0x0101,0x00FC,0xF800,0x0108,0x1302,0x0606,0x0205,0x0001,
  0xF700,0xFE00,0x0805,0x0B0A,0x0D02,0xF5FA,0xF8FB,0xF8FA,0xFE01,0x0806,0x0E0D,0x0A06,0xFAF6,0xF2F7,0xF902,0x020C,0x0A0F,0x1117,0x1209,0xFEFB,0xF1F3,0xF6FE,0x080A,0x0B0B,0x1607,0x100E,0x0801,0xEFF1,0xF1FD,0xFE00,0x06FF,0x0E12,0x1302,0xFEFB,0xF5F3,0xF2F6,
  0x0201,0x0304,0x0B12,0x100F,0x08FC,0xFAF7,0xFBFF,0x04FA,0xFE09,0x0D0C,0x0E0A,0x0703,0xFFF5,0xF6F3,0xFC07,0xFE05,0x0416,0x0E09,0x01FC,0xF4F1,0xF2F5,0xFDFE,0x0003,0x0B0B,0x0B05,0xF701,0xF3F3,0xECF0,0xF1FA,0x0205,0x1309,0x06FD,0x03FC,0xF6F3,0xEBF0,0xFE0E,
  0xFF0A,0x020D,0x0E0A,0x08FF,0xF7F6,0xF5F5,0xF4FE,0x040D,0x0D0C,0x0A0D,0x0101,0x05ED,0xEEEF,0xFD01,0x060D,0x0806,0x0601,0x0B0B,0xF5EB,0xE5EF,0xF5FE,0x0B04,0x0602,0x0B14,0x0EFF,0xF5F2,0xEBF8,0xFA05,0x0302,0x0906,0x0F09,0x0703,0x04F6,0xEFF8,0x0003,0x01FF,
  0xFF02,0x1111,0x0A01,0xFCFA,0xFA00,0xFB00,0xF6F8,0x0202,0x0402,0x03F3,0xFBF7,0xFFFD,0xFF03,0x0C06,0x05FC,0x0103,0xFAFE,0xEEF8,0xFC00,0x120D,0x1105,0x0D09,0x0505,0xFAF2,0xEAF0,0xF70D,0x0A09,0x080B,0x0F13,0x1408,0xF9EC,0xEEF3,0xFBF6,0xF406,0x0C13,0x0B09,
  0x0904,0xFFF5,0xF8F0,0xF1F6,0xFA01,0x0A0B,0x0E10,0x1007,0xF8F3,0xF1FE,0xFD02,0x00FE,0x0A12,0x100F,0x07FD,0xF9FA,0xFBFA,0xFE01,0x0205,0x0B05,0x090B,0x0702,0x00F8,0xF3FA,0xFD02,0x0501,0x0A04,0x0806,0x03FD,0xFAF8,0xFCFF,0x0304,0x040C,0xFF05,0x0C09,0xFEF9,
  0xFDFA,0xFF05,0xF6FE };

UWORD busyptr[] =
	{ 0,0,
	0x0400,0x07C0,0x0000,0x07C0,0x0100,0x0380,0x0000,0x07E0,
	0x07C0,0x1FF8,0x1FF0,0x3FEC,0x3FF8,0x7FDE,0x3FF8,0x7FBE,
	0x7FFC,0xFF7F,0x7EFC,0xFFFF,0x7FFC,0xFFFF,0x3FF8,0x7FFE,
	0x3FF8,0x7FFE,0x1FF0,0x3FFC,0x07C0,0x1FF8,0x0000,0x07E0,
	0,0 };

UWORD cursor[] =
	{ 0,0,
	0xF000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,
	0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,
	0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,
	0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,
	0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,
	0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,
	0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,
	0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,
	0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,
	0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,
	0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,
	0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0x9000,0x0000,0xF000,0x0000,
	0,0 };

struct IntuiText AboutText[] =
	{{ 1,0,JAM1,10,10,NULL,"BeatStomper III V1.0.1",AboutText+1 },
	{ 1,0,JAM1,10,25,NULL,"written in 1993 by Harald Westphal",AboutText+2 },
	{ 1,0,JAM1,10,40,NULL,"© 1993-2012, Harald Westphal",AboutText+3 },
	{ 1,0,JAM1,10,55,NULL,"http://github.com/hwestphal/amiga-beatstomper",NULL }};

struct IntuiText OKText =
	{ 1,0,JAM1,6,3,NULL,"Okay",NULL };

struct IntuiText BodyText =
	{ 6,1,JAM1,10,10,NULL,"Are you sure?",NULL };

struct IntuiText DKText =
	{ 6,1,JAM1,10,10,NULL,"Song uses different drumkit! Load it?",NULL };

struct IntuiText PosText =
	{ 2,1,JAM1,6,3,NULL,"Yes",NULL };

struct IntuiText NegText =
	{ 4,1,JAM1,6,3,NULL,"No",NULL };

UWORD record[78] =
	{
	/* Plane 0 */
	0,0,512,0,0,1536,1,65280,1536,15,65504,1536,31,65520,1536,63,65528,1536,
	63,65528,1536,63,65528,1536,31,65520,1536,15,65504,1536,1,65280,1536,
	0,0,1536,32767,~0,65024,
	/* Plane 1 */
	~0,~0,64512,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,
	49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,32768,0,0
	};

struct Image recordim = { 0,0,39,13,2,&record[0],0xff,0x0,NULL };

UWORD play[78] =
	{
	/* Plane 0 */
	0,0,512,0,0,1536,3840,0,1536,4095,0,1536,4095,65024,1536,4095,65534,1536,
	4095,~0,58880,4095,65534,1536,4095,65024,1536,4095,0,1536,3840,0,1536,
	0,0,1536,32767,~0,65024,
	/* Plane 1 */
	~0,~0,64512,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,
	49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,32768,0,0
	};

struct Image playim = { 0,0,39,13,2,&play[0],0xff,0x0,NULL };

UWORD stop[78] =
	{
	/* Plane 0 */
	0,0,512,0,0,1536,63,65528,1536,48,24,1536,48,24,1536,48,24,1536,48,24,1536,
	48,24,1536,48,24,1536,48,24,1536,63,65528,1536,0,0,1536,32767,~0,65024,
	/* Plane 1 */
	~0,~0,64512,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,
	49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,32768,0,0
	};

struct Image stopim = { 0,0,39,13,2,&stop[0],0xff,0x0,NULL };

UWORD pause[78] =
	{
	/* Plane 0 */
	0,0,512,0,0,1536,63,61432,1536,48,27672,1536,48,27672,1536,48,27672,1536,
	48,27672,1536,48,27672,1536,48,27672,1536,48,27672,1536,63,61432,1536,
	0,0,1536,32767,~0,65024,
	/* Plane 1 */
	~0,~0,64512,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,
	49152,0,0,49152,0,0,49152,0,0,49152,0,0,49152,0,0,32768,0,0
	};

struct Image pauseim = { 0,0,39,13,2,&pause[0],0xff,0x0,NULL };

UWORD pfdown[24] =
	{
	/* Plane 0 */
	2,6,902,902,902,902,4070,1990,902,262,6,32766,
	/* Plane 1 */
	65532,49152,49152,49152,49152,49152,49152,49152,49152,49152,49152,32768
	};

struct Image pfdownim = { 0,0,15,12,2,&pfdown[0],0xff,0x0,NULL };

UWORD pfup[24] =
	{
	/* Plane 0 */
	2,6,262,902,1990,4070,902,902,902,902,6,32766,
	/* Plane 1 */
	65532,49152,49152,49152,49152,49152,49152,49152,49152,49152,49152,32768
	};

struct Image pfupim = { 0,0,15,12,2,&pfup[0],0xff,0x0,NULL };

struct Gadget igad[] =
  {{ igad+1,130,116,39,13,GADGIMAGE|GADGHCOMP,RELVERIFY,BOOLGADGET,
  (APTR)&recordim,NULL,NULL,NULL,NULL,31,NULL },
  { igad+2,180,116,39,13,GADGIMAGE|GADGHCOMP,RELVERIFY,BOOLGADGET,
  (APTR)&playim,NULL,NULL,NULL,NULL,32,NULL },
  { igad+3,230,116,39,13,GADGIMAGE|GADGHCOMP|GADGDISABLED,RELVERIFY,
  BOOLGADGET,(APTR)&stopim,NULL,NULL,NULL,NULL,33,NULL },
  { igad+4,280,116,39,13,GADGIMAGE|GADGHCOMP|GADGDISABLED,RELVERIFY,
  BOOLGADGET,(APTR)&pauseim,NULL,NULL,NULL,NULL,34,NULL },
  { igad+5,130,212,15,12,GADGIMAGE|GADGHCOMP,GADGIMMEDIATE|RELVERIFY,
  BOOLGADGET,(APTR)&pfupim,NULL,NULL,NULL,NULL,35,NULL },
  { igad+6,150,212,15,12,GADGIMAGE|GADGHCOMP,GADGIMMEDIATE|RELVERIFY,
  BOOLGADGET,(APTR)&pfdownim,NULL,NULL,NULL,NULL,36,NULL },
  { igad+7,130,228,15,12,GADGIMAGE|GADGHCOMP,GADGIMMEDIATE|RELVERIFY,
  BOOLGADGET,(APTR)&pfupim,NULL,NULL,NULL,NULL,37,NULL },
  { igad+8,150,228,15,12,GADGIMAGE|GADGHCOMP,GADGIMMEDIATE|RELVERIFY,
  BOOLGADGET,(APTR)&pfdownim,NULL,NULL,NULL,NULL,38,NULL },
  { igad+9,130,244,15,12,GADGIMAGE|GADGHCOMP,GADGIMMEDIATE|RELVERIFY,
  BOOLGADGET,(APTR)&pfupim,NULL,NULL,NULL,NULL,39,NULL },
  { NULL,150,244,15,12,GADGIMAGE|GADGHCOMP,GADGIMMEDIATE|RELVERIFY,
  BOOLGADGET,(APTR)&pfdownim,NULL,NULL,NULL,NULL,40,NULL }};

char *glabels[] =
	{ "Sample A","Sample B","Sample C","Sample D","Sample E",
	"Sample F","Sample G","Sample H","Sample I","Sample J",NULL };

struct NewGadget ngad[] =
	{{ 557,21,60,13,"Cut",NULL,1,PLACETEXT_IN,NULL,NULL },
	{ 557,37,60,13,"Copy",NULL,2,PLACETEXT_IN,NULL,NULL },
	{ 557,53,60,13,"Paste",NULL,3,PLACETEXT_IN,NULL,NULL },
	{ 557,69,60,13,"Mix",NULL,4,PLACETEXT_IN,NULL,NULL },
	{ 0,100,110,13,"Load Song",NULL,5,PLACETEXT_IN,NULL,NULL },
	{ 0,116,110,13,"Save Song",NULL,6,PLACETEXT_IN,NULL,NULL },
	{ 0,132,110,13,"Load Pattern",NULL,7,PLACETEXT_IN,NULL,NULL },
	{ 0,148,110,13,"Save Pattern",NULL,8,PLACETEXT_IN,NULL,NULL },
	{ 0,227,110,13,"About",NULL,9,PLACETEXT_IN,NULL,NULL },
	{ 0,243,110,13,"Quit",NULL,10,PLACETEXT_IN,NULL,NULL },
	{ 0,164,110,13,"Load Drumkit",NULL,11,PLACETEXT_IN,NULL,NULL },
	{ 0,180,110,13,"Save Drumkit",NULL,12,PLACETEXT_IN,NULL,NULL },
	{ 130,132,110,13,"Play Song",NULL,13,PLACETEXT_IN,NULL,NULL },
	{ 130,148,110,13,"Erase Song",NULL,14,PLACETEXT_IN,NULL,NULL },
	{ 130,164,110,13,"Erase Sample",NULL,15,PLACETEXT_IN,NULL,NULL },
	{ 130,180,110,13,"Erase Samples",NULL,16,PLACETEXT_IN,NULL,NULL },
	{ 0,196,200,12,"Song",NULL,17,PLACETEXT_RIGHT,NULL,NULL },
	{ 260,132,110,13,"Load Sample",NULL,18,PLACETEXT_IN,NULL,NULL },
	{ 260,148,26,13,"Metronome",NULL,19,PLACETEXT_RIGHT,NULL,NULL },
	{ 260,164,26,13,"MIDI",NULL,20,PLACETEXT_RIGHT,NULL,NULL },
	{ 260,180,26,13,"Filter",NULL,21,PLACETEXT_RIGHT,NULL,NULL },
	{ 260,196,26,13,"Workbench",NULL,22,PLACETEXT_RIGHT,NULL,NULL },
	{ 450,100,17,9,NULL,NULL,23,NULL,NULL,NULL },
	{ 260,212,26,13,"1",NULL,24,PLACETEXT_RIGHT,NULL,NULL },
	{ 310,212,26,13,"2",NULL,25,PLACETEXT_RIGHT,NULL,NULL },
	{ 260,228,26,13,"3",NULL,26,PLACETEXT_RIGHT,NULL,NULL },
	{ 310,228,26,13,"4",NULL,27,PLACETEXT_RIGHT,NULL,NULL },
	{ 390,212,50,12,"Note",NULL,41,PLACETEXT_RIGHT,NULL,NULL },
	{ 390,228,50,12,"V max",NULL,42,PLACETEXT_RIGHT,NULL,NULL },
	{ 390,244,50,12,"V min",NULL,43,PLACETEXT_RIGHT,NULL,NULL },
	{ 500,212,40,12,"ACh",NULL,44,PLACETEXT_RIGHT,NULL,NULL },
	{ 500,228,40,12,"MCh",NULL,45,PLACETEXT_RIGHT,NULL,NULL },
	{ 500,244,26,13,"Accent",NULL,28,PLACETEXT_RIGHT,NULL,NULL },
	{ 590,216,20,40,"V",NULL,29,PLACETEXT_ABOVE,NULL,NULL },
	{ 620,216,20,40,"P",NULL,30,PLACETEXT_ABOVE,NULL,NULL }};

struct NewWindow NewWindow =
	{ 0,0,640,256,0,1,GADGETUP|GADGETDOWN|MOUSEBUTTONS|VANILLAKEY,SMART_REFRESH|
	BACKDROP|BORDERLESS|ACTIVATE|RMBTRAP,NULL,NULL,NULL,NULL,NULL,0,0,640,256,
	CUSTOMSCREEN };

struct TextAttr Font =
	{ "topaz.font",TOPAZ_EIGHTY,FS_NORMAL,FPF_ROMFONT };

struct NewScreen NewScreen =
	{ 0,0,640,256,2,0,1,HIRES,CUSTOMSCREEN,&Font,"BeatStomper III V1.0.1 © 1993-2012 by Harald Westphal",
	NULL,NULL };

UWORD Pens_3D=-1;

APTR GfxBase,IntuitionBase,GadToolsBase,AslBase,CIABBase;
struct Screen *screen=NULL;
struct Window *window=NULL;
struct RastPort *rp;
struct Gadget *GadgetList=NULL;
struct VisualInfo *VInfo;
struct FileRequester *freq;
struct Process *Process;
struct SimpleSprite sprite;
struct Interrupt InputInterrupt,Interrupt;
struct Task *MidiTask;
long SigNr,MSigNr;
short KeyNr=-1,SNr;
char fb_loadfile[100];
char pattern[26][10][64];
char cbuffer[10][64];
short length[26];
char *songbuf;
struct SampleData SmpData[21];
char SmpName[10][100];
APTR SmpAdr[21];
char DKName[100]="Unnamed";
short AktPat=-1,AktSmp=1,AktMetro;
short Accent,Speed=120,Quant=1,Filter,WBench=1,Midi,Rec,Metro=1,MCh[4];
char pat_path[64]="BS-Patterns",pat_file[32]="";
char dk_path[64]="BS-Drumkits",dk_file[32]="";
char song_path[64]="BS-Songs",song_file[32]="";
char smp_path[64]="BS-Samples",smp_file[32]="";
short alt_key,midi_v1,midi_v2,midi_v3,midi_i;
short sp_i1,i_i1,pos_i1,c_i1,n_i1[4];
short sp_i2,c_i2,cou_i2,pat_i2,ccou_i2,i_i2,n_i2[4],pos_i2;
UWORD DMAWords[4][2]={{0x0001,0x8201},{0x0002,0x8202},{0x0004,0x8204},{0x0008,0x8208}};

main()
	{
	struct IntuiMessage *msg;
	long class,signal;
	short code,id,z;
	OpenAll();
	SetPat(0,MODE_NOP);
	SetSpeed(MODE_NOP);
	SetQuant(MODE_NOP);
	while (1)
		{
		signal=Wait(1<<window->UserPort->mp_SigBit|1<<SigNr|1<<MSigNr);
		while (msg=GT_GetIMsg(window->UserPort))
		{
		class=msg->Class;
		code=msg->Code;
		id=((struct Gadget *)msg->IAddress)->GadgetID;
		GT_ReplyIMsg(msg);
		if (class==GADGETUP)
			{
			switch (id)
				{
				case 1:	Cut(); break;
				case 2:	Copy(); break;
				case 3:	Paste(); break;
				case 4:	Mix(); break;
				case 5:	LoadSong(); break;
				case 6:	SaveSong(); break;
				case 7:	LoadPat(); break;
				case 8:	SavePat(); break;
				case 9:	AutoRequest(window,AboutText,NULL,&OKText,NULL,NULL,150,120); break;
				case 10: if (AutoRequest(window,&BodyText,&PosText,&NegText,NULL,NULL,150,60))
								{
								CloseAll();
								exit(0);
								}
							break;
				case 11: LoadDrumkit(MODE_NORMAL,NULL); RefreshThem(); break;
				case 12: SaveDrumkit(); break;
				case 13: if (Midi) PlaySongMidi(); else PlaySong();
							break;
				case 14: if (AutoRequest(window,&BodyText,&PosText,&NegText,NULL,NULL,150,60))
								EraseSong();
							break;
				case 15: EraseSample(AktSmp); RefreshThem(); break;
				case 16: if (AutoRequest(window,&BodyText,&PosText,&NegText,NULL,NULL,150,60))
								EraseSmps();
							break;
				case 18: LoadSample(); RefreshThem(); break;
				case 31: Rec=1;
				case 32: PlayPat(); break;
				case 33: StopPat(); break;
				case 34: PausePat(); break;
				case 19: Metro=!Metro; break;
				case 20: Midi=!Midi; break;
				case 21: if (Filter) LEDOFF; else LEDON;
							Filter=!Filter; break;
				case 22: SetWB(); break;
				case 24:
				case 25:
				case 26:
				case 27: MCh[id-24]=!MCh[id-24]; break;
				case 28: Accent=!Accent; RefreshThem(); break;
				case 29: SmpData[AktSmp+10*Accent].vol=code;
							break;
				case 30: SmpData[AktSmp+10*Accent].per=428-code;
							break;
				case 41: SmpData[AktSmp+10*Accent].note=Str2Note(GSTRING(GetGadgAdr(41)));
							RefreshThem(); break;
							break;
				case 42: z=SmpData[AktSmp+10*Accent].vmax=GNUMBER(GetGadgAdr(42));
							if (z<SmpData[AktSmp+10*Accent].vmin)
								SmpData[AktSmp+10*Accent].vmax=SmpData[AktSmp+10*Accent].vmin;
							else
							if (z>127)
								SmpData[AktSmp+10*Accent].vmax=127;
							RefreshThem();
							break;
				case 43: z=SmpData[AktSmp+10*Accent].vmin=GNUMBER(GetGadgAdr(43));
							if (z>SmpData[AktSmp+10*Accent].vmax)
								SmpData[AktSmp+10*Accent].vmin=SmpData[AktSmp+10*Accent].vmax;
							else
							if (z<1)
								SmpData[AktSmp+10*Accent].vmin=1;
							RefreshThem();
							break;
				case 44: z=SmpData[AktSmp+10*Accent].ach=GNUMBER(GetGadgAdr(44))-1;
							if (z<0 || z>3)
								SmpData[AktSmp+10*Accent].ach=0;
							RefreshThem();
							break;
				case 45: z=SmpData[AktSmp+10*Accent].mch=GNUMBER(GetGadgAdr(45));
							if (z<0 || z>16)
								SmpData[AktSmp+10*Accent].mch=0;
							RefreshThem();
							break;
				}
			}
		else
		if (class==GADGETDOWN)
			{
			switch (id)
				{
				case 35: while (igad[4].Flags&SELECTED)
								{
								SetSpeed(MODE_PLUS);
								Delay(5);
								}
							break;
				case 36: while (igad[5].Flags&SELECTED)
								{
								SetSpeed(MODE_MINUS);
								Delay(5);
								}
							break;
				case 37: while (igad[6].Flags&SELECTED)
								{
								SetLength(MODE_PLUS);
								Delay(5);
								}
							break;
				case 38: while (igad[7].Flags&SELECTED)
								{
								SetLength(MODE_MINUS);
								Delay(5);
								}
							break;
				case 39: while (igad[8].Flags&SELECTED)
								{
								SetQuant(MODE_PLUS);
								Delay(10);
								}
							break;
				case 40: while (igad[9].Flags&SELECTED)
								{
								SetQuant(MODE_MINUS);
								Delay(10);
								}
							break;
				case 23: AktSmp=code+1; RefreshThem(); break;
				}
			}
		else
		if (class==MOUSEBUTTONS)
			{
			switch (code)
				{
				case SELECTDOWN:	SetEvent(MX,MY); break;
				case MENUDOWN: 	ClearEvent(MX,MY); break;
				}
			}
		else
		if (class==VANILLAKEY)
			{
			if (code>='a' && code <='z') SetPat(code-'a',MODE_NORMAL);
			}
		}
		if (signal&1<<SigNr)
			{
			if (Rec) Record(SNr);
			else PlaySound(SNr);
			}
		if (signal&1<<MSigNr) DrawImage(rp,&metroim[AktMetro],330,100);
		} /* while */
	}

void InputInt()
	{
	/* ALT_L:55 ALT_R:53 SHIFT_L:63 SHIFT_R:61 */
	int_start();
	if (KEYBOARD==95)
		{
		if (KeyNr!=1+alt_key*10)
			{
			SNr=KeyNr=1+alt_key*10;
			Signal(Process,1<<SigNr);
			}
		}
	else
	if (KEYBOARD==93)
		{
		if (KeyNr!=2+alt_key*10)
			{
			SNr=KeyNr=2+alt_key*10;
			Signal(Process,1<<SigNr);
			}
		}
	else
	if (KEYBOARD==91)
		{
		if (KeyNr!=3+alt_key*10)
			{
			SNr=KeyNr=3+alt_key*10;
			Signal(Process,1<<SigNr);
			}
		}
	else
	if (KEYBOARD==89)
		{
		if (KeyNr!=4+alt_key*10)
			{
			SNr=KeyNr=4+alt_key*10;
			Signal(Process,1<<SigNr);
			}
		}
	else
	if (KEYBOARD==87)
		{
		if (KeyNr!=5+alt_key*10)
			{
			SNr=KeyNr=5+alt_key*10;
			Signal(Process,1<<SigNr);
			}
		}
	else
	if (KEYBOARD==85)
		{
		if (KeyNr!=6+alt_key*10)
			{
			SNr=KeyNr=6+alt_key*10;
			Signal(Process,1<<SigNr);
			}
		}
	else
	if (KEYBOARD==83)
		{
		if (KeyNr!=7+alt_key*10)
			{
			SNr=KeyNr=7+alt_key*10;
			Signal(Process,1<<SigNr);
			}
		}
	else
	if (KEYBOARD==81)
		{
		if (KeyNr!=8+alt_key*10)
			{
			SNr=KeyNr=8+alt_key*10;
			Signal(Process,1<<SigNr);
			}
		}
	else
	if (KEYBOARD==79)
		{
		if (KeyNr!=9+alt_key*10)
			{
			SNr=KeyNr=9+alt_key*10;
			Signal(Process,1<<SigNr);
			}
		}
	else
	if (KEYBOARD==77)
		{
		if (KeyNr!=10+alt_key*10)
			{
			SNr=KeyNr=10+alt_key*10;
			Signal(Process,1<<SigNr);
			}
		}
	else
	if (KEYBOARD==55 || KEYBOARD==53) alt_key=1;
	else
	if (KEYBOARD==54 || KEYBOARD==52) alt_key=0;
	else
		KeyNr=-1;
	int_end();
	}

void MidiInput()
	{
	geta4();
	while (1)
		{
		if (((midi_v1=midimayget())&0xf0)==0x90)
			{
			while ((midi_v2=midimayget())==-1);
			while ((midi_v3=midimayget())==-1);
			if (midi_v3)
				{
				midi_v1=(midi_v1&0x0f)+1;
				for (midi_i=1;midi_i<21;midi_i++)
					{
					if (SmpData[midi_i].note==midi_v2 && SmpData[midi_i].vmin<=midi_v3 && SmpData[midi_i].vmax>=midi_v3)
						if (!SmpData[midi_i].mch || SmpData[midi_i].mch==midi_v1)
							{
							SNr=midi_i;
							Signal(Process,1<<SigNr);
							break;
							}
					}
				}
			}
		/**((short *)0xdff180)=rand();*/
		}
	}

void PlaySound(short n) /* BeatStomper V4.0!!! */
  {
  register ULONG *dmabaseLONG=(ULONG *)0xdff000;
  register UWORD *dmabaseWORD=(UWORD *)0xdff000;
  register SHORT i;
  SHORT c[4];
  c[0]=c[1]=c[2]=c[3]=0;
  c[SmpData[n].ach]=n;
  *(dmabaseWORD+0x096/2)=15-(8*(c[3]==0)+4*(c[2]==0)+2*(c[1]==0)+(c[0]==0));
  for (i=0;i<4;i++)
	 {
	 *(dmabaseLONG+(16*i+0x0a0)/4)=(ULONG)SmpAdr[c[i]];
	 *(dmabaseWORD+(16*i+0x0a4)/2)=SmpData[c[i]].length/2;
	 if (c[i])
		{
		*(dmabaseWORD+(16*i+0x0a6)/2)=SmpData[c[i]].per;
		*(dmabaseWORD+(16*i+0x0a8)/2)=SmpData[c[i]].vol;
		}
	 }
  *(dmabaseWORD+0x09e/2)=0x00ff;
  *(dmabaseWORD+0x096/2)=0x820f;
  /*for (i=0;i<100;i++);*/
	;
#asm
	moveq 	#4,d1
.A move.b	$dff006,d0
.B cmp.b 	$dff006,d0
	beq.s 	.B
	dbf		d1,.A
#endasm
  for (i=0;i<4;i++)
	 {
	 *(dmabaseLONG+(16*i+0x0a0)/4)=(ULONG)SmpAdr[0];
	 *(dmabaseWORD+(16*i+0x0a4)/2)=2;
	 }
  }

/*void PlaySound(short n)
	{
	int ch=SmpData[n].ach;
	custom.dmacon=DMAWords[ch][0];
	custom.aud[ch].ac_ptr=SmpAdr[n];
	custom.aud[ch].ac_len=SmpData[n].length/2;
	custom.aud[ch].ac_per=SmpData[n].per;
	custom.aud[ch].ac_vol=SmpData[n].vol;
	custom.dmacon=DMAWords[ch][1];
	*for (i=0;i<DMAWAIT;i++);*
	;
#asm
	moveq 	#4,d1
.A move.b	$dff006,d0
.B cmp.b 	$dff006,d0
	beq.s 	.B
	dbf		d1,.A
#endasm
	custom.aud[ch].ac_ptr=SmpAdr[0];
	custom.aud[ch].ac_len=2;
	}*/

void Record(short n)
	{
	short c=n<11?1:2,pos=(((pos_i1+Quant/2)/Quant)*Quant)%length[AktPat];
	pattern[AktPat][(n-1)%10][pos]=c;
	DrawImage(rp,&boxim[c],pos*8+20,((n-1)%10)*6+23);
	}

void LoadSample()
	{
	UBYTE *fname,title[14]="Load Sample ";
	BPTR handle;
	ULONG len;
	title[12]=AktSmp+64;
	title[13]=0;
	if (!(fname=FileBox(title,smp_path,smp_file))) return;
	if (!(len=GetFileLength(fname))||!(handle=Open(fname,MODE_OLDFILE)))
		{
		ErrorMsg("Can't load sample!");
		return;
		}
	SetPointer(window,busyptr,16,16,-6,0);
	Read8SVX(handle,len,AktSmp,fname);
	ClearPointer(window);
	Close(handle);
	}

void Read8SVX(BPTR handle,ULONG len,SHORT n,UBYTE *fname)
	{
	ULONG check,blen;
	UWORD rate,per;
	Read(handle,&check,4);
	if (check!='FORM')
		{
		ReadRAW(handle,len,n,fname);
		return;
		}
	Seek(handle,4,OFFSET_CURRENT);
	Read(handle,&check,4);
	if (check!='8SVX')
		{
		ErrorMsg("No IFF-8SVX!");
		return;
		}
	Read(handle,&check,4);
	if (check!='VHDR')
		{
		ErrorMsg("Can't find VHDR chunk!");
		return;
		}
	Seek(handle,16,OFFSET_CURRENT);
	Read(handle,&rate,2);
	per=PERIOD(rate);
	if (per>428) per=428;
	if (per<124) per=124;
	Seek(handle,6,OFFSET_CURRENT);
	do
		{
		if (Read(handle,&check,4)!=4)
			{
			ErrorMsg("Can't find BODY chunk!");
			return;
			}
		}
	while (check!='BODY');
	Read(handle,&blen,4);
	if (!(SmpAdr[n]=(APTR)AllocMem(blen,MEMF_CHIP)))
		{
		ErrorMsg("Not enough chip memory!");
		SmpAdr[n]=SmpAdr[n+10];
		return;
		}
	FreeMem(SmpAdr[n+10],SmpData[n].length);
	SmpData[n].per=SmpData[n+10].per=per;
	SmpAdr[n+10]=SmpAdr[n];
	SmpData[n].length=SmpData[n+10].length=blen;
	SmpData[n].vol=SmpData[n+10].vol=64;
	SetSmpName("Loading 8SVX file...",n-1);
	Read(handle,SmpAdr[n],blen);
	strcpy(SmpName[n-1],fname);
	SetSmpName(fname,n-1);
	}

void ReadRAW(BPTR handle,ULONG len,SHORT n,UBYTE *fname)
	{
	Seek(handle,0,OFFSET_BEGINING);
	if (!(SmpAdr[n]=(APTR)AllocMem(len,MEMF_CHIP)))
		{
		ErrorMsg("Not enough chip memory!");
		SmpAdr[n]=SmpAdr[n+10];
		return;
		}
	FreeMem(SmpAdr[n+10],SmpData[n].length);
	SmpData[n].per=SmpData[n+10].per=428;
	SmpAdr[n+10]=SmpAdr[n];
	SmpData[n].length=SmpData[n+10].length=len;
	SmpData[n].vol=SmpData[n+10].vol=64;
	SetSmpName("Loading RAW file...",n-1);
	Read(handle,SmpAdr[n],len);
	strcpy(SmpName[n-1],fname);
	SetSmpName(fname,n-1);
	}

void LoadDrumkit(short mode,char *name)
	{
	UBYTE *fname,dname[100];
	BPTR handle,dhandle;
	ULONG check,len;
	SHORT i;
	if (mode==MODE_NORMAL)
		{
		if (!(fname=FileBox("Load Drumkit",dk_path,dk_file))) return;
		}
	else
		fname=name;
	if (!(handle=Open(fname,MODE_OLDFILE)))
		{
		ErrorMsg("Can't load drumkit!");
		return;
		}
	Read(handle,&check,4);
	if (check!=DKNR)
		{
		ErrorMsg("No BS-Drumkit!");
		Close(handle);
		return;
		}
	SetPointer(window,busyptr,16,16,-6,0);
	for (i=0;i<10;i++)
		{
		Read(handle,dname,100);
		if (!(len=GetFileLength(dname))||!(dhandle=Open(dname,MODE_OLDFILE)))
			{
			ErrorMsg("Can't load sample!");
			ClearPointer(window);
			Close(handle);
			strcpy(DKName,"Unnamed");
			return;
			}
		Read8SVX(dhandle,len,i+1,dname);
		Close(dhandle);
		}
	Read(handle,SmpData,sizeof(SmpData));
	ClearPointer(window);
	Close(handle);
	strcpy(DKName,fname);
	}

void SaveDrumkit()
	{
	UBYTE *fname;
	BPTR handle;
	ULONG check=DKNR;
	if (!(fname=FileBox("Save Drumkit",dk_path,dk_file))) return;
	if (!(handle=Open(fname,MODE_NEWFILE)))
		{
		ErrorMsg("Can't save drumkit!");
		return;
		}
	SetPointer(window,busyptr,16,16,-6,0);
	Write(handle,&check,4);
	Write(handle,SmpName,sizeof(SmpName));
	Write(handle,SmpData,sizeof(SmpData));
	ClearPointer(window);
	Close(handle);
	strcpy(DKName,fname);
	}

void LoadSong()
	{
	UBYTE *fname,dname[100];
	BPTR handle;
	ULONG check;
	if (!(fname=FileBox("Load Song",song_path,song_file))) return;
	if (!(handle=Open(fname,MODE_OLDFILE)))
		{
		ErrorMsg("Can't load song!");
		return;
		}
	Read(handle,&check,4);
	if (check!=SONGNR)
		{
		ErrorMsg("No BS-Song!");
		Close(handle);
		return;
		}
	SetPointer(window,busyptr,16,16,-6,0);
	Read(handle,pattern,sizeof(pattern));
	Read(handle,length,sizeof(length));
	Read(handle,&Speed,2);
	Read(handle,songbuf,SONGSIZE);
	Read(handle,dname,100);
	ClearPointer(window);
	Close(handle);
	SetPat(0,MODE_FORCE);
	SetSpeed(MODE_NOP);
	RefreshGList(GetGadgAdr(17),window,NULL,1);
	if (strcmp(dname,DKName))
		{
		if (AutoRequest(window,&DKText,&PosText,&NegText,NULL,NULL,350,60))
			{
			LoadDrumkit(MODE_NOP,dname);
			RefreshThem();
			}
		}
	}

void SaveSong()
	{
	UBYTE *fname;
	BPTR handle;
	ULONG check=SONGNR;
	if (!(fname=FileBox("Save Song",song_path,song_file))) return;
	if (!(handle=Open(fname,MODE_NEWFILE)))
		{
		ErrorMsg("Can't save song!");
		return;
		}
	SetPointer(window,busyptr,16,16,-6,0);
	Write(handle,&check,4);
	Write(handle,pattern,sizeof(pattern));
	Write(handle,length,sizeof(length));
	Write(handle,&Speed,2);
	Write(handle,songbuf,SONGSIZE);
	Write(handle,DKName,100);
	ClearPointer(window);
	Close(handle);
	}

void LoadPat()
	{
	UBYTE *fname;
	BPTR handle;
	ULONG check;
	if (!(fname=FileBox("Load Pattern",pat_path,pat_file))) return;
	if (!(handle=Open(fname,MODE_OLDFILE)))
		{
		ErrorMsg("Can't load pattern!");
		return;
		}
	Read(handle,&check,4);
	if (check!=PATTNR)
		{
		ErrorMsg("No BS-Pattern!");
		Close(handle);
		return;
		}
	SetPointer(window,busyptr,16,16,-6,0);
	Read(handle,pattern[AktPat],sizeof(pattern[AktPat]));
	Read(handle,&length[AktPat],2);
	ClearPointer(window);
	Close(handle);
	SetPat(AktPat,MODE_FORCE);
	}

void SavePat()
	{
	UBYTE *fname;
	BPTR handle;
	ULONG check=PATTNR;
	if (!(fname=FileBox("Save Pattern",pat_path,pat_file))) return;
	if (!(handle=Open(fname,MODE_NEWFILE)))
		{
		ErrorMsg("Can't save pattern!");
		return;
		}
	SetPointer(window,busyptr,16,16,-6,0);
	Write(handle,&check,4);
	Write(handle,pattern[AktPat],sizeof(pattern[AktPat]));
	Write(handle,&length[AktPat],2);
	ClearPointer(window);
	Close(handle);
	}

void PlayPat()
	{
	Interrupt.is_Node.ln_Type=NT_INTERRUPT;
	Interrupt.is_Node.ln_Pri=0;
	Interrupt.is_Code=PlayPatInt;
	GadgetsAus();
	GetSprite(&sprite,2);
	sprite.x=16;
	sprite.y=22;
	sprite.height=60;
	ChangeSprite(&screen->ViewPort,&sprite,cursor);
	AddICRVector(CIABBase,0,&Interrupt);
	Disable();
	ciab.ciacra=0x11;
	Enable();
	}

void StopPat()
	{
	Disable();
	ciab.ciacra=0x00;
	Enable();
	RemICRVector(CIABBase,0,&Interrupt);
	FreeSprite(2);
	DrawImage(rp,&metroim[1],330,100);
	GadgetsAn();
	Rec=sp_i1=pos_i1=c_i1=0;
	}

void PausePat()
	{
	static pause=0;
	if (pause)
		{
		AddICRVector(CIABBase,0,&Interrupt);
		Disable();
		ciab.ciacra=0x11;
		Enable();
		GADGETON(&igad[2]);
		pause=0;
		}
	else
		{
		Disable();
		ciab.ciacra=0x00;
		Enable();
		RemICRVector(CIABBase,0,&Interrupt);
		GADGETOFF(&igad[2]);
		pause=1;
		}
	RefreshGList(&igad[2],window,NULL,1);
	}

void GadgetsAus()
	{
	struct Gadget *gad=window->FirstGadget;
	do
		{
		GADGETOFF(gad);
		} while (gad=gad->NextGadget);
	GADGETON(&igad[2]);
	GADGETON(&igad[3]);
	GADGETON(&igad[4]);
	GADGETON(&igad[5]);
	GADGETON(&igad[8]);
	GADGETON(&igad[9]);
	GADGETON(GetGadgAdr(1));
	GADGETON(GetGadgAdr(2));
	GADGETON(GetGadgAdr(3));
	GADGETON(GetGadgAdr(4));
	GADGETON(GetGadgAdr(21));
	GADGETON(GetGadgAdr(24));
	GADGETON(GetGadgAdr(25));
	GADGETON(GetGadgAdr(26));
	GADGETON(GetGadgAdr(27));
	RefreshGadgets(window->FirstGadget,window,NULL);
	}

void GadgetsAn()
	{
	struct Gadget *gad=window->FirstGadget;
	do
		{
		GADGETON(gad);
		} while (gad=gad->NextGadget);
	GADGETOFF(&igad[2]);
	GADGETOFF(&igad[3]);
	RefreshGadgets(window->FirstGadget,window,NULL);
	}

void PlaySong()
	{
	char *text="$:$/$";
	struct Gadget *gad=window->FirstGadget;
	if (songbuf[0])
		{
		Interrupt.is_Node.ln_Type=NT_INTERRUPT;
		Interrupt.is_Node.ln_Pri=0;
		Interrupt.is_Code=PlaySongInt;
		do
			{
			GADGETOFF(gad);
			} while (gad=gad->NextGadget);
		RefreshGadgets(window->FirstGadget,window,NULL);
		SetPointer(window,busyptr,16,16,-6,0);
		AddICRVector(CIABBase,0,&Interrupt);
		Disable();
		ciab.ciacra=0x11;
		Enable();
		SetAPen(&screen->RastPort,0);
		SetBPen(&screen->RastPort,1);
		while (!BOTHDOWN && !MIDDOWN)
			{
			WaitTOF();
			text[0]=pat_i2+65;
			text[2]=ccou_i2+49;
			text[4]=cou_i2+48;
			Move(&screen->RastPort,400,7);
			Text(&screen->RastPort,text,5);
			}
		Disable();
		ciab.ciacra=0x00;
		Enable();
		RemICRVector(CIABBase,0,&Interrupt);
		Move(&screen->RastPort,400,7);
		Text(&screen->RastPort,"     ",5);
		ClearPointer(window);
		GadgetsAn();
		sp_i2=c_i2=ccou_i2=pos_i2=0;
		}
	else
		ErrorMsg("Song buffer empty!");
	}

void PlaySongMidi()
	{
	char *text="$:$/$";
	short c;
	struct Gadget *gad=window->FirstGadget;
	if (songbuf[0])
		{
		do
			{
			GADGETOFF(gad);
			} while (gad=gad->NextGadget);
		RefreshGadgets(window->FirstGadget,window,NULL);
		SetPointer(window,busyptr,16,16,-6,0);
		SetAPen(&screen->RastPort,0);
		SetBPen(&screen->RastPort,1);
		sp_i2=2;
		while (midimayget()!=0xfa) if (BOTHDOWN || MIDDOWN) goto Break;
		while (1)
			{
			while ((c=midimayget())==-1);
			if (c==0xfc || BOTHDOWN || MIDDOWN) goto Break;
			if (c==0xf8)
				{
				PlaySongIntMidi();
				text[0]=pat_i2+65;
				text[2]=ccou_i2+49;
				text[4]=cou_i2+48;
				Move(&screen->RastPort,400,7);
				Text(&screen->RastPort,text,5);
				}
			}
Break:
		Move(&screen->RastPort,400,7);
		Text(&screen->RastPort,"     ",5);
		ClearPointer(window);
		GadgetsAn();
		sp_i2=c_i2=ccou_i2=pos_i2=0;
		}
	else
		ErrorMsg("Song buffer empty!");
	}

void PlayPatInt()
	{
	int_start();
	if (sp_i1<2) { sp_i1++; goto Break; }
	sp_i1=0;
	MoveSprite(&screen->ViewPort,&sprite,pos_i1*8+16,22);
	for (i_i1=0;i_i1<10;i_i1++)
		{
		if (pattern[AktPat][i_i1][pos_i1]==1) n_i1[SmpData[i_i1+1].ach]=i_i1+1;
		else
		if (pattern[AktPat][i_i1][pos_i1]==2) n_i1[SmpData[i_i1+11].ach]=i_i1+11;
		}
	custom.dmacon=((n_i1[0]!=0)*DMAWords[0][0])|((n_i1[1]!=0)*DMAWords[1][0])|((n_i1[2]!=0)*DMAWords[2][0])|((n_i1[3]!=0)*DMAWords[3][0]);
	for (i_i1=0;i_i1<4;i_i1++)
		{
		custom.aud[i_i1].ac_ptr=SmpAdr[n_i1[i_i1]];
		custom.aud[i_i1].ac_len=SmpData[n_i1[i_i1]].length/2;
		if (n_i1[i_i1])
			{
			custom.aud[i_i1].ac_per=SmpData[n_i1[i_i1]].per;
			custom.aud[i_i1].ac_vol=SmpData[n_i1[i_i1]].vol;
			}
		}
	n_i1[0]=n_i1[1]=n_i1[2]=n_i1[3]=0;
	if (Rec&&Metro&&!(pos_i1%4))
		{
		AktMetro=c_i1++;
		c_i1%=4;
		Signal(Process,1<<MSigNr);
		if (!(pos_i1%8))
			{
			custom.dmacon=0x0008;
			custom.aud[3].ac_ptr=metrosnd;
			custom.aud[3].ac_len=sizeof(metrosnd)/2;
			custom.aud[3].ac_per=268;
			custom.aud[3].ac_vol=64;
			}
		}
	custom.dmacon=0x820f;
	custom.dmacon=(MCh[0]*DMAWords[0][0])|(MCh[1]*DMAWords[1][0])|(MCh[2]*DMAWords[2][0])|(MCh[3]*DMAWords[3][0]);
	for (i_i1=0;i_i1<100;i_i1++);
	for (i_i1=0;i_i1<4;i_i1++)
		{
		custom.aud[i_i1].ac_ptr=SmpAdr[0];
		custom.aud[i_i1].ac_len=2;
		}
	pos_i1++;
	if (pos_i1==length[AktPat]) pos_i1=0;
Break:
	int_end();
	}

void PlaySongInt()
	{
	int_start();
	if (!songbuf[c_i2]) { c_i2=0; goto Break; }
	pat_i2=songbuf[c_i2+1]-97;
	cou_i2=songbuf[c_i2]-48;
	if (pat_i2<0||pat_i2>25||cou_i2<1||cou_i2>9) { c_i2+=2; goto Break; }
	if (sp_i2<2) { sp_i2++; goto Break; }
	sp_i2=0;
	for (i_i2=0;i_i2<10;i_i2++)
		{
		if (pattern[pat_i2][i_i2][pos_i2]==1) n_i2[SmpData[i_i2+1].ach]=i_i2+1;
		else
		if (pattern[pat_i2][i_i2][pos_i2]==2) n_i2[SmpData[i_i2+11].ach]=i_i2+11;
		}
	custom.dmacon=((n_i2[0]!=0)*DMAWords[0][0])|((n_i2[1]!=0)*DMAWords[1][0])|((n_i2[2]!=0)*DMAWords[2][0])|((n_i2[3]!=0)*DMAWords[3][0]);
	for (i_i2=0;i_i2<4;i_i2++)
		{
		custom.aud[i_i2].ac_ptr=SmpAdr[n_i2[i_i2]];
		custom.aud[i_i2].ac_len=SmpData[n_i2[i_i2]].length/2;
		if (n_i2[i_i2])
			{
			custom.aud[i_i2].ac_per=SmpData[n_i2[i_i2]].per;
			custom.aud[i_i2].ac_vol=SmpData[n_i2[i_i2]].vol;
			}
		}
	n_i2[0]=n_i2[1]=n_i2[2]=n_i2[3]=0;
	custom.dmacon=0x820f;
	for (i_i2=0;i_i2<100;i_i2++);
	for (i_i2=0;i_i2<4;i_i2++)
		{
		custom.aud[i_i2].ac_ptr=SmpAdr[0];
		custom.aud[i_i2].ac_len=2;
		}
	pos_i2++;
	if (pos_i2==length[pat_i2]) { pos_i2=0; ccou_i2++; }
	if (ccou_i2==cou_i2) { c_i2+=2; ccou_i2=0; }
Break:
	int_end();
	}

void PlaySongIntMidi()
	{
	if (!songbuf[c_i2]) c_i2=0;
	pat_i2=songbuf[c_i2+1]-97;
	cou_i2=songbuf[c_i2]-48;
	if (pat_i2<0||pat_i2>25||cou_i2<1||cou_i2>9) { c_i2+=2; return; }
	if (sp_i2<2) { sp_i2++; return; }
	sp_i2=0;
	for (i_i2=0;i_i2<10;i_i2++)
		{
		if (pattern[pat_i2][i_i2][pos_i2]==1) n_i2[SmpData[i_i2+1].ach]=i_i2+1;
		else
		if (pattern[pat_i2][i_i2][pos_i2]==2) n_i2[SmpData[i_i2+11].ach]=i_i2+11;
		}
	custom.dmacon=((n_i2[0]!=0)*DMAWords[0][0])|((n_i2[1]!=0)*DMAWords[1][0])|((n_i2[2]!=0)*DMAWords[2][0])|((n_i2[3]!=0)*DMAWords[3][0]);
	for (i_i2=0;i_i2<4;i_i2++)
		{
		custom.aud[i_i2].ac_ptr=SmpAdr[n_i2[i_i2]];
		custom.aud[i_i2].ac_len=SmpData[n_i2[i_i2]].length/2;
		if (n_i2[i_i2])
			{
			custom.aud[i_i2].ac_per=SmpData[n_i2[i_i2]].per;
			custom.aud[i_i2].ac_vol=SmpData[n_i2[i_i2]].vol;
			}
		}
	n_i2[0]=n_i2[1]=n_i2[2]=n_i2[3]=0;
	custom.dmacon=0x820f;
	for (i_i2=0;i_i2<100;i_i2++);
	for (i_i2=0;i_i2<4;i_i2++)
		{
		custom.aud[i_i2].ac_ptr=SmpAdr[0];
		custom.aud[i_i2].ac_len=2;
		}
	pos_i2++;
	if (pos_i2==length[pat_i2]) { pos_i2=0; ccou_i2++; }
	if (ccou_i2==cou_i2) { c_i2+=2; ccou_i2=0; }
	}

void EraseSong()
	{
	int x,y,z;
	SetPointer(window,busyptr,16,16,-6,0);
	for (x=0;x<26;x++)
		for (y=0;y<10;y++)
			for (z=0;z<64;z++)
				pattern[x][y][z]=0;
	for (x=0;x<26;x++) length[x]=64;
	ClearPointer(window);
	songbuf[0]=0;
	SetPat(0,MODE_FORCE);
	RefreshGList(GetGadgAdr(17),window,NULL,1);
	}

void EraseSample(short A)
	{
	FreeMem(SmpAdr[A],SmpData[A].length);
	SmpData[A].length=SmpData[A+10].length=4;
	SmpAdr[A]=SmpAdr[A+10]=(APTR)AllocMem(4,MEMF_CHIP|MEMF_CLEAR);
	SmpData[A].per=SmpData[A+10].per=0;
	SmpData[A].vol=SmpData[A+10].vol=0;
	SmpData[A].note=SmpData[A+10].note=36+A-1;
	SmpData[A].vmax=127; SmpData[A+10].vmax=63;
	SmpData[A].vmin=64; SmpData[A+10].vmin=1;
	SmpData[A].ach=SmpData[A+10].ach=0;
	SmpData[A].mch=SmpData[A+10].mch=0;
	SmpName[A-1][0]=0;
	SetSmpName(NULL,A-1);
	}

void EraseSmps()
	{
	int i;
	for (i=1;i<11;i++) EraseSample(i);
	strcpy(DKName,"Unnamed");
	RefreshThem();
	}

void SetPat(short n,short mode)
	{
	char buffer[18]="Editing Pattern ?";
	int i,j;
	if (AktPat!=n || mode==MODE_FORCE)
		{
		buffer[16]=n+'A';
		SetAPen(rp,1);
		Move(rp,157,103+TB);
		Text(rp,buffer,17);
		if (mode==MODE_NORMAL || mode==MODE_FORCE)
			{
			SetPointer(window,busyptr,16,16,-6,0);
			for (i=0;i<64;i++)
				for (j=0;j<10;j++)
					DrawImage(rp,&boxim[pattern[n][j][i]],i*8+20,j*6+23);
			ClearPointer(window);
			}
		AktPat=n;
		SetLength(MODE_NOP);
		}
	}

void SetSmpName(char *name,short n)
	{
	char text[100];
	int i;
	if (name)
		{
		i=strlen(name)-1;
		while (i>=0 && name[i]!=':' && name[i]!='/') i--;
		strcpy(text,&name[++i]);
		}
	else
		strcpy(text,"No file loaded");
	text[20]=0;
	SetAPen(rp,1);
	Move(rp,474,n*10+101+TB);
	Text(rp,"                    ",20);
	Move(rp,474,n*10+101+TB);
	Text(rp,text,strlen(text));
	}

void SetLength(short mode)
	{
	char text[10];
	if (mode==MODE_PLUS)
		{
		if (length[AktPat]==64) return;
		length[AktPat]+=8;
		}
	else
	if (mode==MODE_MINUS)
		{
		if (length[AktPat]==8) return;
		length[AktPat]-=8;
		}
	sprintf(text,"Len: %d/4",length[AktPat]/8);
	SetAPen(rp,1);
	Move(rp,170,230+TB);
	Text(rp,text,strlen(text));
	}

void SetSpeed(short mode)
	{
	char text[10];
	UWORD timer;
	if (mode==MODE_PLUS)
		{
		if (Speed==300) return;
		Speed++;
		}
	else
	if (mode==MODE_MINUS)
		{
		if (Speed==40) return;
		Speed--;
		}
	sprintf(text,"BPM: %3d",Speed);
	SetAPen(rp,1);
	Move(rp,170,214+TB);
	Text(rp,text,strlen(text));
	timer=CIATIME/Speed;
	ciab.ciatalo=timer&0xff;
	timer>>=8;
	ciab.ciatahi=timer;
	}

void SetQuant(short mode)
	{
	char text[12];
	if (mode==MODE_PLUS)
		{
		if (Quant==1) return;
		Quant/=2;
		}
	else
	if (mode==MODE_MINUS)
		{
		if (Quant==16) return;
		Quant*=2;
		}
	sprintf(text,"Quant: 1/%-2d",32/Quant);
	SetAPen(rp,1);
	Move(rp,170,246+TB);
	Text(rp,text,strlen(text));
	}

void SetWB()
	{
	if (WBench)
		{
		if (CloseWorkBench())
			WBench=0;
		else
			GT_SetGadgetAttrs(GetGadgAdr(22),window,NULL,GTCB_Checked,TRUE,TAG_END);
		}
	else
		{
		OpenWorkBench();
		WBench=1;
		}
	}

void SetEvent(short x,short y)
	{
	x=(x-20)/8;
	y=(y-23)/6;
	if (x<0 || x>63 || y<0 || y>9) return;
	if (pattern[AktPat][y][x]==1)
		{
		pattern[AktPat][y][x]=2;
		DrawImage(rp,&boxim[2],x*8+20,y*6+23);
		}
	else
		{
		pattern[AktPat][y][x]=1;
		DrawImage(rp,&boxim[1],x*8+20,y*6+23);
		}
	}

void ClearEvent(short x,short y)
	{
	x=(x-20)/8;
	y=(y-23)/6;
	if (x<0 || x>63 || y<0 || y>9) return;
	pattern[AktPat][y][x]=0;
	DrawImage(rp,&boxim[0],x*8+20,y*6+23);
	}

void Cut()
	{
	int i,j;
	for (i=0;i<64;i++)
		for (j=0;j<10;j++)
			{
			cbuffer[j][i]=pattern[AktPat][j][i];
			pattern[AktPat][j][i]=0;
			}
	SetPat(AktPat,MODE_FORCE);
	}

void Copy()
	{
	int i,j;
	for (i=0;i<64;i++)
		for (j=0;j<10;j++)
			cbuffer[j][i]=pattern[AktPat][j][i];
	}

void Paste()
	{
	int i,j;
	for (i=0;i<64;i++)
		for (j=0;j<10;j++)
			pattern[AktPat][j][i]=cbuffer[j][i];
	SetPat(AktPat,MODE_FORCE);
	}

void Mix()
	{
	int i,j;
	for (i=0;i<64;i++)
		for (j=0;j<10;j++)
			if (cbuffer[j][i]) pattern[AktPat][j][i]=cbuffer[j][i];
	SetPat(AktPat,MODE_FORCE);
	}

void RefreshThem()
	{
	short A=AktSmp+10*Accent;
	Note2Str(SmpData[A].note,GSTRING(GetGadgAdr(41)));
	RefreshGList(GetGadgAdr(41),window,NULL,1);
	GT_SetGadgetAttrs(GetGadgAdr(42),window,NULL,GTIN_Number,(long)SmpData[A].vmax,TAG_END);
	GT_SetGadgetAttrs(GetGadgAdr(43),window,NULL,GTIN_Number,(long)SmpData[A].vmin,TAG_END);
	GT_SetGadgetAttrs(GetGadgAdr(44),window,NULL,GTIN_Number,(long)SmpData[A].ach+1,TAG_END);
	GT_SetGadgetAttrs(GetGadgAdr(45),window,NULL,GTIN_Number,(long)SmpData[A].mch,TAG_END);
	GT_SetGadgetAttrs(GetGadgAdr(29),window,NULL,GTSL_Level,(long)SmpData[A].vol,TAG_END);
	GT_SetGadgetAttrs(GetGadgAdr(30),window,NULL,GTSL_Level,(long)(428-SmpData[A].per),TAG_END);
	}

short Str2Note(char *s)
	{
	short n;
	static char *notes[12] =
		{ "C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-" };
	int i;
	for (i=0;i<12;i++)
		{
		if (!strncmp(ucase(s),notes[i],2))
			{
			n=i+12*(s[2]-'0');
			if (n>=0 && n<=127) return n;
			}
		}
	return 0;
	}

void Note2Str(short n,char *s)
	{
	static char *notes[12] =
		{ "C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-" };
	strcpy(s,notes[n%12]);
	s[2]=n/12+'0';
	s[3]=0;
	}

char *ucase(char *s)
	{
	char *r=s;
	do
		{
		if (*s>96 && *s<123) *s-=32;
		}
	while (*++s);
	return r;
	}

struct Gadget *GetGadgAdr(short id)
	{
	struct Gadget *gad=window->FirstGadget;
	while (gad->GadgetID!=id || !gad) gad=gad->NextGadget;
	return gad;
	}

void ErrorMsg(char *text)
	{
	static struct IntuiText Text = { 1,0,JAM1,10,10,NULL,NULL,NULL };
	Text.IText=text;
	AutoRequest(window,&Text,NULL,&OKText,NULL,NULL,IntuiTextLength(&Text)+50,60);
	}

char *FileBox(char *title,char *path,char *file)
	{
	if (!AslRequestTags(freq,ASL_Window,window,ASL_Hail,title,ASL_Dir,path,ASL_File,file,TAG_END))
		return NULL;
	strcpy(path,freq->rf_Dir);
	strcpy(file,freq->rf_File);
	strcpy(fb_loadfile,freq->rf_Dir);
	if (fb_loadfile[strlen(fb_loadfile)-1]!=':' && fb_loadfile[0]!=0)
		strcat(fb_loadfile,"/");
	return (char *)strcat(fb_loadfile,freq->rf_File);
	}

ULONG GetFileLength(UBYTE *name)
	{
	BPTR lock;
	ULONG flen;
	struct FileInfoBlock *FileInfo;
	if (!(lock=Lock(name,ACCESS_READ))) return(0);
	FileInfo=AllocMem(sizeof(struct FileInfoBlock),MEMF_PUBLIC);
	Examine(lock,FileInfo);
	flen=FileInfo->fib_Size;
	FreeMem(FileInfo,sizeof(struct FileInfoBlock));
	UnLock(lock);
	return(flen);
	}

void OpenAll()
	{
	int i,n=36;
	struct Gadget *gad;
	if (!(GfxBase=OpenLibrary("graphics.library",37))) exit(100);
	if (!midiopen()) exit(200);
	IntuitionBase=OpenLibrary("intuition.library",0);
	GadToolsBase=OpenLibrary("gadtools.library",0);
	CIABBase=OpenResource("ciab.resource");
	if (!(AslBase=OpenLibrary("asl.library",0)))
		{
		CloseAll();
		exit(300);
		}
	if (!(screen=OpenScreenTags(&NewScreen,SA_Pens,(long)&Pens_3D,TAG_DONE)))
		{
		CloseAll();
		exit(0);
		}
	VInfo=GetVisualInfo(screen,TAG_END);
	for (i=0; i<ANZ_GAD; i++)
		{
		ngad[i].ng_VisualInfo=VInfo;
		ngad[i].ng_TextAttr=screen->Font;
		}
	gad=CreateContext(&GadgetList);
	igad[9].NextGadget=gad;
	NewWindow.FirstGadget=igad;
	gad=CreateGadget(BUTTON_KIND,gad,ngad,TAG_END);
	gad=CreateGadget(BUTTON_KIND,gad,ngad+1,TAG_END);
	gad=CreateGadget(BUTTON_KIND,gad,ngad+2,TAG_END);
	gad=CreateGadget(BUTTON_KIND,gad,ngad+3,TAG_END);
	gad=CreateGadget(BUTTON_KIND,gad,ngad+4,TAG_END);
	gad=CreateGadget(BUTTON_KIND,gad,ngad+5,TAG_END);
	gad=CreateGadget(BUTTON_KIND,gad,ngad+6,TAG_END);
	gad=CreateGadget(BUTTON_KIND,gad,ngad+7,TAG_END);
	gad=CreateGadget(BUTTON_KIND,gad,ngad+8,TAG_END);
	gad=CreateGadget(BUTTON_KIND,gad,ngad+9,TAG_END);
	gad=CreateGadget(BUTTON_KIND,gad,ngad+10,TAG_END);
	gad=CreateGadget(BUTTON_KIND,gad,ngad+11,TAG_END);
	gad=CreateGadget(BUTTON_KIND,gad,ngad+12,TAG_END);
	gad=CreateGadget(BUTTON_KIND,gad,ngad+13,TAG_END);
	gad=CreateGadget(BUTTON_KIND,gad,ngad+14,TAG_END);
	gad=CreateGadget(BUTTON_KIND,gad,ngad+15,TAG_END);
	gad=CreateGadget(STRING_KIND,gad,ngad+16,GTST_MaxChars,SONGSIZE-1,GTST_String,NULL,TAG_END);
	gad=CreateGadget(BUTTON_KIND,gad,ngad+17,TAG_END);
	gad=CreateGadget(CHECKBOX_KIND,gad,ngad+18,GTCB_Checked,TRUE,TAG_END);
	gad=CreateGadget(CHECKBOX_KIND,gad,ngad+19,TAG_END);
	gad=CreateGadget(CHECKBOX_KIND,gad,ngad+20,TAG_END);
	gad=CreateGadget(CHECKBOX_KIND,gad,ngad+21,GTCB_Checked,TRUE,TAG_END);
	gad=CreateGadget(MX_KIND,gad,ngad+22,GTMX_Labels,&glabels[0],GTMX_Spacing,2,TAG_END);
	gad=CreateGadget(CHECKBOX_KIND,gad,ngad+23,GTCB_Checked,TRUE,TAG_END);
	gad=CreateGadget(CHECKBOX_KIND,gad,ngad+24,GTCB_Checked,TRUE,TAG_END);
	gad=CreateGadget(CHECKBOX_KIND,gad,ngad+25,GTCB_Checked,TRUE,TAG_END);
	gad=CreateGadget(CHECKBOX_KIND,gad,ngad+26,GTCB_Checked,TRUE,TAG_END);
	gad=CreateGadget(STRING_KIND,gad,ngad+27,GTST_MaxChars,3,GTST_String,"C-3",TAG_END);
	gad=CreateGadget(INTEGER_KIND,gad,ngad+28,GTIN_MaxChars,3,GTIN_Number,127,TAG_END);
	gad=CreateGadget(INTEGER_KIND,gad,ngad+29,GTIN_MaxChars,3,GTIN_Number,64,TAG_END);
	gad=CreateGadget(INTEGER_KIND,gad,ngad+30,GTIN_MaxChars,2,GTIN_Number,1,TAG_END);
	gad=CreateGadget(INTEGER_KIND,gad,ngad+31,GTIN_MaxChars,2,GTIN_Number,0,TAG_END);
	gad=CreateGadget(CHECKBOX_KIND,gad,ngad+32,TAG_END);
	gad=CreateGadget(SLIDER_KIND,gad,ngad+33,GTSL_Min,0,GTSL_Max,64,GTSL_Level,0,PGA_Freedom,LORIENT_VERT,GA_RelVerify,TRUE,TAG_END);
	gad=CreateGadget(SLIDER_KIND,gad,ngad+34,GTSL_Min,0,GTSL_Max,304,GTSL_Level,304,PGA_Freedom,LORIENT_VERT,GA_RelVerify,TRUE,TAG_END);
	NewWindow.Screen=screen;
	if (!(window=OpenWindow(&NewWindow)))
		{
		CloseAll();
		exit(0);
		}
	rp=window->RPort;
	freq=AllocAslRequest(ASL_FileRequest,NULL);
	SetRGB4(&screen->ViewPort,0,9,9,9);
	SetRGB4(&screen->ViewPort,1,0,0,0);
	SetRGB4(&screen->ViewPort,2,15,15,15);
	SetRGB4(&screen->ViewPort,3,11,6,7);
	SetRGB4(&screen->ViewPort,21,0,15,0);
	DrawBevelBox(rp,130,100,189,13,GT_VisualInfo,VInfo,GTBB_Recessed,TRUE,TAG_END);
	DrawBevelBox(rp,590,216,20,40,GT_VisualInfo,VInfo,TAG_END);
	DrawBevelBox(rp,620,216,20,40,GT_VisualInfo,VInfo,TAG_END);
	DrawImage(rp,&displayim,0,13);
	DrawImage(rp,&metroim[1],330,100);
	for (i=0;i<10;i++)
		SetSmpName(NULL,i);
	SmpData[0].length=4;
	SmpAdr[0]=(APTR)AllocMem(4,MEMF_CHIP|MEMF_CLEAR);
	for (i=1;i<11;i++)
		{
		SmpData[i].length=SmpData[i+10].length=4;
		SmpAdr[i]=SmpAdr[i+10]=(APTR)AllocMem(4,MEMF_CHIP|MEMF_CLEAR);
		SmpData[i].note=n++;
		SmpData[i].vmin=64;
		SmpData[i].vmax=127;
		}
	n=36;
	for (i=11;i<21;i++)
		{
		SmpData[i].note=n++;
		SmpData[i].vmin=1;
		SmpData[i].vmax=63;
		}
	for (i=0;i<26;i++) length[i]=64;
	songbuf=GSTRING(GetGadgAdr(17));
	Process=(struct Process *)FindTask(NULL);
	Process->pr_WindowPtr=(APTR)window;
	InputInterrupt.is_Node.ln_Type=NT_INTERRUPT;
	InputInterrupt.is_Node.ln_Pri=0;
	InputInterrupt.is_Code=InputInt;
	SigNr=AllocSignal(-1);
	MSigNr=AllocSignal(-1);
	AddIntServer(5,&InputInterrupt);
	MidiTask=CreateTask("BSMidiTask",-5,MidiInput,1024);
	LEDOFF;
	}

void CloseAll()
	{
	int i;
	if (window)
		{
		custom.dmacon=0x000f;
		LEDON;
		RemTask(MidiTask);
		RemIntServer(5,&InputInterrupt);
		FreeSignal(MSigNr);
		FreeSignal(SigNr);
		Process->pr_WindowPtr=NULL;
		for (i=0;i<11;i++) FreeMem(SmpAdr[i],SmpData[i].length);
		FreeAslRequest(freq);
		CloseWindow(window);
		}
	if (screen)
		{
		FreeGadgets(GadgetList);
		FreeVisualInfo(VInfo);
		CloseScreen(screen);
		}
	if (AslBase) CloseLibrary(AslBase);
	CloseLibrary(GadToolsBase);
	CloseLibrary(IntuitionBase);
	CloseLibrary(GfxBase);
	midiclose();
	}

void _cli_parse() {}
void _wb_parse() {}
