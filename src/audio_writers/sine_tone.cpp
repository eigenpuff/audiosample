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
	inited = true;	
	return done;
}

bool SineTone::Write (float * buffer, int32_t numFrames)
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
			float value = sinf( kTau * (time * pitch + phase/hertz) );
			value *= gain;
			buffer[cursor++] = value;
		}
	}
	
	if (cursor < numFrames)
	{
		for( ; cursor < numFrames; cursor++, time += timeStep )
			buffer[cursor++] = 0.0f;
	}
	
	done = time >= duration;
	return done;
}

}
