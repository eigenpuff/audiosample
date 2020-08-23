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


struct Sequencer : Base
{
	std::vector<Base*> children;
	std::vector<float> delays;
	std::vector<bool>  simulWithPrevious;

	bool Init() override;
	bool Write(float *buffer, int32_t numFrames) override;
	
	~Sequencer() override;
};

struct Composite : Base
{
	std::vector<Base*> children;
	
	bool Init() override;
	bool Write(float *buffer, int32_t numFrames) override;
	
	~Composite() override;
};

using EnvelopeFn = float(*) (float time, void *userData);

struct Envelope : Base
{
	Base * child;
	Envelope(EnvelopeFn envFn) : envelope(envFn) {}

	bool Init() override;
	bool Write(float *buffer, int32_t numFrames) override;
	
	~Envelope() override;
	EnvelopeFn envelope;
};

using WaveFn = float (*) (float time, float pitch, float phase);
	
struct Tone: Base
{
	Tone(WaveFn wv) : wave(wv) {}
	bool Init() override;
	bool Write (float * buffer, int32_t numFrames) override;
	
	WaveFn wave = nullptr;
};

float SineWave(float time, float pitch, float phase);
float SawWave(float time, float pitch, float phase);
float SquareWave(float time, float pitch, float phase);
float TriangleWave(float time, float pitch, float phase);

using EnvelopeFn = float(*) (float time, void *userData);

float SineEnvelope(float time, void * userData);
float SplineEnvelop(float time, void * userData);
float AttackDecayEnvelope(float time, void * userData);

}

#endif /* audio_writers_h */
