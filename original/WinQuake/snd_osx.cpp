/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// snd_null.c -- include this instead of all the other snd_* files to have
// no sound code whatsoever

#include "quakedef.h"

#include <AudioUnit/AudioUnit.h>

#define RATE 11025
#define DMABUFSIZE (1 << 16)

AudioComponentInstance aci;
unsigned char dmabuf[DMABUFSIZE];
volatile int dmabufpos = 0;

OSStatus RenderTone(void * ptr, AudioUnitRenderActionFlags * flags, const AudioTimeStamp * ts, UInt32 bus, UInt32 frames, AudioBufferList * data) {
    unsigned char * buffer = (unsigned char *)data->mBuffers[0].mData;
    for (UInt32 i = 0; i < frames; i++) {
        buffer[i] = dmabuf[dmabufpos];
        dmabufpos = (dmabufpos + 1) % DMABUFSIZE;
    }
    return noErr;
}

qboolean SNDDMA_Init(void) {
    AudioComponentDescription defaultOutputDescription;
    defaultOutputDescription.componentType = kAudioUnitType_Output;
    defaultOutputDescription.componentSubType = kAudioUnitSubType_DefaultOutput;
    defaultOutputDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
    defaultOutputDescription.componentFlags = 0;
    defaultOutputDescription.componentFlagsMask = 0;

    AudioComponent defaultOutput = AudioComponentFindNext(NULL, &defaultOutputDescription);
    if (!defaultOutput) {
        Con_Printf("Could not find default audio output\n");
        return 0;
    }

    OSErr err = AudioComponentInstanceNew(defaultOutput, &aci);
    if (err != noErr) {
        Con_Printf("Could not create audio component instance\n");
        return 0;
    }

    AURenderCallbackStruct input;
    input.inputProc = RenderTone;
    err = AudioUnitSetProperty(aci, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &input, sizeof(input));
    if (err != noErr) {
        Con_Printf("Could not define audio callback\n");
        return 0;
    }

    AudioStreamBasicDescription streamFormat;
    streamFormat.mSampleRate = RATE;
    streamFormat.mFormatID = kAudioFormatLinearPCM;
    streamFormat.mFormatFlags = kAudioFormatFlagIsNonInterleaved | kAudioFormatFlagIsPacked;
    streamFormat.mBytesPerPacket = 1;
    streamFormat.mFramesPerPacket = 1;    
    streamFormat.mBytesPerFrame = 1;        
    streamFormat.mChannelsPerFrame = 1;    
    streamFormat.mBitsPerChannel = 8;
    err = AudioUnitSetProperty (aci, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &streamFormat, sizeof(AudioStreamBasicDescription));
    if (err != noErr) {
        Con_Printf("Could not define audio stream\n");
        return 0;
    }

    err = AudioUnitInitialize(aci);
    if (err != noErr) {
        Con_Printf("Could not initialize audio unit\n");
        return 0;
    }

    err = AudioOutputUnitStart(aci);
    if (err != noErr) {
        Con_Printf("Could not start audio unit\n");
        return 0;
    }

    shm = &sn;
    shm->samplebits = 8;
    shm->speed = RATE;
    shm->channels = 1;
    shm->samplepos = 0;
    shm->submission_chunk = 1;
    shm->samples = DMABUFSIZE;
    shm->buffer = dmabuf;

	return 1;
}

int SNDDMA_GetDMAPos(void) {
	return shm->samplepos = dmabufpos;
}

void SNDDMA_Shutdown(void) {
    AudioOutputUnitStop(aci);
    AudioUnitUninitialize(aci);
    AudioComponentInstanceDispose(aci);
}

void SNDDMA_Submit(void) {
}

