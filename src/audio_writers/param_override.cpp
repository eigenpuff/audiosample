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
	
	float * writeBuffer = buffer;
	int32_t writeFrames = numFrames;
	
	if (time < delay)
	{
		if (time + timeStep > delay)
		{
			float delayStep = delay - time;
			float writeStep = timeStep - delayStep;
			writeFrames = int32_t(writeStep * hertz);
			
			int32_t writeStart = numFrames - writeFrames;
			
			// for right now we are developing assuming mono; at some
			// point we will want to fix that.
			int32_t channels = 1;
			writeBuffer = buffer + (writeStart * channels);
			child->Write(writeBuffer, writeFrames);
		}
	}
	else
	{
		done = child->Write(writeBuffer, writeFrames);
	}
	time += timeStep;
	return done;
}

}
