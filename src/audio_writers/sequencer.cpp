//
//  sequencer.cpp
//  audiosample
//
//  Created by Mike Gonzales on 8/23/20.
//

#include "audio_writers.h"
#include "audio_module.h"

namespace AudioWriter
{

void Sequencer::CalcTotalTime()
{
	timeline.resize(0);
	float ac = 0.0f;
	for( auto time : delays)
	{
		ac += time;
		timeline.push_back(ac);
	}
	
	// possibility of minor bug - if there are a bunch of elements
	// with time delay of zero at the end, and the last one is shorter
	// than the ones before, this time may truncate.  For now, we will
	// just not do that, and fix the root problem later.
	duration = ac + (children.back())->duration;
}

//start with an index, returns the time at which that sound plays and the next index
	
bool Sequencer::Init()
{
	inited = true;
	if (children.size() > 0)
	{
		CalcTotalTime();
		
		for (auto * child : children)
		{
			//child->gain = gain;
			//child->phase = phase;
			//child->pitch = pitch;
			
			child->Init();
		}
		
		// set up some scratch space for mixing children
		scratchLen = 2 * 48000 * sizeof(float);
		scratchSpace = (float*) malloc(scratchLen);
		memset(scratchSpace, 0, scratchLen);
	}
	
	done = children.size() == 0;
	return done;
}

bool Sequencer::Write(float *buffer, int32_t numFrames)
{
	if (done)
		return done;
	
	const auto & context = AudioSubmodule::Instance()->GetContext();
	const float hertz = float(context.hertz);
	const float timeJump = float (numFrames) / hertz;
	
	float childTime = time;
	float endTime = time + timeJump;
	while (childTime < endTime && timelineIndex < timeline.size())
	{
		childTime = timeline[timelineIndex];
		if (childTime < time + timeJump)
		{
			playQueue.push_back(children[timelineIndex]);
			timeQueue.push_back(timeline[timelineIndex]);
			timelineIndex++;
		}
	}
	
	for (int32_t e = 0; e < playQueue.size(); e++)
	{
		auto * child = playQueue[e];
		if (child->done)
			continue;
		
		float childTime = timeQueue[e];
		int32_t childStart = 0;
		if (childTime > time)
		{
			float delay = childTime - time;
			childStart = delay * hertz;
		}
	
		memset(scratchSpace, 0, scratchLen);
		child->Write(scratchSpace + childStart, numFrames - childStart);
		
		// has known mis-performance where if there is zero padding
		// off the end of it, it'll copy in the zeros, too, until it
		// hits numFrames
		for (int c = childStart; c < numFrames; c++)
		{
			buffer[c] += scratchSpace[c];
		}
	}
	
	int32_t popcount = 0;
	for (popcount = 0; popcount < playQueue.size(); popcount++)
	{
		auto * child = playQueue[popcount];
		if (!child->done)
			break;
	}
	
	for ( ; popcount > 0; popcount-- )
	{
		playQueue.pop_front();
		timeQueue.pop_front();
	}
	
	time += timeJump;
	done = time > duration;
	return done;
}

Sequencer::~Sequencer()
{
	for (auto * child : children)
	{
		delete child;
	}
	
	free(scratchSpace);
	scratchSpace = nullptr;
}

void Sequencer::PushChild(Base * child, float cumulDelay)
{
	if (child && cumulDelay >= 0.0f)
	{
		children.push_back(child);
		delays.push_back(cumulDelay);
	}
}

}
