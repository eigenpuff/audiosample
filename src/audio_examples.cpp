//
//  audio_examples.cpp
//  audiosample
//
//  Created by Mike Gonzales on 8/21/20.
//


#include "entry_point.h"
#include "audio_writers.h"

static AudioStream * sinf = nullptr;

void AppWrapper::StartLogic()
{
	sinf = m_audio.CreateAudioStream(SinFunction);
	sinf->Start();
}


void AppWrapper::UpdateLogic(float dt)
{
	sinf->Update(dt);
	m_audio.UpdateAudioStream(dt);
}


