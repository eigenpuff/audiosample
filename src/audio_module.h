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

struct WriterBase
{
	float pitch = 1.0f;
	float gain = 1.0f;
	float phase = 0.0f;
	float time = 0.0f;
	float duration = 0.0f;
	bool done = false;
	bool inited = false;
	
	// does initialization, returns whether to abort
	virtual bool Init () = 0;
	
	// writes to a number of frames to a buffer, returns whether to be done
	virtual bool Write (float * buffer, int32_t numFrames) = 0;
};

struct AudioStream
{
	const int32_t channels = 1;
	const int32_t hertz = 48 * 1000;
	const float timeLength = 2.0f; // in seconds
	
	FMOD::Sound * handle = nullptr;
	FMOD::Channel * instance = nullptr;
	FMOD::System * system = nullptr;
	FMOD_RESULT errorCode = FMOD_OK;
	
	WriterBase * audioWriter = nullptr;
	
	int32_t NumChannels() const { return channels; }
	int32_t NumFrames() const { return int32_t(hertz * timeLength); }
	int32_t NumSamples() const { return channels * NumFrames(); }
	
	AudioStream(FMOD::System * system, WriterBase * audioWriter);
	
	~AudioStream();
	
	static AudioStream * Create(FMOD::System * system, WriterBase * audioWriter);
	
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
	struct Context
	{
		int32_t hertz = 48000;
	};
	
	Context context;
	
	AudioSubmodule()
	{
		BX_ASSERT(sInstance == nullptr);
		sInstance = this;
	}
	
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
