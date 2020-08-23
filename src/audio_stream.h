//
//  audio_stream.h
//  audiosample
//
//  Created by Mike Gonzales on 8/22/20.
//

#ifndef audio_stream_h
#define audio_stream_h

#include <fmod/fmod_errors.h>
#include <stdint.h>

namespace FMOD
{
	class System;
	class Sound;
	class Channel;
}

namespace AudioWriter
{
	struct Base;
}

struct AudioStream
{
	const int32_t channels = 1;
	const int32_t hertz = 48 * 1000;
	const float timeLength = 2.0f; // in seconds
	
	FMOD::Sound * handle = nullptr;
	FMOD::Channel * instance = nullptr;
	FMOD::System * system = nullptr;
	FMOD_RESULT errorCode = FMOD_OK;
	
	AudioWriter::Base * audioWriter = nullptr;
	
	int32_t NumChannels() const { return channels; }
	int32_t NumFrames() const { return int32_t(hertz * timeLength); }
	int32_t NumSamples() const { return channels * NumFrames(); }
	
	AudioStream(FMOD::System * system, AudioWriter::Base * audioWriter);
	
	~AudioStream();
	
	static AudioStream * Create(FMOD::System * system, AudioWriter::Base * audioWriter);
	
	static void Destroy(AudioStream *& data);
	
	void Start();
	void Stop();
	void Update(float dt);
};

#endif /* audio_stream_h */
