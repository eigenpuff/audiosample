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

void AudioSubmodule::Init()
{
	FMOD_RESULT result;

	result = FMOD::System_Create(&system);      // Create the main system object.
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
	}

	result = system->init(512, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
	}
}

void AudioSubmodule::Update()
{
	system->update();
}

void AudioSubmodule::Shutdown()
{
	system->release();
}

