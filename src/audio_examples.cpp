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
const int32_t kBeatsPerMeasure = 3;
const int32_t kLeadInBeat = 1;

enum NoteStep
{
	kNoteA1 = -12,
	kNoteBb1 = -11,
	kNoteC1 = -9,
	kNoteD1 = -7,
	kNoteE1 = -5,
	kNoteF1 = -4,
	kNoteG1 = -2,
	kNoteA2 = 0,
	kNoteBb2 = 1,
	kNoteC2 = 3,
	
	kRest = 1024
};

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
	{ kNoteC1, 0.75f },
	{ kNoteC1, 0.25f },
	
	{ kNoteD1, 1.0f },	// Measure 1
	{ kNoteC1, 1.0f },
	{ kNoteF1, 1.0f },
	
	{ kNoteE1, 2.0f },	// Measure 2
	{ kNoteC1, 0.75f },
	{ kNoteC1, 0.25f },
	
	{ kNoteD1, 1.0f },	// Measure 3
	{ kNoteC1, 1.0f },
	{ kNoteG1, 1.0f },
	
	{ kNoteF1, 2.0f },	// Measure 4
	{ kNoteC1, 0.75f },
	{ kNoteC1, 0.25 },
	
	{ kNoteC2, 1.0f },	// Measure 5
	{ kNoteA2, 1.0f },
	{ kNoteF1, 1.0f },
	
	{ kNoteE1, 1.0f },	// Measure 6
	{ kNoteD1, 1.0f },
	{ kNoteBb2, 0.75f },
	{ kNoteBb2, 0.25f },
	
	{ kNoteA2, 1.0f },	// Measure 7
	{ kNoteF1, 1.0f },
	{ kNoteG1, 1.0f },
	
	{ kNoteF1, 2.0f },	// Measure 8
};


float NoteStepToHertz(NoteStep steps)
{
	const float stepValue = powf(2.0f, 1.0f/12.0f);
	const float toSteps = float(steps);
	float hertz = kAHertzValue * powf(stepValue, toSteps);
	return hertz;
}

AudioWriter::Base * GenerateNoteWriter(NoteValue note, bool downBeat)
{
	if (note.note == kRest)
		return nullptr;
	
	AudioWriter::Base * tone = new AudioWriter::Tone(AudioWriter::SineWave);
	auto * param = new AudioWriter::ParamOverride();
	
	auto * env = new AudioWriter::Envelope(new AudioWriter::AttackSustainDecayEnvelope);
	env->child = tone;
	
	param->gain = downBeat ? 1.0f : 0.70f;
	param->pitch = NoteStepToHertz(note.note);
	param->duration = kBeatTime * note.duration * 1.1f;
	
	param->child = env;
	return param;
}

AudioWriter::Base * SimpleTest()
{
	auto note = GenerateNoteWriter({kNoteA2, 2.0f}, false);
	auto treeNote = AudioWriter::Tree(note);
	
	auto * sin = new AudioWriter::Tone(AudioWriter::SawWave);
	
	auto * env = new AudioWriter::Envelope(new AudioWriter::AttackSustainDecayEnvelope);
	env->child = sin;
	
	auto * root = new AudioWriter::ParamOverride();
	
	root->child = env;
	root->gain = 0.050f;
	root->pitch = 440.0f;
	root->delay = 0.75f;
	root->duration = 1.0f;
	return root;
}

AudioWriter::Base * SequenceTest()
{
	auto * root = new AudioWriter::Sequencer();
	
	auto * note0 = GenerateNoteWriter({kNoteA2, 3.0f}, false);
	root->PushChild(note0, 0.0f);
	
	auto * note1 = GenerateNoteWriter({kNoteBb2, 3.0f}, false);
	root->PushChild(note1, kBeatTime*3.5f);
	
	return root;
}

AudioWriter::Base * ScaleTest()
{
	auto * root = new AudioWriter::Sequencer();
	
	auto * note0 = GenerateNoteWriter({kNoteA1, 1.0f}, false);
	root->PushChild(note0, kBeatTime);
	
	auto * note1 = GenerateNoteWriter({kNoteBb1, 1.0f}, false);
	root->PushChild(note1, kBeatTime);
	
	auto * note2 = GenerateNoteWriter({kNoteC1, 1.0f}, false);
	root->PushChild(note2, kBeatTime);
	
	auto * note3 = GenerateNoteWriter({kNoteD1, 1.0f}, false);
	root->PushChild(note3, kBeatTime);
	
	auto * note4 = GenerateNoteWriter({kNoteE1, 1.0f}, false);
	root->PushChild(note4, kBeatTime);
	
	auto * note5 = GenerateNoteWriter({kNoteF1, 1.0f}, false);
	root->PushChild(note5, kBeatTime);
	
	auto * note6 = GenerateNoteWriter({kNoteG1, 1.0f}, false);
	root->PushChild(note6, kBeatTime);
	
	auto * note7 = GenerateNoteWriter({kNoteA2, 1.0f}, false);
	root->PushChild(note7, kBeatTime);
	
	auto * note8 = GenerateNoteWriter({kNoteA1, 1.0f}, false);
	root->PushChild(note8, kBeatTime);
	
	return root;
}

AudioWriter::Base * MelodyTest()
{
	auto * root = new AudioWriter::Sequencer();
	int32_t arrSize = sizeof(melody) / sizeof(melody[0]);
	for (int32_t ord = 0; ord < arrSize; ord++)
	{
		auto & noteData = melody[ord];
		auto * noteInst = GenerateNoteWriter(noteData, (ord + kLeadInBeat) % kBeatsPerMeasure == 0);
		
		float noteDelay = 0.0f;
		if (ord > 0)
			noteDelay = melody[ord - 1].duration;
		
		root->PushChild(noteInst,noteDelay);
	}
	
	return root;
}

void AppWrapper::StartLogic()
{
	//auto * root = SimpleTest();
	//auto * root = SequenceTest();
	//auto * root = ScaleTest();
	auto * root = MelodyTest();
	memStream = m_audio.CreateAudioStream(root);
	memStream->Start();
}


void AppWrapper::UpdateLogic(float dt)
{
	memStream->Update(dt);
	m_audio.UpdateAudioStream(dt);
}


