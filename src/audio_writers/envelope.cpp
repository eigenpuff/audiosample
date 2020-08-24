//
//  envelope.cpp
//  audiosample
//
//  Created by Mike Gonzales on 8/23/20.
//

#include "audio_writers.h"
#include "audio_module.h"
#include <math.h>

namespace AudioWriter
{

float SineEnvelope::operator () (float t)
{
	// multiply by the segment of sine from 0 to PI
	return sinf(t * kTau / 2.0f);
}

float SplineEnvelope::operator () (float t)
{
	// todo
	return 1.0f;
}

float AttackDecayEnvelope::operator () (float t)
{
	//linear ramp from f(0) = 0 to f(.1) = 1,
	if (t < 0.10f)
		return t * 10.0f;
	
	// followed by 1/x fall off, f(.1) = 1 to f(1) = 0
	return (1.0f - t) / (9.0f * t) ;
}

float AttackSustainDecayEnvelope::operator () (float t)
{
	if (t < 0.1f)
		return t * 10.0f;
	if (t < 0.8f)
		return 1.0f;
	
	return (1.0f - t) * 5.0f;
}

bool Envelope::Init()
{
	inited = true;
	if (child)
	{
		child->gain = gain;
		child->phase = phase;
		child->pitch = pitch;
		
		child->duration = duration;
		child->Init();
	}
	
	done = child == nullptr;
	return done;
}

bool Envelope::Write(float *buffer, int32_t numFrames)
{
	const auto context = AudioSubmodule::Instance()->GetContext();
	const float hertz = float(context.hertz);
	
	done = child->Write(buffer, numFrames);
	
	float timeStep = 1.0f / hertz;
	if (envelope)
	{
		for (int32_t frame = 0; frame < numFrames; time += timeStep, frame++)
		{
			float t = time / duration;
			float value = (*envelope)(t);
			buffer[frame] = value * buffer[frame];
		}
	}
	
	return done;
}

Envelope::~Envelope()
{
	delete envelope;
	delete child;
}

}
