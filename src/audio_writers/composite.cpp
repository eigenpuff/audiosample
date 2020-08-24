//
//  composite.cpp
//  audiosample
//
//  Created by Mike Gonzales on 8/23/20.
//

#include "audio_writers.h"
#include "audio_module.h"

namespace AudioWriter
{

bool Composite::Init()
{
	inited = true;
	
	for (auto * child : children)
	{
		child->Init();
	}
	
	// set up some scratch space for mixing children
	scratchLen = 2 * 48000 * sizeof(float);
	scratchSpace = (float*) malloc(scratchLen);
	memset(scratchSpace, 0, scratchLen);
	
	return done;
}

bool Composite::DetermineDone()
{
	bool donetest = true;
	for (auto * child : children)
		donetest &= child->done;
	
	return donetest;
}

bool Composite::Write(float *buffer, int32_t numFrames)
{
	const auto & context = AudioSubmodule::Instance()->GetContext();
	const float hertz = float(context.hertz);
	const float timeJump = float (numFrames) / hertz;
	
	for (auto * child : children)
	{
		memset(scratchSpace, 0, scratchLen);
		child->Write(scratchSpace, numFrames);
		
		for ( int32_t c = 0 ; c < numFrames; c++ )
		{
			buffer[c] += scratchSpace[c];
		}
	}

	time += timeJump;
	done = DetermineDone();
	return done;
}

void Composite::PushChild(Base * child)
{
	children.push_back(child);
}

Composite::~Composite()
{
	for (auto * child : children)
	{
		delete child;
	}
	
	free(scratchSpace);
}

}
