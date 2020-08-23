//
//  param_override.cpp
//  audiosample
//
//  Created by Mike Gonzales on 8/22/20.
//

#include "audio_writers.h"
#include "audio_module.h"

namespace AudioWriter
{

bool ParamOverride::Init()
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

void ParamOverride::CopyParams()
{
	child->gain = gain;
	child->phase = phase;
	child->pitch = pitch;
	
	child->duration = duration;
}

bool ParamOverride::Write(float * buffer, int32_t numFrames)
{
	CopyParams();
	
	const auto & context = AudioSubmodule::Instance()->GetContext();
	const float hertz = float(context.hertz);
	
	const float timeStep = float (numFrames) / hertz;
	
	if (time < delay)
	{
		if (time + timeStep > delay)
		{
			float delayStep = delay - time;
			float writeStep = timeStep - delayStep;
			int32_t writeFrames = int32_t(writeStep * hertz);
			int32_t writeStart = numFrames - writeFrames;
			
			int32_t channels = 1;
			child->Write(buffer + (writeStart * channels), writeFrames);
		}
	}
	else
	{
		done = child->Write(buffer, numFrames);
	}
	
	time += timeStep;
	return done;
}

}
