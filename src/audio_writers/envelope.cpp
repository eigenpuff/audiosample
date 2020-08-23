//
//  envelope.cpp
//  audiosample
//
//  Created by Mike Gonzales on 8/23/20.
//
#include "audio_writers.h"

namespace AudioWriter
{

bool Envelope::Init()
{

	return true;
}

bool Envelope::Write(float *buffer, int32_t numFrames)
{

	return true;
}

Envelope::~Envelope()
{
	delete child;
}

}
