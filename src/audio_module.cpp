//
//  AudioModule.cpp
//  probmon
//
//  Created by Mike Gonzales on 8/17/20.
//

#include "audio_module.h"

//#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>
#include <stdio.h>
#include <stdlib.h>
#include "presentation_modules.h"

#define StackAlloc alloca

AudioSubmodule * AudioSubmodule::sInstance;

inline void PostFMODError(FMOD_RESULT result)
{
	DebugSubmodule::Instance()->SetDebugString(FMOD_ErrorString(result));
}
	
AudioStream::AudioStream(FMOD::System * system) :
	system(system)
{
	FMOD_MODE mode = FMOD_OPENMEMORY_POINT | FMOD_CREATESTREAM | FMOD_OPENRAW | FMOD_LOOP_NORMAL;

	dataBuffer = (float *) calloc(NumSamples(), sizeof(float));
	dataLength = size_t( NumSamples() * sizeof(float) );

	FMOD_CREATESOUNDEXINFO info = {0};
	info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	info.length = timeLength;
	info.numchannels = channels;
	info.defaultfrequency = hertz;
	info.format = FMOD_SOUND_FORMAT_PCMFLOAT;

	errorCode = system->createStream((const char *) dataBuffer, mode, &info, &handle);
	PostFMODError(errorCode);
}

AudioStream::~AudioStream()
{
	handle->release();
	free(dataBuffer);
}

AudioStream * AudioStream::Create(FMOD::System * system)
{
	AudioStream * ret = new AudioStream(system);
	return ret;
}

void AudioStream::Destroy(AudioStream *& data)
{
	delete data;
	data = nullptr;
}

inline void WriteFrame(float * dstBuffer, float * srcBuffer, int32_t channels, int32_t dstIndex, int32_t srcIndx)
{
	switch(channels)
	{
		case 2: dstBuffer[dstIndex * channels + 1] = srcBuffer[srcIndx * channels + 1];
		case 1: dstBuffer[dstIndex * channels + 0] = srcBuffer[srcIndx * channels + 0];
		break;
	}
}

void AudioStream::WriteSamples(float dt, WriteFunction function)
{
	const int32_t framesToWrite = dt * hertz;
	const size_t allocSize = framesToWrite * channels * sizeof(float);
	const int32_t numFrames = NumFrames();
	
	float * tempBuffer = (float*) StackAlloc( allocSize );
	function(tempBuffer, channels, framesToWrite, hertz, writeTime);
	
	if (writeCursor + framesToWrite > NumFrames())
	{
		int32_t numStartFrames = (writeCursor + framesToWrite) - numFrames;
		int32_t numEndFrames = framesToWrite - numStartFrames;
		
		for (int32_t dst = writeCursor, src = 0; dst < numFrames; dst++, src++)
		{
			WriteFrame(dataBuffer, tempBuffer, channels, dst, src);
		}
		
		for (int32_t dst = 0, src = numEndFrames; dst < numStartFrames; dst++, src++)
		{
			WriteFrame(dataBuffer, tempBuffer, channels, dst, src);
		}
		
		writeCursor = numStartFrames;
	}
	else
	{
		for (int32_t dst = writeCursor, src = 0; dst < framesToWrite; dst++, src++)
		{
			WriteFrame(dataBuffer, tempBuffer, channels, dst, src);
		}
		
		writeCursor += framesToWrite;
	}
}

void AudioStream::Start(WriteFunction function)
{
	writeFunction = function;
	system->playSound(handle, nullptr, true, &instance);
	instance->setVolume(0.701f);
	
	if (writeFunction)
	{
		WriteSamples(2.0f / 60.0f, writeFunction);
	}
	
	instance->setPaused(false);
}

void AudioStream::Stop()
{
	instance->stop();
}

void AudioStream::Update(float dt)
{
	WriteSamples(dt, writeFunction);
}

AudioStream * AudioSubmodule::CreateAudioStream()
{
	AudioStream * ret = AudioStream::Create(system);
	pool.push_back(ret);
	return ret;
}

void AudioSubmodule::UpdateAudioStream(float dt)
{
	for(auto audio : pool)
		audio->Update(dt);
}

void AudioSubmodule::DestroyAudioStreams()
{
	for(auto & audio : pool)
		AudioStream::Destroy(audio);
	
	pool.clear();
}

void AudioSubmodule::Init()
{
	FMOD_RESULT result;

	result = FMOD::System_Create(&system);      // Create the main system object.
	if (result != FMOD_OK)
	{
		PostFMODError(result);
	}

	result = system->init(32, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.
	if (result != FMOD_OK)
	{
		PostFMODError(result);
	}
}

void AudioSubmodule::Update()
{
	system->update();
}

void AudioSubmodule::Shutdown()
{
	DestroyAudioStreams();
	system->release();
}

