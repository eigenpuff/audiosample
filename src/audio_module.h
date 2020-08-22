//
//  AudioModule.hpp
//  probmon
//
//  Created by Mike Gonzales on 8/17/20.
//

#ifndef AudioModule_h
#define AudioModule_h

#include <fmod/fmod.hpp>
#include <stdint.h>
#include <vector>
#include "bgfx_utils.h"

using WriteFunction = void (*) (float * buffer, int32_t numChannels, int32_t numFrames, int32_t hertz, float startTime);

struct AudioStream
{
	const int32_t channels = 1;
	const int32_t hertz = 48 * 1000;
	const float timeLength = 2.0f; // in seconds
	
	float * 	dataBuffer = nullptr;
	FMOD::Sound * handle = nullptr;
	FMOD::Channel * instance = nullptr;
	FMOD::System * system = nullptr;
	FMOD_RESULT errorCode = FMOD_OK;
	
	WriteFunction writeFunction = nullptr;
	
	int32_t NumChannels() const { return channels; }
	int32_t NumFrames() const { return int32_t(hertz * timeLength); }
	int32_t NumSamples() const { return channels * NumFrames(); }
	
	AudioStream(FMOD::System * system, WriteFunction function);
	
	~AudioStream();
	
	static AudioStream * Create(FMOD::System * system, WriteFunction function);
	
	static void Destroy(AudioStream *& data);
	
	void Start();
	void Stop();
	void Update(float dt);
};

using AudioPool = std::vector<AudioStream*>;

class AudioSubmodule
{
	FMOD::System *system = nullptr;
	FMOD::Sound *buffer = nullptr;
	FMOD_RESULT errorCode = FMOD_OK;
	AudioPool pool;
	
	static AudioSubmodule * sInstance;
	
public:
	
	AudioSubmodule()
	{
		BX_ASSERT(sInstance == nullptr);
		sInstance = this;
	}
	
	static AudioSubmodule *Instance() { return sInstance; }

	AudioStream * CreateAudioStream(WriteFunction writeFunction);
	void DestroyAudioStreams();
	void UpdateAudioStream(float dt);
	
	void Init();
	void Update();
	void Shutdown();
	
	FMOD_RESULT GetError() const { return errorCode; }
};

#endif /* AudioModule_h */
