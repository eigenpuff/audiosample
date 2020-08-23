//
//  audio_writers.h
//  audiosample
//
//  Created by Mike Gonzales on 8/21/20.
//

#ifndef audio_writers_h
#define audio_writers_h

#include <stdint.h>

const float kTau = 6.28318530718f;

namespace AudioWriter
{

struct Base
{
	float pitch = 1.0f;
	float gain = 1.0f;
	float phase = 0.0f;
	float time = 0.0f;
	float duration = 0.0f;
	bool  done = false;
	bool  inited = false;
	
	// does initialization, returns whether to abort
	virtual bool Init () = 0;
	
	// writes to a number of frames to a buffer, returns whether to be done
	virtual bool Write (float * buffer, int32_t numFrames) = 0;
};

struct SineTone : Base
{
	bool Init() override;
	bool Write (float * buffer, int32_t numFrames) override;
};

struct ParamOverride : Base
{
	Base * child = nullptr;
	float duration = 0.0f;
	
	void CopyParams();
	
	bool Init() override;
	bool Write(float * buffer, int32_t numFrames) override;
};

}

#endif /* audio_writers_h */
