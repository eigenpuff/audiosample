//
//  fft.h
//  samples_osx
//
//  Created by Mike Gonzales on 8/15/14.
//
//

#ifndef _PLUGIN_FFT_H_
#define _PLUGIN_FFT_H_

#include "support/entropy.h"

//#include "mathutil.h"

const int kKB = 1024;

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
inline bool isPowerOfTwo(int i)
{
	return (i & (i-1)) == 0;
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
struct Complex
{
	float r, i;
	
	Complex(){}
	Complex(float _r, float _i = 0):r(_r), i(_i) {}
	Complex(const Complex &c):	r(c.r), i(c.i) {}
	
	Complex & operator= (const Complex& c){r = c.r; i = c.i; return *this;}
	
	Complex operator + (const Complex & c) const {return Complex(r + c.r, i + c.i);}
	Complex operator - (const Complex & c) const {return Complex(r - c.r, i - c.i);}
	Complex operator * (const Complex & c) const {return Complex(r*c.r - i*c.i, r*c.i + i*c.r);}
	Complex operator / (const Complex & c) const {return (*this) * (~c)/(+c);}
	
	Complex operator / (float f) const {return Complex(r/f, i/f);}
	Complex operator * (float f) const {return Complex(r*f, i*f);}
	
	Complex operator ~ () const {return Complex(r, -i);}
	float operator + () const {return entropy::Sqrt(r*r + i*i);}
	Complex operator - () const {return Complex(-r, -i);}

	Complex &operator += (const Complex &c) { r += c.r; i += c.i; return *this;}
	Complex &operator -= (const Complex &c) { r -= c.r; i -= c.i; return *this;}
	Complex &operator *= (const Complex &c) {*this = *this*c; return *this;}
	Complex &operator /= (const Complex &c) { *this = *this/c; return *this;}
	
	Complex &operator / (float f)  {r/=r; i/=f; return *this;}
	Complex &operator * (float f)  {r*=f, i*=f; return *this;}
};

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
inline Complex expi(float i)
{
	return Complex( entropy::Cos(i), entropy::Sin(i));
}

const Complex I(0,1);

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
class Envelope
{
public:
	virtual float operator[] (int sample) = 0;
	virtual ~Envelope() {};
};

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
class SampleEnvelope : public Envelope
{
	bool ownMemory;
	int numSamples;
	float *samples;
	
public:
	float operator[] (int sample)
	{
		return samples[sample];
	}
	
	SampleEnvelope(int len)
	{
		ownMemory 	= true;
		samples 	= (float*)entropy::Memory_Alloc(sizeof(float)*len);
		numSamples 	= len;
	}
	
	SampleEnvelope(int len, float *mem)
	{
		ownMemory	= false;
		samples 	= mem;
		numSamples	= len;
	}
	
	~SampleEnvelope()
	{
		if(ownMemory)
			entropy::Memory_Free(samples);
	}
	
	int getLen() const {return numSamples;}
	float* buffer() {return samples;}
	const float * buffer() const {return samples;}
};

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
class Hamming : public Envelope
{
public:
	int len;
	
	Hamming(int len);
	~Hamming(){}
	float operator[] (int sample);
	
	static float Sample(float t);
};

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
class Hamming4k: public Envelope
{
public:
	float samples[4*kKB];
	
	Hamming4k();
	~Hamming4k(){}
	
	float operator[](int index)
	{
		return samples[index];
	}
};

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
class Hamming16k: public Envelope
{
public:
	float samples[16*kKB];
	
	Hamming16k();
	~Hamming16k(){}
	
	float operator[](int index)
	{
		return samples[index];
	}
};

class Spectra;

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
class RawSpectra
{
	bool ownMemory;
	int numSpectra;
	Complex * spectra;
public:
	
	RawSpectra();
	RawSpectra(int len);
	RawSpectra(const Complex * samples, int len);
	
	//if for preallocating memory, pass false for copy
	RawSpectra(Complex * samples, int len, bool copy);
	~RawSpectra();
	
	void copy(const Complex * samples, int len);
	
	int len() const;
	
	Complex * buffer();
	const Complex * buffer() const;
	
	operator Complex * () {return spectra;}
	operator const Complex* () const {return spectra;}
	
	Complex & operator[](int index);
	const Complex & operator[](int index) const ;
	
	//also for supplying preallocated memory
	void setMemory(Complex * mem, int len);
	
	//TODO:
	//To add RawSpectra to this to get a combination wave
	void combine(const RawSpectra * other);
	//Convert processed Spectra back to Raw Spectra for ifft
	void unprocess(const Spectra * processed);
	
	bool ownsMemory() const {return ownMemory;}
};



/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
class Spectra
{
	bool ownMemory;
	int numSpectra;
	Complex * spectra;
	
	int peakIndex;
	float peakValue;
public:
	
	Spectra();
	Spectra(int len);
	
	//only for pre-allocating memory
	Spectra(Complex * spectra, int len);
	
	Spectra(const RawSpectra * raw);
	
	~Spectra();
	
	int len() const;
	
	void process(const RawSpectra * raw);
	
	operator Complex * () {return spectra;}
	operator const Complex* () const {return spectra;}
	
	Complex & operator[](int index);
	const Complex & operator[](int index) const ;
	
	Complex * buffer();
	const Complex * buffer()const;
	
	//also for supplying preallocated memory
	void setMemory(Complex * mem, int len);
	
	Spectra * preprocess(float time);

	//TODO:
	//Combine processed Spectra together to get a combination wave
	void combine(const Spectra * other);
	bool ownsMemory() const {return ownMemory;}
};

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
class ChromaticSpectra
{
	
public:
	int maxPeak;
	float peakValue;

	enum{kOctaves = 7, kNumNotes = kOctaves*12, kBands = 8};
	float bands[kBands];
	float spectra[kNumNotes];
		
	ChromaticSpectra();
	
	void strictProcess(const Spectra * raw, float time);
	void bandProcess(const Spectra * raw, float time);
	void deltaProcess(const Spectra * n1, const Spectra * n0, float time);

	void strictProcess(const Spectra * left, const Spectra * right, float time);
	void bandProcess(const Spectra * left, const Spectra * right, float time);
	
	int				getPeakIndex() 	const	{return maxPeak;}
	float *			getSpectra() 			{return spectra;}
	const float *	getSpectra() 	const	{return spectra;}
	int				getNumSpectra() const	{return getNumFrequencies();}
	
	float *			getBands()				{return bands;}
	const float *	getBands()		const	{return bands;}
	int				getNumBands()	const	{return kBands;}

	//returns the values at a given note index
	float & operator[](int index);
	const float & operator[](int index) const ;
	
	//information how to map each index to an absolute frequency
	static int 		getNumFrequencies()			{return kNumNotes;}
	static float* 	getFrequencies();
	//set the calibration frequency for "concert A" that determines all the
	// other frequencies (set to 440 by default, but hippy-dippy new age music
	// sometimes uses 432)
	static void		setA5Frequency(float hz);
	static float	getA5Frequency();
	
	//returns the frequency for a note index
	float  operator()(int index) const		{return getFrequencies()[index];}
	
protected:

	static float	A5Frequency;
	static bool		calcBuckets;
	static float	freqBuckets[kNumNotes];
	static void		createFreqBuckets();
};

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
class BeatSpectra
{
public:
	BeatSpectra();
	int counter;
	enum{ kHistory = 32};
	ChromaticSpectra history[kHistory];
	ChromaticSpectra beat;
	ChromaticSpectra energy;
	ChromaticSpectra variance;


	void ProcessSpectra(const ChromaticSpectra &append);
	void ProcessBands(const ChromaticSpectra & append);
	void Process(const ChromaticSpectra & append);

	float SpectraAverageEnergy(int band);
	float SpectraVariance(int band, float average);
	float BandAverageEnergy(int band);
	float BandVariance(int band, float average);
	bool ready() {return counter >= 0;}
};


/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
class Signal
{
	bool ownMemory;
	int numSamples;
	Complex * samples;
public:
	
	Signal();
	Signal(int len);
	Signal(const Complex * spectra, int len);
	Signal(Complex * spectra, int len, bool copy = true);
	
	~Signal();
	
	int len() const;
	
	operator Complex * () {return samples;}
	operator const Complex* () const {return samples;}
	
	Complex & operator[](int index);
	const Complex & operator[](int index) const ;
		
	Complex * buffer();
	const Complex * buffer()const;
	
	//for preallocation
	void setMemory(Complex * mem, int len);
};


	/*
---	** -----------------------------------------------------------------------------------------------------------
	*/

void	initFFTSystem();
void	fftSystemTest();

//for any power of two
void	fft	(const float * samples, int len, RawSpectra *spectraOut, Envelope * envelope = NULL);
void	fft	(const Complex * samples, int len, RawSpectra * spectraOut, Envelope * envelope = NULL);
void	ifft(const RawSpectra * spectra, Signal * signalOut);

//for hardcoded 4k and 16k most common use cases
void	fft16k	(const float samples[16*kKB], RawSpectra *spectraOut, Envelope *envelope=NULL);
void	ifft16k	(const RawSpectra * spectra, Signal * signalOut);
void	fft4k	(const float samples[4*kKB], RawSpectra *spectraOut, Envelope *envelope=NULL);
void	ifft4k	(const RawSpectra * spectra, Signal *signalOut);

//simplest implementation with recursion, for testing
void	fft_simple	(const float * samples, int len, RawSpectra * spectraOut, Envelope * envelope = NULL);
void	fft_simple	(const Complex * samples, int len, RawSpectra * spectraOut,  Envelope * envelope = NULL);
void	ifft_simple	(const RawSpectra *samples, Signal * signalOut);
//void	fft_chromatic(const float *samples, int len, float time, ChromaticSpectra* spectraOut, Envelope * envelope = NULL);

//convert signal back to raw samples
void	ifft_finalize(const Signal *signal, float * samplesOut);

//stereo routines that leverage above
void	fft_stereo	(const float * left, const float *right, int len, RawSpectra* out, Envelope * envelope = NULL);
void	fft_stereo	(const float * left, const float *right, int len, RawSpectra* leftOut, RawSpectra* rightOut, Envelope * envelope = NULL);
void	fft_stereo	(const float * interleaved, int numFrames, RawSpectra *out, Envelope * envelope = NULL);
void	fft_stereo	(const float * interleaved, int numFrames, RawSpectra * leftOut, RawSpectra * rightOut, Envelope * env = NULL);
void	ifft_stereo	(const RawSpectra* left, Signal * out);
void	ifft_stereo	(const RawSpectra * left, const RawSpectra * right, Signal * out);

void	fft16k_stereo(const float left[16*kKB], const float right[16*kKB], RawSpectra * spectraOut, Envelope * envelope = NULL);
void	fft16k_stereo(const float left[16*kKB], const float right[16*kKB], RawSpectra * leftOut, RawSpectra* rightOut, Envelope * envelope = NULL);
void	fft16k_stereo(const float interleaved[2*16*kKB], RawSpectra * spectraOut, Envelope * envelope = NULL);
void	fft16k_stereo(const float interleaved[2*16*kKB], RawSpectra * leftOut, RawSpectra* rightOut, Envelope * envelope = NULL);

void	fft4k_stereo(const float left[4*kKB], const float right[4*kKB], RawSpectra *spectraOut, Envelope * envelope = NULL);
void	fft4k_stereo(const float left[4*kKB], const float right[4*kKB], RawSpectra *leftOut, RawSpectra * rightOut, Envelope * envelope = NULL);
void	fft4k_stereo(const float interleaved[2*4*kKB], RawSpectra *spectraOut, Envelope * envelope = NULL);
void	fft4k_stereo(const float interleaved[2*4*kKB], RawSpectra *leftOut, RawSpectra * rightOut, Envelope * envelope = NULL);

void	fft_stereo_unpack(RawSpectra * interleaved, RawSpectra * leftOut, RawSpectra * rightOut);

void	ifft16k_stereo	(const RawSpectra * spectra, Signal * signalOut);
void	ifft16k_stereo	(const RawSpectra * left, const RawSpectra * right, Signal * signalOut);
void	ifft4k_stereo	(const RawSpectra * spectra, Signal *signalOut);
void	ifft4k_stereo	(const RawSpectra * left, const RawSpectra* right, Signal *signalOut);

//convert stereo signal back to raw samples
void	ifft_stereofinalize(const Signal * signal, float *left, float *right);
void	ifft_stereofinalize(const Signal * signal, float *interleaved);

/*//------------------------------------------------------------------------
//Declared below, but for small ffts to be done on the stack
*///------------------------------------------------------------------------
/*
template <int size>	void	fft_stack(const float samples[size], RawSpectra *spectra);
template <int size>	void	ifft_stack(const RawSpectra * spectra, Signal *signalOut);
*///------------------------------------------------------------------------

	/*
---	** -----------------------------------------------------------------------------------------------------------
	*/

//Implementation stuff, you can ignore from here down

inline Complex &		RawSpectra::operator[]		(int index)		{return spectra[index];}
inline Complex &		Spectra::operator[]			(int index)		{return spectra[index];}
inline float &			ChromaticSpectra::operator[](int index)		{return spectra[index];}
inline Complex &		Signal::operator[]			(int index)		{return samples[index];}

inline const Complex &	RawSpectra::operator[]		(int index)	const  {return spectra[index];}
inline const Complex &	Spectra::operator[]			(int index)	const  {return spectra[index];}
inline const float &	ChromaticSpectra::operator[](int index)	const  {return spectra[index];}
inline const Complex &	Signal::operator[]			(int index)	const  {return samples[index];}

Complex *	createTwiddles(Complex * ws, int len);
Complex *	createInverseTwiddles(Complex * ws, int len);
int *		createBitRemap(int * bitRemap, int len);

/*--------------------------------------------------------------------------
*
*///------------------------------------------------------------------------
template <int N>
void fft_preline(Complex *samples, const Complex *twiddleFactors)
{
	//initialise pass parameters}
	int numPasses		= (int)log2f(float(N));
	int numBlocks		= 1<<(numPasses-1);
	int pointsPerBlock	= 2;
	int twiddleStep		= N/2;
	
	//perform p passes}
	for(int pass = 0; pass < numPasses; pass++)
	{
		//pass loop}
		int numButterflies = pointsPerBlock/2;
		
		int evenIndex = 0;
		for( int block = 0; block < numBlocks; block++)
		{
			//block loop}
			int oddIndex = evenIndex + numButterflies;
			for(int k = 0; k < numButterflies; k++)
			{
				//butterfly loop}
				Complex w = twiddleFactors[twiddleStep*k];
				Complex e = samples[evenIndex+k]; 
				Complex o = w * samples[oddIndex+k];
				
				samples [evenIndex+k]	= e + o;
				samples [oddIndex+k]	= e - o;
			}
			
			evenIndex = evenIndex + pointsPerBlock;
		}
		
		// calc parameters for next pass}
		numBlocks		/= 2;
		pointsPerBlock	*= 2;
		twiddleStep		/= 2;
	}
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
template <int N>
void fft_pre(const Complex samples[N], const Complex ws[N/2], char scratch[N], const int bitRemap[N], RawSpectra &spectra)
{ 
	//handle bit reversal
	entropy::Memory_Zero(scratch, N*sizeof(char));
	
	for(int i = 0; i < N; i++)
	{
		int remap = bitRemap[i];
		if(!scratch[i])
		{
			spectra[i]		= samples[remap];
			spectra[remap]	= samples[i];
			
			scratch[i] = 1;
			scratch[remap] = 1;
		}
	}
	
	fft_preline<N>(spectra, ws);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
template <int N>
void ifft_pre(const Complex spectra[N], const Complex iws[N/2], char scratch[N], const int bitRemap[N], Signal &signal)
{
	//handle bit reversal
	entropy::Memory_Zero(scratch, N*sizeof(char));
	
	for(int i = 0; i < N; i++)
	{
		int remap = bitRemap[i];
		if(!scratch[i])
		{
			signal[i]		= spectra[remap];
			signal[remap] = spectra[i];
			
			scratch[i] = 1;
			scratch[remap] = 1;
		}
	}
	
	fft_preline<N>(signal, iws);
}


/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
template<int N>
void fft_prestereo(const Complex buffer[N], const Complex ws[N/2], char scratch[N], const int bitRemap[N], RawSpectra &spectra)
{
	fft_pre<N>(buffer, ws, scratch, bitRemap, spectra);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
template<int N>
void fft_prestereo(const Complex buffer[N], const Complex ws[N/2], char scratch[N], const int bitRemap[N], RawSpectra &leftOut, RawSpectra &rightOut )
{
	RawSpectra temp(N);
	fft_prestereo<N>(buffer, ws, scratch, bitRemap, temp);
	
	leftOut[0] = rightOut[0] = Complex(0,0);
	
	for(int i = 1; i < N; i++)
	{
		int inv = N - i;
		Complex c = temp[i];
		Complex z = ~temp[inv];
		
		leftOut[i] = (c + z)/2.0f;
		rightOut[i] = (-I)*(c - z)/2.0f;
	}
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
template<int N>
void ifft_prestereo(const Complex * spectra, const Complex iws[N/2], char scratch[N], const int bitRemap[N], Signal * signal)
{
	ifft_pre<N>(spectra, iws, scratch, bitRemap, *signal);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
template<int N>
void ifft_prestereo(const Complex * left, const Complex *right, const Complex iws[N/2], char scratch[N], const int bitRemap[N],  Signal * signal)
{
	RawSpectra temp(N);
	
	for(int f = 0; f < N; f++)
	{
		temp[f] = left[f] + I*right[f];
	}
	
	ifft_prestereo<N>(temp, iws, scratch, bitRemap,  signal);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
template <int size>
void	fft_stack(const float samples[size], RawSpectra *spectra)
{
	Complex buffer[size];
	Complex ws[size/2];
	int	bitRemap[size];
	char scratch[size];
	
	createTwiddles(ws, size);
	createBitRemap(bitRemap, size);
	
	for(int i = 0; i < size; i++)
	{
		buffer[i] = Complex(samples[i]);
	}

	fft_pre<size>(buffer, ws, scratch, bitRemap, *spectra);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
template <int size>
void	ifft_stack(const RawSpectra * spectra, Signal *signalOut)
{
	Complex iws[size/2];
	int	bitRemap[size];
	char scratch[size];
	
	createInverseTwiddles(iws, size);
	createBitRemap(bitRemap, size);
	
	ifft_pre<size>(spectra->buffer(), iws, scratch, bitRemap, *signalOut);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
template <int size>
void fft_stereostack(const float left[size], const float right[size], RawSpectra * leftOut, RawSpectra *rightOut)
{
	Complex buffer[size];
	Complex ws[size/2];
	int	bitRemap[size];
	char scratch[size];
	
	createTwiddles(ws, size);
	createBitRemap(bitRemap, size);
	
	for(int i = 0; i < size; i++)
	{
		buffer[i] = Complex(left[i], right[i]);
	}

	fft_prestereo<size>(buffer, ws, scratch, bitRemap, *leftOut, *rightOut);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
template <int size>
void ifft_stereostack(const RawSpectra *left, const RawSpectra * right, Signal *signal)
{
	Complex iws[size/2];
	int	bitRemap[size];
	char scratch[size];
	
	createInverseTwiddles(iws, size);
	createBitRemap(bitRemap, size);
	
	ifft_prestereo<size>(left->buffer(), right->buffer(), iws, scratch, bitRemap, signal);
}

void fft_simple_opt(const Complex *samples, int len, RawSpectra* spectra, Envelope * envelope = NULL);
void fft_simple_opt(const float *samples, int len, RawSpectra * spectra, Envelope * envelope = NULL);
void fft_opt(const Complex * samples, int len, RawSpectra * spectra, Envelope * envelope = NULL);
void fft_opt(const float * samples, int len, RawSpectra * spectra, Envelope *envelope = NULL);

#endif
