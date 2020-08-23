//
//  audio_examples.cpp
//  audiosample
//
//  Created by Mike Gonzales on 8/21/20.
//


#include "entry_point.h"
#include "audio_writers.h"
#include <math.h>

static AudioStream * memStream = nullptr;

const float kBeatsPerMinute = 80.0f;
const float kBeatsPerSecond = kBeatsPerMinute / 60.0f;
const float kBeatTime = 1.0f / kBeatsPerSecond;
const float kAHertzValue = 440.0f;

enum NoteStep {kNoteC = -9, kNoteD = -7, kNoteE = -5, kNoteF = -4, kNoteG = -2, kNoteA = 0, kNoteBb = 1, kNoteC2 = 3, kRest = 1024};

struct NoteValue
{
	NoteStep note;
	float duration;
};

/*
C - .75
C - .25

D - 1     	F	(F, A, C)
C - 1
F - 1

E - 2    	C7	(C, G, Bb)
C - .75
C - .25

D - 1
C - 1
G - 1

F - 2		F	(F, A, C)
C - .75
C - .25

C^ - 1
A - 1
F - 1

E - 1		Bb	(Bb, D, F)
D - 1
Bb - .75
Bb - .25

A - 1		C	(C, E, G)
F - 1
G - 1		C7 	(C, E, Bb)

F - 2		F
*/

static NoteValue melody[] =
{
	{ kRest, 2.0f },	// Measure 0
	{ kNoteC, 0.75f },
	{ kNoteC, 0.25f },
	
	{ kNoteD, 1.0f },	// Measure 1
	{ kNoteC, 1.0f },
	{ kNoteF, 1.0f },
	
	{ kNoteE, 2.0f },	// Measure 2
	{ kNoteC, 0.75f },
	{ kNoteC, 0.25f },
	
	{ kNoteD, 1.0f },	// Measure 3
	{ kNoteC, 1.0f },
	{ kNoteG, 1.0f },
	
	{ kNoteF, 2.0f },	// Measure 4
	{ kNoteC, 0.75f },
	{ kNoteC, 0.25 },
	
	{ kNoteC2, 1.0f },	// Measure 5
	{ kNoteA, 1.0f },
	{ kNoteF, 1.0f },
	
	{ kNoteE, 1.0f },	// Measure 6
	{ kNoteD, 1.0f },
	{ kNoteBb, 0.75f },
	{ kNoteBb, 0.25f },
	
	{ kNoteA, 1.0f },	// Measure 7
	{ kNoteF, 1.0f },
	{ kNoteG, 1.0f },
	
	{ kNoteF, 2.0f },	// Measure 8
};


float NoteStepToHertz(NoteStep steps)
{
	const float stepValue = pow(2.0f, -12.0f);
	float hertz = kAHertzValue * pow (stepValue, steps);
	return hertz;
}

AudioWriter::Base * GenerateNoteWriter(NoteValue note, bool downBeat)
{
	if (note.note == kRest)
		return nullptr;
	
	AudioWriter::Base * tone = new AudioWriter::SineTone();
	auto * param = new AudioWriter::ParamOverride();
	
	param->gain = downBeat ? 1.0f : 0.70f;
	param->pitch = NoteStepToHertz(note.note);
	param->duration = kBeatTime * note.duration * 1.1f;
	
	param->child = tone;
	return param;
}


void AppWrapper::StartLogic()
{
	auto note = GenerateNoteWriter({kNoteA, 2.0f}, false);
	auto treeNote = AudioWriter::Tree(note);
	
	auto * sin = new AudioWriter::SineTone();
	auto * root = new AudioWriter::ParamOverride();
	root->child = sin;
	root->gain = 1.0f;
	root->pitch = 440.0f;
	root->delay = 0.75f;
	root->duration = 3.0f;
	
	memStream = m_audio.CreateAudioStream(root);
	memStream->Start();
}


void AppWrapper::UpdateLogic(float dt)
{
	memStream->Update(dt);
	m_audio.UpdateAudioStream(dt);
}


