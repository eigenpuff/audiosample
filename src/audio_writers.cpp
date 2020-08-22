//
//  audio_writers.cpp
//  audiosample
//
//  Created by Mike Gonzales on 8/21/20.
//

#include "audio_writers.h"
#include "audio_module.h"

#include <math.h>

using WriteFunction = void (*) (float * buffer, int32_t numChannels, int32_t numFrames, int32_t hertz, float startTime);

void SinFunction(float * buffer, int32_t numChannels, int32_t numFrames, int32_t hertz, float startTime)
{
	float pitch = 432.0f;
	float gain = 1.0f;
	float phase = 0.0f;
	
	const float timeStep = 1.0f / float(hertz);
	float time = startTime + phase;
	
	for(int32_t cursor = 0; cursor < numFrames; cursor++, time += timeStep)
	{
		float value = sinf(2.0f * 3.14159 * time * pitch);
		value *= gain;
		
		switch (numChannels)
		{
			case 2: buffer[ cursor * numChannels + 1] = value;
			case 1: buffer[ cursor * numChannels + 0] = value;
			break;
		}
	}
}
