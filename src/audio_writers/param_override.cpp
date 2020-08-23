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
	const auto & context = AudioSubmodule::Instance()->GetContext();
	const float hertz = float(context.hertz);
	
	CopyParams();
	done = child->Write(buffer, numFrames);
	time += numFrames / float(hertz);
	
	if (time > duration)
		done = true;
	
	return done;
}

}
