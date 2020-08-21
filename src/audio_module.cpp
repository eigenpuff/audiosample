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

#define StackAlloc alloca


struct AudioStream
{
	const int32_t channels = 1;
	const int32_t hertz = 48 * 1000;
	const float timeLength = 2.0f; // in seconds
	
	float * 	dataBuffer = nullptr;
	FMOD::Sound * handle = nullptr;
	FMOD::Channel * instance = nullptr;
	FMOD::System * system = nullptr;
	
	size_t dataLength = 0;
	FMOD_RESULT errorCode;
	
	int32_t writeFrame = 0;
	float writeTime = 0.0f;
	
	using WriteFunction = void (*) (float * buffer, int32_t numChannels, int32_t numSamples, int32_t hertz, float startTime);
	WriteFunction writeFunction = nullptr;
	
	int32_t NumChannels() const { return channels; }
	int32_t NumFrames() const { return int32_t(hertz * timeLength); }
	int32_t NumSamples() const { return channels * NumFrames(); }
	
	AudioStream(FMOD::System * system) :
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
	}
	
	~AudioStream()
	{
		handle->release();
		free(dataBuffer);
	}
	
	static AudioStream * Create(FMOD::System * system)
	{
		AudioStream * ret = new AudioStream(system);
		return ret;
	}
	
	static void Destroy(AudioStream *& data)
	{
		delete data;
		data = nullptr;
	}
	
	inline void WriteFrame(float * dstBuffer, float * srcBuffer, int32_t dstIndex, int32_t srcIndx)
	{
		switch(channels)
		{
			case 2: dstBuffer[dstIndex * channels + 1] = srcBuffer[srcIndx * channels + 1];
			case 1: dstBuffer[dstIndex * channels + 0] = srcBuffer[srcIndx * channels + 0];
			break;
		}
	}
	
	void WriteSamples(float dt, WriteFunction function)
	{
		int32_t numFrames = dt * hertz;
		float * tempBuffer = (float*) StackAlloc( numFrames * channels * sizeof(float) );
		function(tempBuffer, channels, numFrames, hertz, writeTime);
		
		if (writeFrame + numFrames > NumFrames())
		{
			int32_t numStartFrames = (writeFrame + numFrames) - NumFrames();
			int32_t numEndFrames = numFrames - numStartFrames;
			
			for (int32_t dst = writeFrame, src = 0; dst < NumFrames(); dst++, src++)
			{
				WriteFrame(dataBuffer, tempBuffer, dst, src);
			}
			
			for (int32_t dst = 0, src = numEndFrames; dst < numStartFrames; dst++, src++)
			{
				WriteFrame(dataBuffer, tempBuffer, dst, src);
			}
		}
		else
		{
			for (int32_t dst = writeFrame, src = 0; dst < NumFrames(); dst++, src++)
			{
				WriteFrame(dataBuffer, tempBuffer, dst, src);
			}
		}
	}
	
	void Start(WriteFunction function)
	{
		system->playSound(handle, nullptr, true, &instance);
		instance->setVolume(0.701f);
		
		if (writeFunction)
		{
			WriteSamples(2.0f / 60.0f, writeFunction);
		}
		
		instance->setPaused(false);
	}
	
	void Stop()
	{
		instance->stop();
	}
	
	void Update(float dt)
	{
		WriteSamples(dt, writeFunction);
	}
};

void AudioSubmodule::CreateAudioStream()
{
	pool.push_back(AudioStream::Create(system));
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
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
	}

	result = system->init(32, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
	}
	
	CreateAudioStream();
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

