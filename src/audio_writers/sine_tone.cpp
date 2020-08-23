//
//  sine_tone.cpp
//  audiosample
//
//  Created by Mike Gonzales on 8/22/20.
//

#include "audio_writers.h"

#include "audio_module.h"
#include <math.h>

namespace AudioWriter
{

bool SineTone::Init()
{
	const auto & context = AudioSubmodule::Instance()->GetContext();
	const float hertz = float(context.hertz);
	time = phase * kTau / hertz;
	inited = true;
	
	return false;
}

bool SineTone::Write (float * buffer, int32_t numFrames)
{
	const auto & context = AudioSubmodule::Instance()->GetContext();
	const float hertz = float(context.hertz);
	const float timeStep = 1.0f / float(hertz);
	
	int32_t cursor = 0;
	for( ; cursor < numFrames && time < duration; cursor++, time += timeStep)
	{
		float value = sinf( kTau * time * pitch);
		value *= gain;
		buffer[cursor] = value;
	}
	
	for( ; cursor < numFrames; cursor++, time += timeStep)
		buffer[cursor] = 0.0f;
	
	return false;
}

}
