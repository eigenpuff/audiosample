//
//  AudioModule.hpp
//  probmon
//
//  Created by Mike Gonzales on 8/17/20.
//

#ifndef AudioModule_h
#define AudioModule_h

#include <fmod/fmod_errors.h>
#include <stdint.h>
#include <vector>

#include "bgfx_utils.h"
#include "audio_writers.h"
#include "audio_stream.h"

namespace FMOD
{
	class System;
	class Sound;
}

void PostFMODError(FMOD_RESULT result);

class AudioSubmodule
{
	FMOD::System *system = nullptr;
	FMOD::Sound *buffer = nullptr;
	FMOD_RESULT errorCode = FMOD_OK;
	
	using AudioPool = std::vector<AudioStream*>;
	AudioPool pool;
	
	static AudioSubmodule * sInstance;
	
public:
	struct Context
	{
		int32_t hertz = 48000;
	};
	
	Context context;
	
	AudioSubmodule();
	
	static AudioSubmodule *Instance() { return sInstance; }

	AudioStream * CreateAudioStream(WriterBase * audioWriter);
	void DestroyAudioStreams();
	void UpdateAudioStream(float dt);
	
	void Init();
	void Update();
	void Shutdown();
	
	const Context & GetContext() const { return context; }
	
	FMOD_RESULT GetError() const { return errorCode; }
};

#endif /* AudioModule_h */
