//
//  sequencer.cpp
//  audiosample
//
//  Created by Mike Gonzales on 8/23/20.
//

#include "audio_writers.h"

namespace AudioWriter
{

bool Sequencer::Init()
{
	return true;
}

bool Sequencer::Write(float *buffer, int32_t numFrames)
{
	return true;
}

Sequencer::~Sequencer()
{
	for (auto * child : children)
	{
		delete child;
	}
}

}
