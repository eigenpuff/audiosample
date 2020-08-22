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
	
	size_t dataLength = 0;
	FMOD_RESULT errorCode;
	
	int32_t writeCursor = 0;
	float writeTime = 0.0f;
	
	WriteFunction writeFunction = nullptr;
	
	int32_t NumChannels() const { return channels; }
	int32_t NumFrames() const { return int32_t(hertz * timeLength); }
	int32_t NumSamples() const { return channels * NumFrames(); }
	
	AudioStream(FMOD::System * system);
	
	~AudioStream();
	
	static AudioStream * Create(FMOD::System * system);
	
	static void Destroy(AudioStream *& data);
	
	void WriteSamples(float dt, WriteFunction function);
	
	void Start(WriteFunction function);
	void Stop();
	void Update(float dt);
};

using AudioPool = std::vector<AudioStream*>;

class AudioSubmodule
{
	FMOD::System *system = nullptr;
	FMOD::Sound *buffer = nullptr;
	
	AudioPool pool;
	
public:

	AudioStream * CreateAudioStream();
	void DestroyAudioStreams();
	void UpdateAudioStream(float dt);
	
	void Init();
	void Update();
	void Shutdown();
};

#endif /* AudioModule_h */
