/*
Copyright (c) 1993-2012, Harald Westphal
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <functions.h>
#include "midi.h"

static struct MsgPort *ReadPort,*WritePort;
static struct IOExtSer *ReadRequest,*WriteRequest;
static unsigned char rb,wb;

int midiopen()
	{
	ReadRequest=CreateExtIO((ReadPort=CreatePort(0,0)),sizeof(struct IOExtSer));
	ReadRequest->io_SerFlags=SERF_SHARED|SERF_RAD_BOOGIE;
	if (OpenDevice(SERIALNAME,0,ReadRequest,0))
		{
		DeleteExtIO(ReadRequest);
		DeletePort(ReadPort);
		return ERROR_RETURN;
		}
	ReadRequest->IOSer.io_Length=1;
	ReadRequest->IOSer.io_Data=(APTR)&rb;
	ReadRequest->io_Baud=31250;
	ReadRequest->io_ReadLen=8;
	ReadRequest->io_WriteLen=8;
	ReadRequest->io_CtlChar=0xf4f4f4f4;
	ReadRequest->IOSer.io_Command=SDCMD_SETPARAMS;
	DoIO(ReadRequest);
	ReadRequest->IOSer.io_Command=CMD_READ;
	WriteRequest=CreateExtIO((WritePort=CreatePort(0,0)),sizeof(struct IOExtSer));
	WriteRequest->io_SerFlags=SERF_SHARED|SERF_RAD_BOOGIE;
	WriteRequest->io_StopBits=1;
	if (OpenDevice(SERIALNAME,0,WriteRequest,0))
		{
		DeleteExtIO(ReadRequest);
		DeletePort(ReadPort);
		DeleteExtIO(WriteRequest);
		DeletePort(WritePort);
		return ERROR_RETURN;
		}
	WriteRequest->IOSer.io_Length=1;
	WriteRequest->IOSer.io_Data=(APTR)&wb;
	WriteRequest->io_SerFlags=SERF_SHARED|SERF_XDISABLED;
	WriteRequest->IOSer.io_Command=CMD_WRITE;
	BeginIO(ReadRequest);
	return OK_RETURN;
	}

void midiclose()
	{
	CloseDevice(ReadRequest);
	CloseDevice(WriteRequest);
	DeleteExtIO(ReadRequest);
	DeletePort(ReadPort);
	DeleteExtIO(WriteRequest);
	DeletePort(WritePort);
	}

void midiput(unsigned char c)
	{
	wb=c;
	DoIO(WriteRequest);
	}

unsigned char midiget()
	{
	unsigned char c;
	WaitIO(ReadRequest);
	c=rb;
	BeginIO(ReadRequest);
	return c;
	}

void midiwrite(char *s,int l)
	{
	WriteRequest->IOSer.io_Length=l;
	WriteRequest->IOSer.io_Data=(APTR)s;
	DoIO(WriteRequest);
	WriteRequest->IOSer.io_Length=1;
	WriteRequest->IOSer.io_Data=(APTR)&wb;
	}

unsigned char midimayget()
	{
	unsigned char c;
	if (!CheckIO(ReadRequest)) return -1;
	WaitIO(ReadRequest);
	c=rb;
	BeginIO(ReadRequest);
	return c;
	}
