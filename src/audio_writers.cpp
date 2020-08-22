//
//  audio_writers.cpp
//  audiosample
//
//  Created by Mike Gonzales on 8/21/20.
//

#include "audio_writers.h"
#include "audio_module.h"

#include <math.h>

const float kTau = 6.28318530718f;

bool SinWriter::Init()
{
	const auto & context = AudioSubmodule::Instance()->GetContext();
	const float hertz = float(context.hertz);
	time = phase * kTau / hertz;
	inited = true;
	
	return false;
}

bool SinWriter::Write (float * buffer, int32_t numFrames)
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

	
bool WriterOverride::Init()
{
	if (child)
	{
		CopyParams();
		done = child->Init();
		inited = true;
	}
	else {
		done = true;
	}
	
	return done;
}

void WriterOverride::CopyParams()
{
	child->gain = gain;
	child->phase = phase;
	child->pitch = pitch;
	child->duration = duration;
}

bool WriterOverride::Write(float * buffer, int32_t numFrames) 
{
	const auto & context = AudioSubmodule::Instance()->GetContext();
	const float hertz = float(context.hertz);
	
	CopyParams();
	done = child->Write(buffer, numFrames);
	time += numFrames / float(hertz);
	
	if (time > duration)
		done = true;
	
	return done;
}
