/*
Copyright (c) 1993-2012, Harald Westphal
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <proto/exec.h>
#include <devices/serial.h>

#define OK_RETURN 1
#define ERROR_RETURN 0
#define midiwrites(s) midiwrite(s,strlen(s))
#define BSYSTEM_EXCLUSIVE 0xf0
#define SYSTEM_EXCLUSIVE midiput(0xf0)
#define BEND_OF_EXCLUSIVE 0xf7
#define END_OF_EXCLUSIVE midiput(0xf7)
#define UNIT_NUMBER(u) ((u)-1)
#define ID_ROLAND 0x41
#define ID_D10 0x16
#define ID_JD800 0x3d

int midiopen(void);
void midiclose(void);
void midiput(unsigned char);
unsigned char midiget(void);
void midiwrite(char *,int);
unsigned char midimayget(void);
