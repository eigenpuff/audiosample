//
//  audio_stream.cpp
//  audiosample
//
//  Created by Mike Gonzales on 8/22/20.
//

#include "audio_stream.h"

#include <fmod/fmod.hpp>
#include "audio_module.h"


FMOD_RESULT PCMReadCallback_Zero(FMOD_SOUND * soundraw, void * data, uint32_t datalen)
{
	uint8_t * buffer = (uint8_t *) data;
	for (int c = 0; c < datalen; c++)
		buffer[c] = 0;
	
	return FMOD_OK;
}

FMOD_RESULT PCMSetPosCallback_Null(FMOD_SOUND * soundraw, int subsound, uint32_t position, FMOD_TIMEUNIT postype)
{
	return FMOD_OK;
}

FMOD_RESULT PCMReadCallback_Writer(FMOD_SOUND * soundraw,  void * data, uint32_t datalen)
{
	int32_t numChannels = 1;
	int32_t numBits = 32;
	
	FMOD_SOUND_FORMAT format;
	FMOD_SOUND_TYPE type;
	FMOD_Sound_GetFormat(soundraw, &type, &format, &numChannels, &numBits);
	
	const int32_t bufferLen = datalen / sizeof(float);
	const int32_t numFrames = bufferLen / numChannels;
	
	float * buffer = (float *) data;
	
	//over write with zeroes to make sure we have no garbage in
	// in there to begin with
	memset(buffer, 0, datalen);
	
	AudioWriter::Base * writer;
	FMOD_Sound_GetUserData(soundraw, (void**) &writer);
	
	if (writer)
		writer->Write(buffer, numFrames);
	else
		return FMOD_ERR_INVALID_PARAM;

	return FMOD_OK;
}

FMOD_RESULT PCMSetPosCallback_Initialize(FMOD_SOUND * soundraw, int subsound, uint32_t position, FMOD_TIMEUNIT postype)
{
	AudioWriter::Base * writer;
	FMOD_Sound_GetUserData(soundraw, (void**) &writer);
	if (writer && !writer->inited)
		writer->Init();
	else
		return FMOD_ERR_INVALID_PARAM;
	
	return FMOD_OK;
}

AudioStream::AudioStream(FMOD::System * system, AudioWriter::Base * write) :
	system(system), audioTree(write)
{
	if (!system)
		return;
	
	if (AudioSubmodule::Instance()->GetError() == FMOD_OK)
	{
		FMOD_MODE mode = FMOD_OPENUSER | FMOD_CREATESTREAM | FMOD_LOOP_NORMAL;

		FMOD_CREATESOUNDEXINFO info = {0};
		info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
		info.length = uint32_t(timeLength * hertz);
		info.numchannels = channels;
		info.defaultfrequency = hertz;
		info.format = FMOD_SOUND_FORMAT_PCMFLOAT;
		info.pcmreadcallback = PCMReadCallback_Writer;
		info.pcmsetposcallback = PCMSetPosCallback_Initialize;
		info.userdata = (void *) audioTree.root;

		errorCode = system->createStream("", mode, &info, &handle);
		if (errorCode != FMOD_OK)
		{
			PostFMODError(errorCode);
		}
	}
	else
	{
		//if the audio system as a whole has an error, copy it to our error to
		// poison our playback so that this sound doesn't assume everything is
		// all hunky-dory
		errorCode = AudioSubmodule::Instance()->GetError();
	}
}

AudioStream::~AudioStream()
{
	handle->release();
}

AudioStream * AudioStream::Create(FMOD::System * system, AudioWriter::Base * audioWriter)
{
	AudioStream * ret = new AudioStream(system, audioWriter);
	return ret;
}

void AudioStream::Destroy(AudioStream *& data)
{
	delete data;
	data = nullptr;
}

void AudioStream::Start()
{
	if (errorCode == FMOD_OK)
	{
		//result = system->playSound(sound, null, false, out channel);
		errorCode = system->playSound(handle, nullptr, true, &instance);
		if (errorCode != FMOD_OK)
		{
			PostFMODError(errorCode);
		}
		else
		{
			errorCode = instance->setVolume(0.701f);
			if (errorCode != FMOD_OK)
			{
				PostFMODError(errorCode);
			}
			else
			{
				errorCode = instance->setPaused(false);
				if (errorCode != FMOD_OK)
				{
					PostFMODError(errorCode);
				}
			}
		}
	}
}

void AudioStream::Stop()
{
	if (errorCode == FMOD_OK)
		instance->stop();
}

void AudioStream::Update(float dt)
{
	if (audioTree.root && audioTree.root->done)
		Stop();
}

