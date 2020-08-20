//
//  AudioModule.hpp
//  probmon
//
//  Created by Mike Gonzales on 8/17/20.
//

#ifndef AudioModule_h
#define AudioModule_h
#include <fmod/fmod.hpp>

class AudioSubmodule
{
	FMOD::System *system = nullptr;
	
public:
	void Init();
	void Update();
	void Shutdown();
};

#endif /* AudioModule_h */
