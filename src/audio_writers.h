//
//  audio_writers.h
//  audiosample
//
//  Created by Mike Gonzales on 8/21/20.
//

#ifndef audio_writers_h
#define audio_writers_h

#include <stdint.h>
#include <vector>
#include <deque>

const float kTau = 6.28318530718f;

namespace AudioWriter
{

struct Base
{
	float pitch = 1.0f;
	float gain = 1.0f;
	float phase = 0.0f;
	float time = 0.0f;
	float duration = 0.0f;
	
	bool  done = false;
	bool  inited = false;
	
	// does initialization, returns whether to abort
	virtual bool Init () = 0;
	
	// writes to a number of frames to a buffer, returns whether to be done
	// Buffer is assumed to be zero filled or otherwise initialized before
	// it arrives to this write.
	virtual bool Write (float * buffer, int32_t numFrames) = 0;
	virtual ~Base() {}
};

//Tree is meant to be used by value to hold on to a dynamically
// allocated root pointer.
struct Tree
{
	Base * root;
	
	Tree(Base * base) :
		root(base)
	{}
	
	bool Init()
	{
		if (root)
			return root->Init();
		return true;
	}
	
	bool Write(float * buffer, int32_t numFrames)
	{
		if (root)
			return root->Write(buffer, numFrames);
		return true;
	}
	
	~Tree()
	{
		delete root;
	}
};

struct ParamOverride : Base
{
	Base * child = nullptr;
	float delay = 0.0f;
	
	void CopyParams();
	
	bool Init() override;
	bool Write(float * buffer, int32_t numFrames) override;
	
	~ParamOverride() { delete child; }
};


//sequencer plays a bunch of sounds in sequence, with the
// delay associated with each sound telling us when to start
// each sound after the previous
struct Sequencer : Base
{
	std::vector<Base*> children;
	std::vector<float> delays;
	
private:
	//scratch variables, calculated upon play
	float * scratchSpace = nullptr;
	size_t scratchLen;
	
	std::vector<float> timeline;
	int32_t timelineIndex = 0;
	
	int32_t queueIndex = 0;
	std::deque<Base*> playQueue;
	std::deque<float> timeQueue;

public:
	bool Init() override;
	bool Write(float *buffer, int32_t numFrames) override;
	
	void CalcTotalTime();
	void PushChild(Base * child, float cumulDelay);
	
	~Sequencer() override;
};

struct Composite : Base
{
	std::vector<Base*> children;
	
	bool Init() override;
	bool Write(float *buffer, int32_t numFrames) override;
	
	~Composite() override;
};

struct EnvelopeBase
{
	virtual float operator () (float t) = 0;
	virtual ~EnvelopeBase() {}
};

struct Envelope : Base
{
	Base * child;
	EnvelopeBase * envelope;
	
	Envelope(EnvelopeBase * envFn) : envelope(envFn) {}

	bool Init() override;
	bool Write(float *buffer, int32_t numFrames) override;
	
	~Envelope() override;
};

using WaveFn = float (*) (float time, float pitch, float phase);
	
struct Tone: Base
{
	Tone(WaveFn wv) : wave(wv) {}
	bool Init() override;
	bool Write (float * buffer, int32_t numFrames) override;
	
	WaveFn wave = nullptr;
};

// for Wave Generators
// time is real time in seconds;
// pitch is in hertz;
// phase is a fraction of one wavecycle to offset by, -1 to 1

float SineWave(float time, float pitch, float phase);
float SawWave(float time, float pitch, float phase);
float SquareWave(float time, float pitch, float phase);
float TriangleWave(float time, float pitch, float phase);

// for envelopes
// t is 0 - 1 of length over the whole playback

struct SineEnvelope : EnvelopeBase
{
	float operator () (float t) override;
};

struct SplineEnvelope : EnvelopeBase
{
	std::vector<float> cp;
	float operator () (float t) override;
};

struct AttackDecayEnvelope : EnvelopeBase
{
	float operator () (float t) override;
};

struct AttackSustainDecayEnvelope : EnvelopeBase
{
	float operator () (float t) override;
};
}

#endif /* audio_writers_h */
