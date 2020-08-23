//
//  audio_examples.cpp
//  audiosample
//
//  Created by Mike Gonzales on 8/21/20.
//


#include "entry_point.h"
#include "audio_writers.h"

static AudioStream * memStream = nullptr;

void AppWrapper::StartLogic()
{
	auto * sin = new AudioWriter::SineTone();
	
	auto * root = new AudioWriter::ParamOverride();
	root->child = sin;
	root->gain = 1.0f;
	root->pitch = 440.0f;
	root->duration = 10.0f;
	
	memStream = m_audio.CreateAudioStream(root);
	memStream->Start();
}


void AppWrapper::UpdateLogic(float dt)
{
	memStream->Update(dt);
	m_audio.UpdateAudioStream(dt);
}


