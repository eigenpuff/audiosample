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

struct AudioStream;
using AudioPool = std::vector<AudioStream*>;

class AudioSubmodule
{
	FMOD::System *system = nullptr;
	FMOD::Sound *buffer = nullptr;
	
	AudioPool pool;
	
	void CreateAudioStream();
	void DestroyAudioStreams();
	void UpdateAudioStream(float dt);
	
public:
	void Init();
	void Update();
	void Shutdown();
};

#endif /* AudioModule_h */
