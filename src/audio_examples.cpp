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
static AudioStream * memStream2 = nullptr;

const float kBeatsPerMinute = 80.0f;
const float kBeatsPerSecond = kBeatsPerMinute / 60.0f;
const float kBeatTime = 1.0f / kBeatsPerSecond;
const float kAHertzValue = 440.0f;
const int32_t kBeatsPerMeasure = 3;
const int32_t kLeadInBeat = 0;

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
	kNoteD2 = 5,
	kNoteE2 = 7,
	kNoteF2 = 8,
	kNoteG2 = 10,
	kNoteA3 = 12,
	
	kRest = 1024
};

struct NoteValue
{
	float startTime;
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
	{ 0.0f, kRest, 2.0f },	// Measure 0
	{ 2.0f, kNoteC1, 0.75f },
	{ 2.75f, kNoteC1, 0.25f },
	
	{ 3.0f, kNoteD1, 1.0f },	// Measure 1
	{ 4.0f, kNoteC1, 1.0f },
	{ 5.0f, kNoteF1, 1.0f },
	
	{ 6.0f, kNoteE1, 2.0f },	// Measure 2
	{ 8.0f, kNoteC1, 0.75f },
	{ 8.75f, kNoteC1, 0.25f },
	
	{ 9.0f, kNoteD1, 1.0f },	// Measure 3
	{ 10.0f, kNoteC1, 1.0f },
	{ 11.0f, kNoteG1, 1.0f },
	
	{ 12.0f, kNoteF1, 2.0f },	// Measure 4
	{ 14.0f, kNoteC1, 0.75f },
	{ 14.75f, kNoteC1, 0.25 },
	
	{ 15.0f, kNoteC2, 1.0f },	// Measure 5
	{ 16.0f, kNoteA2, 1.0f },
	{ 17.0f, kNoteF1, 1.0f },
	
	{ 18.0f, kNoteE1, 1.0f },	// Measure 6
	{ 19.0f, kNoteD1, 1.0f },
	{ 20.0f, kNoteBb2, 0.75f },
	{ 20.75f, kNoteBb2, 0.25f },
	
	{ 21.0f, kNoteA2, 1.0f },	// Measure 7
	{ 22.0f, kNoteF1, 1.0f },
	{ 23.0f, kNoteG1, 1.0f },
	
	{ 24.0f, kNoteF1, 2.0f },	// Measure 8
};

static NoteValue harmony[] =
{
	{ 3.0f, kNoteA1, 2.75f },
	{ 3.0f, kNoteC1, 2.75f },
	{ 3.0f, kNoteF1, 2.75f },
	
	{ 6.0f, kNoteBb1, 2.75f },
	{ 6.0f, kNoteC1, 2.75f },
	{ 6.0f, kNoteG1, 2.75f },
	
	{ 9.0f, kNoteBb1, 2.75f },
	{ 9.0f, kNoteC1, 2.75f },
	{ 9.0f, kNoteG1, 2.75f },
	
	{ 12.0f, kNoteA1, 2.75f },
	{ 12.0f, kNoteC1, 2.75f },
	{ 12.0f, kNoteF1, 2.75f },
	
	{ 15.0f, kNoteA1, 2.75f },
	{ 15.0f, kNoteC1, 2.75f },
	{ 15.0f, kNoteF1, 2.75f },
	
	{ 18.0f, kNoteBb1, 2.75f },
	{ 18.0f, kNoteD1, 2.75f },
	{ 18.0f, kNoteF1, 2.75f },
	
	{ 21.0f, kNoteC1, 1.5f },
	{ 21.0f, kNoteE1, 1.5f },
	{ 21.0f, kNoteG1, 1.5f },
	
	{ 23.0f, kNoteC1, 1.0f },
	{ 23.0f, kNoteE1, 1.0f },
	{ 23.0f, kNoteBb2, 1.0f},
	
	{ 24.0f, kNoteF1, 2.75f },
	{ 24.0f, kNoteA2, 2.75f },
	{ 24.0f, kNoteC2, 2.75f },
};


float NoteStepToHertz(NoteStep steps)
{
	const float stepValue = powf(2.0f, 1.0f/12.0f);
	const float toSteps = float(steps);
	float hertz = kAHertzValue * powf(stepValue, toSteps);
	return hertz;
}

AudioWriter::Base * GenerateNoteComposite(NoteValue note, AudioWriter::WaveFn wave, float baseGain)
{
	float basePitch = NoteStepToHertz(note.note);
	
	AudioWriter::Tone * fund = new AudioWriter::Tone(wave);
	fund->gain = baseGain * 0.5f;
	fund->pitch = basePitch;
	fund->duration = kBeatTime * note.duration * 1.1f;
	
	AudioWriter::Tone * second = new AudioWriter::Tone(wave);
	second->gain = baseGain * 0.25f;
	second->pitch = 2.0f * basePitch;
	second->duration = kBeatTime * note.duration * 1.1f;
	
	AudioWriter::Tone * tert = new AudioWriter::Tone(wave);
	tert->gain = baseGain * 0.125f;
	tert->pitch = 4.0f * basePitch;
	tert->duration = kBeatTime * note.duration * 1.1f;
	
	AudioWriter::Tone * quart = new AudioWriter::Tone(wave);
	quart->gain = baseGain * 0.0625f;
	quart->pitch = 8.0f * basePitch;
	quart->duration = kBeatTime * note.duration * 1.1f;
	
	AudioWriter::Composite * comp = new AudioWriter::Composite();
	comp->PushChild(fund);
	comp->PushChild(second);
	comp->PushChild(tert);
	comp->PushChild(quart);
	
	return comp;
}

AudioWriter::Base * GenerateNoteWriter(NoteValue note, AudioWriter::WaveFn wave, float baseGain)
{
	if (note.note == kRest)
		return nullptr;
	
	auto * tone = GenerateNoteComposite(note, wave, baseGain);
	
	auto * env = new AudioWriter::Envelope(new AudioWriter::AttackSustainDecayEnvelope);
	env->child = tone;
	
	auto * param = new AudioWriter::ParamOverride();
	param->gain = 0.701f;
	param->pitch = NoteStepToHertz(note.note);
	param->duration = kBeatTime * note.duration * 1.1f;
	
	param->child = env;
	return param;
}

AudioWriter::Base * SimpleTest()
{
	auto note = GenerateNoteWriter({0.0f, kNoteA2, 2.0f}, AudioWriter::SineWave, 0.35f);
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
	
	auto * note0 = GenerateNoteWriter({0.25f, kNoteA2, 3.0f}, AudioWriter::SineWave, 0.35f);
	root->PushChild(note0, 0.0f);
	
	auto * note1 = GenerateNoteWriter({2.5f, kNoteBb2, 3.0f}, AudioWriter::SineWave, 0.35f);
	root->PushChild(note1, kBeatTime*3.5f);
	
	return root;
}

AudioWriter::Base * ChordTest()
{
	auto * root = new AudioWriter::Sequencer();
	
	auto * note0 = GenerateNoteWriter({0.0f, kNoteA2, 3.0f}, AudioWriter::SineWave, 0.35f);
	root->PushChild(note0, 0.8f);
	
	auto * note1 = GenerateNoteWriter({0.0f, kNoteC2, 3.0f}, AudioWriter::SineWave, 0.35f);
	root->PushChild(note1, 0.0f);
	
	auto * note2 = GenerateNoteWriter({0.0f, kNoteE2, 3.0f}, AudioWriter::SineWave, 0.35f);
	root->PushChild(note2, 0.0f);

	return root;
}

AudioWriter::Base * ScaleTest()
{
	auto * root = new AudioWriter::Sequencer();
	
	auto * note0 = GenerateNoteWriter({0.0f, kNoteA1, 1.0f}, AudioWriter::SineWave, 0.35f);
	root->PushChild(note0, kBeatTime);
	
	auto * note1 = GenerateNoteWriter({1.0f, kNoteBb1, 1.0f}, AudioWriter::SineWave, 0.35f);
	root->PushChild(note1, kBeatTime);
	
	auto * note2 = GenerateNoteWriter({2.0f, kNoteC1, 1.0f}, AudioWriter::SineWave, 0.35f);
	root->PushChild(note2, kBeatTime);
	
	auto * note3 = GenerateNoteWriter({3.0f, kNoteD1, 1.0f}, AudioWriter::SineWave, 0.35f);
	root->PushChild(note3, kBeatTime);
	
	auto * note4 = GenerateNoteWriter({4.0f, kNoteE1, 1.0f}, AudioWriter::SineWave, 0.35f);
	root->PushChild(note4, kBeatTime);
	
	auto * note5 = GenerateNoteWriter({5.0f, kNoteF1, 1.0f}, AudioWriter::SineWave, 0.35f);
	root->PushChild(note5, kBeatTime);
	
	auto * note6 = GenerateNoteWriter({6.0f, kNoteG1, 1.0f}, AudioWriter::SineWave, 0.35f);
	root->PushChild(note6, kBeatTime);
	
	auto * note7 = GenerateNoteWriter({7.0f, kNoteA2, 1.0f}, AudioWriter::SineWave, 0.35f);
	root->PushChild(note7, kBeatTime);
	
	auto * note8 = GenerateNoteWriter({8.0f, kNoteA1, 1.0f}, AudioWriter::SineWave, 0.35f);
	root->PushChild(note8, kBeatTime);
	
	return root;
}

AudioWriter::Base * SequenceGenerator(NoteValue *notes, int32_t len, AudioWriter::WaveFn wave, float baseGain)
{
	auto * root = new AudioWriter::Sequencer();
	for (int32_t ord = 0; ord < len; ord++)
	{
		auto & noteData = notes[ord];
		
		//Note this doens't handle successive rests
		if (noteData.note == kRest)
			continue;
		
		auto * noteInst = GenerateNoteWriter(noteData, wave, baseGain);
		
		float noteDelay = 0.0f;
		if (ord > 0)
			noteDelay = notes[ord].startTime - notes[ord - 1].startTime;
		else
			noteDelay = notes[ord].startTime;
		
		root->PushChild(noteInst,noteDelay);
	}
	
	return root;
}

AudioWriter::Base * MelodyTest()
{
	return SequenceGenerator(melody, sizeof(melody) / sizeof(melody[0]), AudioWriter::SineWave, 0.35f);
}

AudioWriter::Base * HarmonyTest()
{
	return SequenceGenerator(harmony, sizeof(harmony)/sizeof(harmony[0]), AudioWriter::SawWave, 0.035f);
}

void AppWrapper::StartLogic()
{
	//auto * root = SimpleTest();
	//auto * root = SequenceTest();
	//auto * root = ScaleTest();
	//auto * root = ChordTest();
	auto * root = MelodyTest();
	
	memStream = m_audio.CreateAudioStream(root);
	memStream->Start();
	
	auto * root2 = HarmonyTest();
	
	memStream2 = m_audio.CreateAudioStream(root2);
	memStream2->Start();
}

void AppWrapper::UpdateLogic(float dt)
{
	memStream->Update(dt);
	memStream2->Update(dt);
	m_audio.UpdateAudioStream(dt);
}


