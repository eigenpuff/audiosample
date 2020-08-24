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

float SineWave(float time, float pitch, float phase)
{
	return sinf( kTau * (time * pitch + phase) );
}

float SawWave(float time, float pitch, float phase)
{
	return fmod(time+phase, 1.0f/pitch) * pitch;
}

float SquareWave(float time, float pitch, float phase)
{
	//hacky but works for testing purposes
	const float mod = 1.0f / pitch;
	const float value = fmod(time + phase, 2.0f * mod);
	return value < mod ? 1.0f : -1.0f;
}

float TriangleWave(float time, float pitch, float phase)
{
	return 0;
}

bool Tone::Init()
{
	inited = true;
	done = wave == nullptr;
	return done;
}

bool Tone::Write (float * buffer, int32_t numFrames)
{
	const auto & context = AudioSubmodule::Instance()->GetContext();
	const float hertz = float(context.hertz);
	const float timeStep = 1.0f / float(hertz);
	
	int32_t cursor = 0;
	
	if (time < duration)
	{
		float writeTime = duration - time;
		int32_t writeFrames = int32_t(writeTime * hertz);
		
		if (writeFrames > numFrames)
			writeFrames = numFrames;
		
		for( int32_t writeCursor = 0; writeCursor < writeFrames && cursor < numFrames; writeCursor++, time += timeStep )
		{
			float value = wave(time, pitch, phase/hertz);
			value *= gain;
			buffer[cursor++] = value;
		}
	}
	
	done = time >= duration;
	return done;
}


}
