//
//  audio_writers.h
//  audiosample
//
//  Created by Mike Gonzales on 8/21/20.
//

#ifndef audio_writers_h
#define audio_writers_h

#include <stdint.h>
#include "audio_module.h"

struct SinWriter : WriterBase
{
	bool Init() override;
	bool Write (float * buffer, int32_t numFrames) override;
};

struct WriterOverride : WriterBase
{
	WriterBase * child = nullptr;
	float duration = 0.0f;
	
	void CopyParams();
	
	bool Init() override;
	bool Write(float * buffer, int32_t numFrames) override;
};

#endif /* audio_writers_h */
