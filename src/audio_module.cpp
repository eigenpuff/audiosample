//
//  AudioModule.cpp
//  probmon
//
//  Created by Mike Gonzales on 8/17/20.
//

#include "audio_module.h"

//#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>
#include <fmod/fmod.hpp>
#include <stdio.h>
#include <stdlib.h>
#include "presentation_modules.h"

#define StackAlloc alloca

AudioSubmodule * AudioSubmodule::sInstance;

void PostFMODError(FMOD_RESULT result)
{
	DebugSubmodule::Instance()->SetDebugString(FMOD_ErrorString(result));
}

AudioSubmodule::AudioSubmodule()
{
	BX_ASSERT(sInstance == nullptr);
	sInstance = this;
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
	errorCode = FMOD::System_Create(&system);      // Create the main system object.
	if (errorCode != FMOD_OK)
	{
		PostFMODError(errorCode);
	}
	else
	{
		errorCode = system->init(32, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.
		if (errorCode != FMOD_OK)
		{
			PostFMODError(errorCode);
		}
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

