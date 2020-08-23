//
//  composite.cpp
//  audiosample
//
//  Created by Mike Gonzales on 8/23/20.
//

#include "audio_writers.h"

namespace AudioWriter
{

bool Composite::Init()
{

	return true;
}

bool Composite::Write(float *buffer, int32_t numFrames)
{

	return true;
}

Composite::~Composite()
{
	for (auto * child : children)
	{
		delete child;
	}
}

}
