
#include "support/native.h"
#if WINDOWS && _DEBUG && MEM_LEAK_CHECKING
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "fft/fft.h"
using namespace std;

	/*
---	** ----------------------------------------------------------------------------------------------
	*/

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
RawSpectra::RawSpectra(int len)
{
	ownMemory = true;
	numSpectra = len;
	spectra = (Complex*)entropy::Memory_Alloc(len * sizeof(Complex));
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
RawSpectra::RawSpectra(const Complex * samples, int len)
{
	ownMemory = true;
	numSpectra = len;
	spectra = (Complex*)entropy::Memory_Alloc(len * sizeof(Complex));
	entropy::Memory_Copy(spectra, samples, len*sizeof(Complex));
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
RawSpectra::RawSpectra(Complex * samples, int len, bool copy)
{
	if(copy)
	{
		ownMemory = true;
		numSpectra = len;
		spectra = (Complex*)entropy::Memory_Alloc(len * sizeof(Complex));
		entropy::Memory_Copy(spectra, samples, len*sizeof(Complex));
	}
	else
	{
		//if we are not copying, consider the memory passed in as a gift (probably stack allocated)
		ownMemory = false;
		numSpectra = len;
		spectra = samples;
	}
}
/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void RawSpectra::copy(const Complex * samples, int len)
{
	entropy::Memory_Copy(spectra, samples, len*sizeof(Complex));
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
RawSpectra::~RawSpectra()
{
	if(ownMemory)
		entropy::Memory_Free(spectra);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
RawSpectra::RawSpectra()
{
	ownMemory = false;
}

	/*
---	** ----------------------------------------------------------------------------------------------
	*/


/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
Signal::Signal()
{
	ownMemory = false;
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
Signal::Signal(int len)
{
	ownMemory = true;
	numSamples = len;
	samples = (Complex*)entropy::Memory_Alloc(len*sizeof(Complex));
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
Signal::Signal(const Complex * spectra, int len)
{
	ownMemory = true;
	numSamples = len;
	samples = (Complex*)entropy::Memory_Alloc(len*sizeof(Complex));
	entropy::Memory_Copy(samples, spectra, len);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
Signal::Signal(Complex * spectra, int len, bool copy)
{
	if(copy)
	{
		ownMemory = true;
		numSamples = len;
		samples = (Complex*)entropy::Memory_Alloc(len*sizeof(Complex));
		entropy::Memory_Copy(samples, spectra, len);
	}
	else
	{
		//if not copying, then consider the memory a gift, probably stack allocated
		ownMemory = false;
		numSamples = len;
		samples = spectra;
	}
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
Signal::~Signal()
{
	if(ownMemory)
		entropy::Memory_Free(samples);
}

	/*
---	** ----------------------------------------------------------------------------------------------
	*/

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
Hamming::Hamming(int l): len(l)
{
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
float Hamming::operator[] (int sample)
{
	return Sample(float(sample)/float(len));
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
float Hamming::Sample(float t)
{
	float a = 0.54f;
	float arg = TAU*t + TAU/2.0f;
	float cos = entropy::Cos(arg);
	return (1.0f - a)*cos + a;
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
Hamming4k::Hamming4k()
{
	for(int i = 0; i < 4*kKB; i++)
	{
		samples[i] = Hamming::Sample(float(i)/float(4*kKB));
	}
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
Hamming16k::Hamming16k()
{
	for(int i = 0; i < 16*kKB; i++)
	{
		samples[i] = Hamming::Sample(float(i)/float(16*kKB));
	}
}

	/*
---	** ----------------------------------------------------------------------------------------------
	*/

	
/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
Spectra::Spectra()
{
	ownMemory = false;
	spectra = NULL;
	numSpectra = 0;
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
Spectra::Spectra(int len)
{
	ownMemory = true;
	numSpectra = len;
	spectra = (Complex*) entropy::Memory_Alloc(sizeof(Complex*)*len);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
Spectra::Spectra(const RawSpectra * raw)
{
	ownMemory = true;
	numSpectra = raw->len()/2;
	spectra = (Complex*) entropy::Memory_Alloc(sizeof(Complex*)*numSpectra);
	
	process(raw);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
Spectra::Spectra(Complex * sp, int len)
{
	ownMemory = false;
	numSpectra = len;
	spectra = sp;
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void Spectra::process(const RawSpectra * raw)
{
	if(raw->len()/2 > numSpectra)
	{	LOGSPRINTF("Not enough space allocated to Spectra (len:%d) to process RawSpectra (len:%d) - needa at least RawSpectra/2 = %d", numSpectra, raw->len(), raw->len()/2);
		EE_ASSERT_MSG(raw->len()/2 <= numSpectra, "Not enough space allocated to Spectra to process RawSpectra");
	}
	
	numSpectra = raw->len()/2;
	
	int len = raw->len();
	float norm = float(len)/65536.0f; //entropy::Sqrt((float)len);
	
	peakValue = 0;
	peakIndex = -1;
	for(int c = 0; c < numSpectra; c++)
	{
		int copyFrom = len - c -1;
		Complex temp = (*raw)[copyFrom]/norm;
		
		if(c != 0 && c < numSpectra - 1)
			temp = temp * 2.0f;
	
		float power = +temp;
		float phase = entropy::ATan2(temp.i, temp.r);
		
		if(power > peakValue)
		{
			peakIndex = c;
			peakValue = power;
		}
		
		spectra[c].r = power;
		spectra[c].i = phase;
	}
}

Complex prebuffer[16*1024];

Spectra * Spectra::preprocess(float time)
{
	entropy::Memory_Zero(prebuffer, sizeof(Complex)*len());

	Spectra * pre = new(entropy::Memory_Alloc(sizeof(Spectra))) Spectra(prebuffer, len());
	
	float A5 = ChromaticSpectra::getA5Frequency();
	
	float step = (float)pow(2.0f, 1.0f/12.0f);
	float A5_adjusted = A5 * time;
	
	float lowerCoeff = (float)pow(2.0f, -2.0f);
	float bottom = A5_adjusted*lowerCoeff;

	float higherCoeff = (float)pow(2.0f, 4.0f);
	float top = A5_adjusted*higherCoeff;

	//todo: find the actual note just under the len/2 if top is > len/2
	if(top >len())
		top = float(len());
	
	LOGSPRINTF("Chromatic Spectrum: bottom %.2f = %.2f * %.2f; top %.2f = %.2f * %.2f; len %d", 
				bottom/time, A5_adjusted/time, lowerCoeff, top/time, A5_adjusted/time, higherCoeff, len);
	
	for(int i = 0; i < (int)bottom; i++)
	{
		pre->spectra[i] = spectra[i];
	}

	peakValue = 0;
	int	n = 0;
	int floor = 0;
	int f = (int)bottom;
	for(; f < len()/2; f++)
	{
		Complex value = spectra[f];
		pre->spectra[f] += value;
		int numharmonic = 10;
		for(int h = 1; h < numharmonic && (f << h) < len(); h++)
		{
			int f0 = f << h;
			float v = value.r/ float(f>>h);
			pre->spectra[f0].r -= v;

			//not sure if this makes sense, but also not sure if I"ll be in 
			// a situation to care
			pre->spectra[f0].i -= value.i;
		}
	}

	for( ; f < len(); f++)
	{
		Complex value = spectra[f];
		pre->spectra[f] += value;
	}

	for(f = 0; f < len(); f++)
	{
		float temp = pre->spectra[f].r;
		// a way to clamp things to >= 0 without using a branch.
		temp = (temp + fabs(temp))/2.0f;
		pre->spectra[f].r = temp;
	}

	return pre;
}


/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
Spectra::~Spectra()
{
	if(ownMemory)
		entropy::Memory_Free(spectra);
}

	/*
---	** ----------------------------------------------------------------------------------------------
	*/
float 	ChromaticSpectra::A5Frequency = 440.0f;
bool	ChromaticSpectra::calcBuckets = false;
float	ChromaticSpectra::freqBuckets[kNumNotes] = {0};

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void ChromaticSpectra::createFreqBuckets()
{
	float A5 = A5Frequency;
	
	float step = (float)pow(2, 1.0f/12.0f);
	float bottom = A5*(float)pow(2, -5);
	
	for(int f = 0; f < kNumNotes; f++)
	{
		freqBuckets[f] = bottom;
		bottom *= step;
	}

	calcBuckets = true;
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
ChromaticSpectra::ChromaticSpectra()
{
	entropy::Memory_Zero(spectra, kNumNotes * sizeof(float));
	entropy::Memory_Zero(bands, kOctaves * sizeof(float));
	
	maxPeak = -1;
	peakValue = 0;
	
	if(!calcBuckets)
		createFreqBuckets();
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void ChromaticSpectra::setA5Frequency(float hz)
{
	A5Frequency = hz;
	if(calcBuckets)
		createFreqBuckets();
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
float ChromaticSpectra::getA5Frequency()
{
	return A5Frequency;
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void ChromaticSpectra::strictProcess(const Spectra * spec, float time)
{
	EE_ASSERT(0);
	/*
	int len = spec->len();
	float A5 = A5Frequency;
	
	float step = (float)pow(2, 1.0f/12.0f);
	float A5_adjusted = A5 * time;
	
	float bottom = A5_adjusted*pow(2.0f, -5.0f);
	float top = A5_adjusted*pow(2.0f, 4.0f);

	//todo: find the actual note just under the len/2 if top is > len/2
	if(top >len)
		top = float(len);
		
	peakValue = 0;
	int	n = 0;
	float f = bottom;
	for(; f <= top; f*=step, n++)
	{
		//TODO develop a window to add up adjacent energies?
		
		int floor = (int)f;
		float rem = f - (float)floor;
		
		Complex left =(*spec)[floor];
		Complex right =(*spec)[floor +1];
		spectra[n] =  left.r*(1-rem)+ right.r*rem;
		
		if(spectra[n] > peakValue)
		{
			maxPeak = n;
			peakValue = spectra[n];
		}
	}
	
	int max = n;
	for(n =0; n < max; n++)
	{
		float val = spectra[n];
		val /= peakValue;
		spectra[n] = val*val;
	}
	*/
}

/*--------------------------------------------------------------------------
*//** 
*///------------------------------------------------------------------------
void ChromaticSpectra::strictProcess(const Spectra * left, const Spectra * right, float time)
{
	EE_ASSERT(0);
	/*
	EE_ASSERT(left->len() == right->len());
	int len = left->len();
	float A5 = A5Frequency;
	
	float step = (float)pow(2.0f, 1.0f/12.0f);
	float A5_adjusted = A5 * time;
	
	float lowerCoeff = (float)pow(2.0f, -5.0f);
	float bottom = A5_adjusted*lowerCoeff;

	float higherCoeff = (float)pow(2.0f, 4.0f);
	float top = A5_adjusted*higherCoeff;

	//todo: find the actual note just under the len/2 if top is > len/2
	if(top >len)
		top = float(len);
	
	LOGSPRINTF("Chromatic Spectrum: bottom %.2f = %.2f * %.2f; top %.2f = %.2f * %.2f; len %d", 
				bottom/time, A5_adjusted/time, lowerCoeff, top/time, A5_adjusted/time, higherCoeff, len);
		
	peakValue = 0;
	int	n = 0;
	int floor = 0;
	float f = bottom;
	for(; f < top; f*=step, n++)
	{
		//TODO develop a window to add up adjacent energies?
		floor = (int)f;
		float rem = f - (float)floor;
		
		if(n >= kNumNotes || floor >= left->len()-1)
		{
			LOGSPRINTF("Chromatic Spectrum: Stopping because either iterations %d == notes %d or freq %d has hit nyquist %d", 
					n, kNumNotes, floor+1, left->len());
			break;
		}

		Complex l1 =(*left)[floor];
		Complex l2 =(*right)[floor +1];
		
		Complex r1 =(*right)[floor];
		Complex r2 =(*right)[floor+1];
		
		float val1 =(r1.r+l1.r)*(1-rem);
		float val2 =(l1.r+l2.r)*rem;
		
		spectra[n] = val1 > val2? val1: val2 ;
		
		if(spectra[n] > peakValue)
		{
			maxPeak = n;
			peakValue = spectra[n];
		}
	}

	LOGSPRINTF("Chromatic Spectrum: iterations %d, freq %d at end ", n, floor+1);
			
	//if we fill in less than all of the notes because our spectra is too small,
	// padd the rest with zeroes
	for( n ; n < kNumNotes; n++)
		spectra[n] = 0;
	
	int max = n;
	for(n = 0; n < max; n++)
	{
		float val = spectra[n];
		val /= peakValue;
		spectra[n] *= val*val;
	}
	*/
}

/*--------------------------------------------------------------------------
*//** 
*///------------------------------------------------------------------------
void ChromaticSpectra::bandProcess(const Spectra * spec, float time)
{
	int len				= spec->len();

	float A5			= A5Frequency;
	
	float step			= (float)pow(2.0f, 1.0f/12.0f);
	float halfstep		= (float)pow(2.0f, 1.0f/24.0f);
	float A5_adjusted	= A5 * time;
	
	float bottom		= A5_adjusted*(float)pow(2.0f, -2.0f);
	float top			= A5_adjusted*(float)pow(2.0f, 4.0f);

	if(top >len)
		top = float(len);
	
	int	n = 0;
	int bandbottom = 0;
	int bandtop = (int)bottom;
	for( ; bandtop <= top; bandtop *= 2, n++)
	{
		float ac = 0;
		int diff = (int)(bandtop-bandbottom);
	
		for(int l = bandbottom; l < bandtop; l++)
		{
			float value = (*spec)[l].r;
			ac += value*value;
			ac /= 65336.0f;
		}
		bands[n] = ac;
		bandbottom = bandtop;
	}

	//one final interation of the band from the last octave all the
	// way to the top
	{
		float ac = 0;
		int diff = (int)(len-bandbottom);
	
		for(int l = (int)bandbottom; l < len; l++)
		{
			float value = (*spec)[l].r;
			ac += value*value;
			ac /= 65336.0f;
		}
		bands[n] = ac;
	}

	maxPeak = -1;
	peakValue = 0;

	n = 0;
	float f = bottom;
	for(; f <= top; f*=step, n++)
	{
		if(n == kNumNotes)
			break;

		float lower = f/halfstep;
		float higher= f*halfstep;
		
		float ac = 0;

		int diff = (int)(higher-lower);
		if(diff ==0) diff = 1;
		for(int l = (int)lower; l < (int)lower + diff; l++)
		{
			int offset = (int)(l-lower);
			float value = (*spec)[l].r;
			ac += value*value;
			ac /= 65336.0f;
		}
		
		spectra[n] = ac;
		
		if(ac > peakValue)
		{
			maxPeak = n;
			peakValue = ac;
		}
	}
	
	/** //Not compatible with multiplying by local ham value?
	int max = n;
	for(n =0; n < max; n++)
	{
		float val = spectra[n];
		val /= peakValue;
		spectra[n] = val*val;
	}
	*/
}

//rectified distance
inline float rdist(float s1, float s0)
{
	float temp = fabs(s1) - fabs(s0);
	return (temp + fabs(temp))/2.0f;
}

/*--------------------------------------------------------------------------
*//** 
*///------------------------------------------------------------------------
void ChromaticSpectra::deltaProcess(const Spectra * n1, const Spectra * n0, float time)
{
	EE_ASSERT(n1->len() == n0->len());
	int len				= n1->len();

	float A5			= A5Frequency;
	
	float step			= (float)pow(2.0f, 1.0f/12.0f);
	float halfstep		= (float)pow(2.0f, 1.0f/24.0f);
	float A5_adjusted	= A5 * time;
	
	float bottom		= A5_adjusted*(float)pow(2.0f, -2.0f);
	float top			= A5_adjusted*(float)pow(2.0f, 4.0f);

	if(top >len)
		top = float(len);
	
	int	n = 0;
	int bandbottom = 0;
	int bandtop = (int)bottom;
	for( ; bandtop <= top; bandtop *= 2, n++)
	{
		float ac = 0;
		int diff = (int)(bandtop-bandbottom);
	
		for(int l = bandbottom; l < bandtop; l++)
		{
			float s1 = (*n1)[l].r, s0 = (*n0)[l].r;
			ac += rdist(s1, s0);
		}
		bands[n] = ac;
		bandbottom = bandtop;
	}

	//one final interation of the band from the last octave all the
	// way to the top
	{
		float ac = 0;
		int diff = (int)(len-bandbottom);
	
		for(int l = (int)bandbottom; l < len; l++)
		{
			float s1 = (*n1)[l].r, s0 = (*n0)[l].r;
			ac += rdist(s1, s0);
		}
		bands[n] = ac;
	}

	maxPeak = -1;
	peakValue = 0;

	n = 0;
	float f = bottom;
	for(; f <= top; f*=step, n++)
	{
		if(n == kNumNotes)
			break;

		float lower = f/halfstep;
		float higher= f*halfstep;
		
		float ac = 0;

		int diff = (int)(higher-lower);
		if(diff ==0) diff = 1;
		for(int l = (int)lower; l < (int)lower + diff; l++)
		{
			int offset = (int)(l-lower);
			float s1 = (*n1)[l].r, s0 = (*n0)[l].r;
			ac += rdist(s1, s0);
		}
		
		spectra[n] = ac;
		
		if(ac > peakValue)
		{
			maxPeak = n;
			peakValue = ac;
		}
	}
	
	/** //Not compatible with multiplying by local ham value?
	int max = n;
	for(n =0; n < max; n++)
	{
		float val = spectra[n];
		val /= peakValue;
		spectra[n] = val*val;
	}
	*/
}

/*--------------------------------------------------------------------------
*//** 
*///------------------------------------------------------------------------
void ChromaticSpectra::bandProcess(const Spectra * left, const Spectra * right, float time)
{
	EE_ASSERT(left->len() == right->len());
	//TODO Recreate from above after I have it all solved out
}

/*--------------------------------------------------------------------------
**///-----------------------------------------------------------------------
float* ChromaticSpectra::getFrequencies()
{
	if(!calcBuckets)
		createFreqBuckets();
	return freqBuckets;
}

/*--------------------------------------------------------------------------
**///-----------------------------------------------------------------------
BeatSpectra::BeatSpectra()
{
	counter = -16;
	
	for(int i = 0; i < kHistory; i++)
	{
		for(int n = 0; n < ChromaticSpectra::kNumNotes; n++)
		{
			history[i][n] = 0;
		}
		
		for(int b = 0; b < ChromaticSpectra::kBands; b++)
			history[i].bands[b] = 0;
	}

	for(int n = 0; n < ChromaticSpectra::kNumNotes; n++)
	{
		beat[n]		= 0;
		energy[n]	= 0;
		variance[n]	= 0;
	}	
	
	for(int b = 0; b < ChromaticSpectra::kBands; b++)
	{
		beat.bands[b] = 0;
		energy.bands[b] = 0;
		variance.bands[b] = 0;
	}
}		

/*--------------------------------------------------------------------------
**///-----------------------------------------------------------------------
float BeatSpectra::SpectraAverageEnergy(int band)
{
	float ac = 0;
	for(int i = 0; i < kHistory; i++)
	{
		float real = history[i][band];
		ac += real;
	}

	ac /= (float)kHistory;
	return ac;
}

/*--------------------------------------------------------------------------
**///-----------------------------------------------------------------------
float BeatSpectra::SpectraVariance(int band, float avg)
{
	float ac = 0;
	for(int i = 0; i < kHistory; i++)
	{
		float real = history[i][band];
		float temp = real - avg;

		ac += temp*temp;
	}

	ac /= (float)kHistory;
	return ac;
}

/*--------------------------------------------------------------------------
**///-----------------------------------------------------------------------
float BeatSpectra::BandAverageEnergy(int band)
{
	float ac = 0;
	for(int i = 0; i < kHistory; i++)
	{
		float real = history[i].bands[band];
		ac += real;
	}

	ac /= (float)kHistory;
	return ac;
}

/*--------------------------------------------------------------------------
**///-----------------------------------------------------------------------
float BeatSpectra::BandVariance(int band, float avg)
{
	float ac = 0;
	for(int i = 0; i < kHistory; i++)
	{
		float real = history[i].bands[band];
		float temp = real - avg;

		ac += temp*temp;
	}

	ac /= (float)kHistory;
	return ac;
}


/*--------------------------------------------------------------------------
**///-----------------------------------------------------------------------
void BeatSpectra::ProcessSpectra(const ChromaticSpectra &append)
{
	for(int n =0; n< append.kNumNotes; n++)
	{
		float e = SpectraAverageEnergy(n);
		float v = SpectraVariance(n, e);
		
		float entry = append[n];

		energy[n] = e;
		variance[n]= v;
		beat[n] = entry;
	}

	for(int i = kHistory - 1; i > 0; i--)
	{
		for(int n = 0; n < append.kNumNotes; n++)
		{
			history[i][n] = history[i-1][n];
		}
	}
	
	for(int n =0; n< append.kNumNotes; n++)
	{
		float entry = append[n];
		history[0][n] = entry;
	}

}

/*--------------------------------------------------------------------------
**///-----------------------------------------------------------------------
void BeatSpectra::ProcessBands(const ChromaticSpectra & append)
{
	for(int n =0; n< append.kBands; n++)
	{
		float e = BandAverageEnergy(n);
		float v = BandVariance(n, e);
		
		float entry = append[n];

		energy.bands[n] = e;
		variance.bands[n]= v;
		beat.bands[n] = entry;
	}

	for(int i = kHistory - 1; i > 0; i--)
	{
		for(int n = 0; n < append.kBands; n++)
		{
			history[i].bands[n] = history[i-1].bands[n];
		}
	}
	
	for(int n =0; n< append.kBands; n++)
	{
		float entry = append[n];
		history[0].bands[n] = entry;
	}
}

/*--------------------------------------------------------------------------
**///-----------------------------------------------------------------------
void BeatSpectra::Process(const ChromaticSpectra & append)
{
	ProcessBands(append);
	ProcessSpectra(append);
	counter++;
}


//scratch space that is tied to the size of the fft we are trying to process
Complex * scratch = NULL;

/*--------------------------------------------------------------------------
**
*///------------------------------------------------------------------------
void fft_recurse(Complex * samples, int len)
{
	if(len == 2)
	{
		Complex s1 = samples[0] + samples[1];
		Complex s2 = samples[0] - samples[1];
		samples[0] = s1;
		samples[1] = s2;
		return;
	}

	int half = len / 2;
	
	Complex * buffer = scratch+len;
	
	Complex *even	= buffer;
	Complex *odd	= buffer + half;
	
	for(int i = 0; i < half; i++)
	{
		even [i] = samples[2*i];
		odd [i] = samples[2*i + 1];
	}

	fft_recurse(even, half);
	fft_recurse(odd, half);
	
	for(int k = 0; k < half; k++)
	{
		float phase =(float)k/(float)len;
		Complex w = expi( -1.0f*TAU * phase);
		Complex e = even[k];
		Complex o = w * odd[k];
		samples[k] = e + o;
		samples[k+half] = e - o;
	}
}


/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void fft_simple(const Complex *samples, int len, RawSpectra* spectra, Envelope * envelope)
{
	EE_ASSERT(isPowerOfTwo(len));
	
	Complex * buffer	= *spectra;
	
	//TODO: don't dynamically allocate this scratch space?
	//scratch	= new Complex[2*len];
	scratch = (Complex*)entropy::Memory_Alloc(2*len*sizeof(Complex));
	
	for(int i = 0; i < len; i++)
	{
		if(envelope)
			buffer[i] = samples[i] * (*envelope)[i];
		else
			buffer[i] = samples[i];
	}
	
	fft_recurse(buffer, len);
	
	//delete[] scratch;
	entropy::Memory_Free(scratch);
	scratch = NULL;
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void fft_simple(const float *samples, int len, RawSpectra * spectra, Envelope * envelope)
{
	EE_ASSERT(isPowerOfTwo(len));
	
	Complex * buffer = (Complex*)entropy::Memory_Alloc(len * sizeof(Complex));
	
	for(int i = 0; i < len; i++)
	{
		Complex temp = Complex(samples[i]);
		buffer[i] = temp;
	}
	
	fft_simple(buffer, len, spectra, envelope);
	
	entropy::Memory_Free(buffer);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void ifft_simple(const RawSpectra *samples,  Signal* signal)
{
	int len = signal->len();
	
	Complex * buffer	= *signal;
	//scratch	= new Complex[2*len];
	scratch = (Complex *) entropy::Memory_Alloc(2*len*sizeof(Complex));
	
	for(int i = 0; i < len; i++)
	{
		buffer[i] = ~(*samples)[i];
	}
	
	fft_recurse(buffer, len);
	
	for(int i = 0; i < len; i++)
	{
		buffer[i] = (~buffer[i]);
	}

	//delete[] scratch;
	entropy::Memory_Free(scratch);
	scratch = NULL;
}

	/*
---	** ----------------------------------------------------------------------------------------------
	*/

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
int * createBitRemap(int * bitRemap, int len)
{
	int numBits = (int)log2f((float)len);
	
	for(int i = 0; i < len; i++)
	{
		int ac = 0;
		for(int b = numBits -1, b2 = 0; b >= 0; b--, b2++)
		{
			int bit = 1 << b;
			int rbit = 1 << b2;
			
			if(i & bit)
				ac |= rbit;
		}
		
		bitRemap[ac] = i;
	}
	
	return bitRemap;
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
int * createBitRemap(int len)
{
	int * bitRemap = (int*)entropy::Memory_Alloc(len*sizeof(int));
	return createBitRemap(bitRemap, len);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
Complex * createTwiddles(Complex * ws, int len)
{
	for(int k =0; k < len/2; k++)
	{
		float phase =(float)k/ (float)len;
		Complex  w = expi(-1.0f*TAU * phase);
		ws[k] = w;
	}
	return ws;
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
Complex * createInverseTwiddles(Complex * ws, int len)
{
	for(int k =0; k < len/2; k++)
	{
		float phase =(float)k/ (float)len;
		Complex  w = expi(TAU * phase);
		ws[k] = w;
	}
	return ws;
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
Complex * createTwiddles(int len)
{
	Complex *ws = (Complex*)entropy::Memory_Alloc(len/2 * sizeof(Complex));
	return createTwiddles(ws, len);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
Complex * createInverseTwiddles(int len)
{
	Complex *ws = (Complex*)entropy::Memory_Alloc(len/2 * sizeof(Complex));
	return createInverseTwiddles(ws, len);
}


/*--------------------------------------------------------------------------
* Thanks, engineering productivity tools, for the tutorial!
*///------------------------------------------------------------------------
Complex * fft_inline(Complex *samples, int len, const Complex *twiddleFactors)
{
	//initialise pass parameters}
	int numPasses		= (int)log2f(float(len));
	int numBlocks		= 1<<(numPasses-1);
	int pointsPerBlock	= 2;
	int twiddleStep		= len/2;
	
	//perform p passes
	for(int pass = 0; pass < numPasses; pass++)
	{
		//pass loop 
		int numButterflies = pointsPerBlock/2;
		
		int evenIndex = 0;
		for( int block = 0; block < numBlocks; block++)
		{
			//block loop
			int oddIndex = evenIndex + numButterflies;
			for(int k = 0; k < numButterflies; k++)
			{
				//butterfly loop
				Complex w = twiddleFactors[twiddleStep*k];
				Complex e = samples[evenIndex+k];
				Complex o = w * samples[oddIndex+k];
				
				samples [evenIndex+k]	= e + o;
				samples [oddIndex+k]	= e - o;
			}
			
			evenIndex += pointsPerBlock;
		}
		
		// calc parameters for next pass
		numBlocks		/= 2;
		pointsPerBlock	*= 2;
		twiddleStep		/= 2;
	}
	
	return samples;
}



/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void fft(const Complex * samples, int len, RawSpectra * spectra, Envelope * envelope)
{
	EE_ASSERT(isPowerOfTwo(len));
	EE_ASSERT(len == spectra->len());

	int * bitRemap = createBitRemap(len);
	Complex * ws = createTwiddles(len);
	
	{
		//handle bit reversal
		char * scratch = (char*)alloca(len);
		entropy::Memory_Zero(scratch, len*sizeof(char));
		
		for(int i = 0; i < len; i++)
		{
			int remap = bitRemap[i];
			if(!scratch[i])
			{
				float envi = 1.0;
				float envr = 1.0;
				if(envelope)
				{
					envi	= (*envelope)[i];
					envr	= (*envelope)[remap];
				}
				
				(*spectra)[i]		= samples[remap] * envr;
				(*spectra)[remap]	= samples[i] * envi;
				
				scratch[i] = 1;
				scratch[remap] = 1;
			}
		}
	}
	
	fft_inline(spectra->buffer(), len, ws);
	
	entropy::Memory_Free(bitRemap);
	entropy::Memory_Free(ws);
}


/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void fft(const float * samples, int len, RawSpectra * spectra, Envelope *envelope)
{
	EE_ASSERT(isPowerOfTwo(len));
	Complex * buffer = (Complex*)malloc(sizeof(Complex)* len);
	
	for(int s = 0; s < len; s++)
	{
		buffer[s] = Complex(samples[s]);
	}
	
	fft(buffer, len, spectra, envelope);
	
	entropy::Memory_Free(buffer);
}


/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	ifft(const RawSpectra * spectra, Signal * signalOut)
{
	int len = spectra->len();
	int * bitRemap = createBitRemap(len);
	Complex * iws = createInverseTwiddles(len);
	
	{
		//handle bit reversal
		char * scratch = (char*)alloca(len);
		entropy::Memory_Zero(scratch, len*sizeof(char));
		
		for(int i = 0; i < len; i++)
		{
			int remap = bitRemap[i];
			if(!scratch[i])
			{
				(*signalOut)[i]		= (*spectra)[remap];
				(*signalOut)[remap]	= (*spectra)[i];
				
				scratch[i] = 1;
				scratch[remap] = 1;
			}
		}
	}
	
	fft_inline(signalOut->buffer(), len, iws);
	
	
	entropy::Memory_Free(bitRemap);
	entropy::Memory_Free(iws);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	ifft_finalize(const Signal *signal, float * samplesOut)
{
	int len = signal->len();
	for(int s = 0; s < len; s++)
	{
		samplesOut[s] = (*signal)[s].r / float(len);
	}
}


/*--------------------------------------------------------------------------
*//** MAYBE: Shift "space" here so that we get frequency information automatically
** in the chromatic scale, which would reduce the density of our passes from
** 40k to perhaps ~200-300, even when accounting for dissipated energy
*///------------------------------------------------------------------------
void	fft_chromatic(const float *samples, int len, float time, ChromaticSpectra* spectraOut, Envelope * envelope)
{
	EE_ASSERT_MSG(0, "unimplemented");
}
	/*
---	** ----------------------------------------------------------------------------------------------
	*/

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
template <int N>
int * createBitRemap()
{
	int * ret = (int *) entropy::Memory_Alloc (N*sizeof(int));
	createBitRemap(ret, N);
	return ret;
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
template <int N>
Complex * createTwiddles()
{
	Complex * ws = (Complex*)  entropy::Memory_Alloc(N*sizeof(Complex));
	createTwiddles(ws, N);
	return ws;
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
template <int N>
Complex * createInverseTwiddles()
{
	Complex * ws = (Complex*)  entropy::Memory_Alloc(N*sizeof(Complex));
	createInverseTwiddles(ws, N);
	return ws;
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
template <int N>
char * createScratchSapce()
{
	char * scratch = (char*)  entropy::Memory_Alloc(N*sizeof(char));
	return scratch;
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
template <int N>
Complex * createBufferSpace()
{
	Complex * buffer = (Complex*) entropy::Memory_Alloc(N * sizeof(Complex));
	return buffer;
}



	/*
---	** ----------------------------------------------------------------------------------------------
	*/

static Complex  ws_4k		[4*kKB/2];
static Complex  iws_4k		[4*kKB/2];
static Complex	buffer_4k	[4*kKB];
static char		scratch_4k	[4*kKB];
static int		bitRemap_4k	[4*kKB];

static Complex	ws_16k		[16*kKB/2];
static Complex	iws_16k		[16*kKB/2];
static Complex	buffer_16k	[16*kKB];
static char		scratch_16k	[16*kKB];
static int		bitRemap_16k[16*kKB];


/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void fft16k(const float samples[16*kKB], RawSpectra * spectra, Envelope *envelope)
{
	const int size =16*kKB;
	EE_ASSERT(spectra->len() == size);
	if(envelope)
	{
		for(int i = 0; i < size; i++)
		{
			buffer_16k[i] = Complex(samples[i]) * (*envelope)[i];
		}
	}
	else for(int i = 0; i < size; i++)
	{
		buffer_16k[i] = Complex(samples[i]);
	}

	fft_pre<size>(buffer_16k, ws_16k, scratch_16k, bitRemap_16k, *spectra);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void fft4k(const float samples[4*kKB], RawSpectra * spectra, Envelope *envelope)
{
	const int size =4*kKB;
	EE_ASSERT(spectra->len() == size);
	if(envelope)
	{
		for(int i = 0; i < size; i++)
		{
			buffer_4k[i] = Complex(samples[i]) * (*envelope)[i];
		}
	}
	else for(int i = 0; i < size; i++)
	{
		buffer_4k[i] = Complex(samples[i]);
	}
	
	//MAYBE: create multiple 4k buffer sets so that we can run multiple 4k FFTs
	fft_pre<size>(buffer_4k, ws_4k, scratch_4k, bitRemap_4k, *spectra);
}


/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void ifft16k(const RawSpectra * spectra, Signal *signalOut)
{
	const int size =16*kKB;
	
	//MAYBE: create multiple 4k buffer sets so that we can run multiple 4k FFTs
	ifft_pre<size>(spectra->buffer(), iws_16k, scratch_16k, bitRemap_16k, *signalOut);
	
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void ifft4k(const RawSpectra * spectra, Signal *signalOut)
{
	const int size =4*kKB;
	
	//MAYBE: create multiple 4k buffer sets so that we can run multiple 4k FFTs
	ifft_pre<size>(spectra->buffer(), iws_4k, scratch_4k, bitRemap_4k, *signalOut);
}

	/*
---	** -----------------------------------------------------------------------------------------------------------
	*/

/*--------------------------------------------------------------------------
*//** Stereo Unpack expects all three RawSpectra parameters to previously have
** been initialized to the right buffer lengths
*///------------------------------------------------------------------------
void fft_stereo_unpack(RawSpectra * interleaved, RawSpectra * leftOut, RawSpectra * rightOut)
{
	EE_ASSERT(interleaved->len() == leftOut->len());
	EE_ASSERT(interleaved->len() == rightOut->len());
	
	Complex * buffer = interleaved->buffer();
	int len = interleaved->len();
	
	(*leftOut)[0] = (*rightOut)[0] = Complex(0,0);
	 
	for(int f = 1; f < len; f++)
	{
		int inv = len - f;
		Complex c = buffer[f];
		Complex z = buffer[inv];
		
		(*leftOut)[f] = (c + ~z)/2.0f;
		(*rightOut)[f] = (-I)*(c - ~z)/2.0f;
	}
}


/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	fft_stereo(const float * left, const float *right, int len, RawSpectra* spectra, Envelope * envelope)
{
	EE_ASSERT(isPowerOfTwo(len));
	EE_ASSERT(spectra->len() == len);
	Complex * buffer = (Complex*)entropy::Memory_Alloc(sizeof(Complex)* len);
	
	for(int s = 0; s < len; s++)
	{
		buffer[s] = Complex(left[s], right[s]);
	}
	
	fft(buffer, len, spectra, envelope);
	
	entropy::Memory_Free(buffer);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	fft_stereo(const float * left, const float *right, int len, RawSpectra* leftOut, RawSpectra* rightOut, Envelope * envelope)
{
	RawSpectra temp(leftOut->len());
	
	//used to use leftOut here, and then reuse it again below, but found out
	// that I need a temp otherwise I'd get wrong data halfway through processing
	//MAYBE: optimize the temp away?
	fft_stereo(left, right, len, &temp, envelope);
	fft_stereo_unpack(&temp, leftOut, rightOut);
}

/*--------------------------------------------------------------------------
** NOTE: len is the number of frames (left-right sample pairs), so there should
** be 2x len number of samples in the array
*///------------------------------------------------------------------------
void	fft_stereo	(const float * interleaved, int len, RawSpectra *out, Envelope * envelope)
{
	EE_ASSERT(isPowerOfTwo(len));
	EE_ASSERT(out->len() == len);
	
	fft((Complex*)interleaved, len, out, envelope);
}

/*--------------------------------------------------------------------------
** NOTE: len is the number of frames (left-right sample pairs), so there should
** be 2x len number of samples in the array
*///------------------------------------------------------------------------
void	fft_stereo	(const float * interleaved, int len, RawSpectra * leftOut, RawSpectra * rightOut, Envelope * env)
{
	RawSpectra temp(leftOut->len());

	fft_stereo(interleaved, len, &temp, env);	
	fft_stereo_unpack(&temp, leftOut, rightOut);
}


/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	ifft_stereo(const RawSpectra* spectra, Signal * out)
{
	ifft(spectra, out);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	ifft_stereo(const RawSpectra * left, const RawSpectra * right, Signal * out)
{
	RawSpectra temp(left->len());
	
	for(int f = 0; f < left->len(); f++)
	{
		Complex l = (*left)[f];
		Complex	r = (*right)[f];
		Complex o = l + I*r;
	
		temp[f] = o;
	}
	
	ifft_stereo(&temp, out);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	ifft_stereofinalize(const Signal * signal, float *left, float *right)
{
	int len = signal->len();
	for(int s = 0; s < len; s++)
	{
		Complex c = (*signal)[s] / float(len);
		left[s] = c.r;
		right[s]= c.i;
	}
}


/*--------------------------------------------------------------------------
*//** Since interleaved is two channels, it has to be twice the length of 
** Signal
*///------------------------------------------------------------------------
void	ifft_stereofinalize(const Signal * signal, float *interleaved)
{
	int len = signal->len();
	for(int s = 0; s < len; s++)
	{
		Complex c = (*signal)[s] / float(len);
		interleaved[2*s]	= c.r;
		interleaved[2*s+1]	= c.i;
	}
}

	/*
---	** -----------------------------------------------------------------------------------------------------------
	*/

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	fft16k_stereo(const float left[16*kKB], const float right[16*kKB], RawSpectra * spectra, Envelope * envelope)
{
	const int size =16*kKB;
	
	if(envelope)
	{
		for(int i = 0; i < size; i++)
		{
			buffer_16k[i] = Complex(left[i], right[i]) * (*envelope)[i];
		}
	}
	else for(int i = 0; i < size; i++)
	{
		buffer_16k[i] = Complex(left[i], right[i]);
	}

	EE_ASSERT(spectra->len() == size);
	fft_prestereo<size>(buffer_16k, ws_16k, scratch_16k, bitRemap_16k, *spectra);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	fft16k_stereo(const float left[16*kKB], const float right[16*kKB], RawSpectra * leftOut, RawSpectra *rightOut, Envelope * envelope)
{
	const int size =16*kKB;
	
	
	if(envelope)
	{
		for(int i = 0; i < size; i++)
		{
			buffer_16k[i] = Complex(left[i], right[i]) * (*envelope)[i];
		}
	}
	else for(int i = 0; i < size; i++)
	{
		buffer_16k[i] = Complex(left[i], right[i]);
	}
	
	EE_ASSERT(leftOut->len() == size);
	EE_ASSERT(rightOut->len() == size);
	fft_prestereo<size>(buffer_16k, ws_16k, scratch_16k, bitRemap_16k, *leftOut, *rightOut);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	fft16k_stereo(const float interleaved[2*16*kKB], RawSpectra * spectraOut, Envelope * envelope)
{
	const int size =16*kKB;
	
	if(envelope)
	{
		for(int i = 0; i < size; i++)
		{
			buffer_16k[i] = Complex(interleaved[2*i], interleaved[2*i+1]) * (*envelope)[i];
		}
	}
	else for(int i = 0; i < size; i++)
	{
		buffer_16k[i] = Complex(interleaved[2*i], interleaved[2*i+1]);
	}
	
	EE_ASSERT(spectraOut->len() == size);
	fft_prestereo<size>(buffer_16k, ws_16k, scratch_16k, bitRemap_16k, *spectraOut);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	fft16k_stereo(const float interleaved[2*16*kKB], RawSpectra * leftOut, RawSpectra* rightOut, Envelope * envelope)
{
	const int size =16*kKB;
	
	if(envelope)
	{
		for(int i = 0; i < size; i++)
		{
			buffer_16k[i] = Complex(interleaved[2*i], interleaved[2*i+1]) * (*envelope)[i];
		}
	}
	else for(int i = 0; i < size; i++)
	{
		buffer_16k[i] = Complex(interleaved[2*i], interleaved[2*i+1]);
	}
	
	EE_ASSERT(leftOut->len() == size);
	EE_ASSERT(rightOut->len() == size);
	fft_prestereo<size>(buffer_16k, ws_16k, scratch_16k, bitRemap_16k, *leftOut, *rightOut);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	fft4k_stereo(const float left[4*kKB], const float right[4*kKB], RawSpectra *spectra, Envelope * envelope)
{
	const int size =4*kKB;
		
	if(envelope)
	{
		for(int i = 0; i < size; i++)
		{
			buffer_4k[i] = Complex(left[i], right[i]) * (*envelope)[i];
		}
	}
	else for(int i = 0; i < size; i++)
	{
		buffer_4k[i] = Complex(left[i], right[i]);
	}
	EE_ASSERT(spectra->len() == size);
	fft_prestereo<size>(buffer_4k, ws_4k, scratch_4k, bitRemap_4k, *spectra);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	fft4k_stereo(const float left[4*kKB], const float right[4*kKB], RawSpectra *leftOut, RawSpectra * rightOut, Envelope * envelope)
{
	const int size =4*kKB;
		
	if(envelope)
	{
		for(int i = 0; i < size; i++)
		{
			buffer_4k[i] = Complex(left[i], right[i]) * (*envelope)[i];
		}
	}
	else for(int i = 0; i < size; i++)
	{
		buffer_4k[i] = Complex(left[i], right[i]);
	}
	EE_ASSERT(leftOut->len() == size);
	EE_ASSERT(rightOut->len() == size);
	fft_prestereo<size>(buffer_4k, ws_4k, scratch_4k, bitRemap_4k, *leftOut, *rightOut);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	fft4k_stereo(const float interleaved[2*4*kKB], RawSpectra *spectraOut, Envelope * envelope)
{
	const int size =4*kKB;
		
	if(envelope)
	{
		for(int i = 0; i < size; i++)
		{
			buffer_4k[i] = Complex(interleaved[2*i], interleaved[2*i+1]) * (*envelope)[i];
		}
	}
	else for(int i = 0; i < size; i++)
	{
		buffer_4k[i] = Complex(interleaved[2*i], interleaved[2*i+1]);
	}
	EE_ASSERT(spectraOut->len() == size);
	fft_prestereo<size>(buffer_4k, ws_4k, scratch_4k, bitRemap_4k, *spectraOut);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	fft4k_stereo(const float interleaved[2*4*kKB], RawSpectra *leftOut, RawSpectra * rightOut, Envelope * envelope)
{
	const int size =4*kKB;
		
	if(envelope)
	{
		for(int i = 0; i < size; i++)
		{
			buffer_4k[i] = Complex(interleaved[2*i], interleaved[2*i+1]) * (*envelope)[i];
		}
	}
	else for(int i = 0; i < size; i++)
	{
		buffer_4k[i] = Complex(interleaved[2*i], interleaved[2*i+1]);
	}
	EE_ASSERT(leftOut->len() == size);
	EE_ASSERT(rightOut->len() == size);
	fft_prestereo<size>(buffer_4k, ws_4k, scratch_4k, bitRemap_4k, *leftOut, *rightOut);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	ifft16k_stereo(const RawSpectra * spectra, Signal * signalOut)
{
	const int size =16*kKB;
	ifft_prestereo<size>(spectra->buffer(), ws_16k, scratch_16k, bitRemap_16k,   signalOut);
}


/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	ifft16k_stereo(const RawSpectra * left, const RawSpectra * right, Signal * signal)
{
	const int size =16*kKB;

	ifft_prestereo<size>(left->buffer(), right->buffer(), ws_16k, scratch_16k, bitRemap_16k, signal);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	ifft4k_stereo(const RawSpectra * spectra, Signal *signalOut)
{
	const int size =4*kKB;
	
	//MAYBE: create multiple 4k buffer sets so that we can run multiple 4k FFTs
	ifft_prestereo<size>(spectra->buffer(), iws_4k, scratch_4k, bitRemap_4k, signalOut);
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	ifft4k_stereo(const RawSpectra * left, const RawSpectra * right, Signal *signalOut)
{
	const int size =4*kKB;
	ifft_prestereo<size>(left->buffer(), right->buffer(), iws_4k, scratch_4k, bitRemap_4k, signalOut);
}

	/*
---	** -----------------------------------------------------------------------------------------------------------
	*/

void initFFTSystem()
{
	createBitRemap(bitRemap_4k, 4*kKB);
	createBitRemap(bitRemap_16k, 16*kKB);
	
	createInverseTwiddles(iws_16k, 16*kKB);
	createInverseTwiddles(iws_4k, 4*kKB);
	
	createTwiddles(ws_16k, 16*kKB);
	createTwiddles(ws_4k, 4*kKB);

	//fftSystemTest();
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void	fftSystemTest()
{
	/*
	float r2 = entropy::Sqrt(2)/2.0f;
	float left[8] = {0, 1, 0, -1, 0, 1, 0, -1};
	float right[8]= {1, r2, 0, -r2, -1, -r2, 0, r2};
	
	RawSpectra lsp;
	RawSpectra rsp;
	
	Complex l8[8];
	Complex r8[8];
	
	lsp.setMemory(l8, 8);
	rsp.setMemory(r8, 8);
	fft_stack<8>(right, &rsp);
	//fft_stereostack<8>(left, right, &lsp, &rsp);
	
	Signal signal;
	Complex sg8[8];
	signal.setMemory(sg8, 8);
	 
	ifft_stereostack<8>(&lsp, &rsp, &signal);
	
	float releft[8];
	float reright[8];
	ifft_stereofinalize(&signal, releft, reright);
	/*/
	
	float r2 = entropy::Sqrt(2)/2.0f;
	float left[32] = {0, 1, 0, -1, 0, 1, 0, -1,0, 1, 0, -1, 0, 1, 0, -1,0, 1, 0, -1, 0, 1, 0, -1,0, 1, 0, -1, 0, 1, 0, -1};
	float right[8]= {1, r2, 0, -r2, -1, -r2, 0, r2};
	
	RawSpectra lsp;
	RawSpectra rsp;
	
	Complex l8[32];
	Complex r8[32];
	
	lsp.setMemory(l8, 32);
	rsp.setMemory(r8, 32);
	
	fft(left, 32, &rsp);
	fft_opt(left, 32, &lsp);
		
	r2 = entropy::Sqrt(2)/2.0f;
	//*/
}


	/*
---	**	F O R M E R - I N L I N E S ---------------------------------------------------
	*/
	
void RawSpectra::setMemory(Complex * mem, int len)
{
	ownMemory	= false;
	spectra		= mem;
	numSpectra	= len;
}

int RawSpectra::len() const
{
	return numSpectra;
}

Complex * RawSpectra::buffer()
{
	return spectra;
}

const Complex * RawSpectra::buffer() const
{
	return spectra;
}
	
int Spectra::len() const
{
	return numSpectra;
}

Complex * Spectra::buffer()
{
	return spectra;
}

const Complex * Spectra::buffer()const
{
	return spectra;
}

void Spectra::setMemory(Complex * mem, int len)
{
	ownMemory	= false;
	spectra		= mem;
	numSpectra	= len;
}

int Signal::len() const
{
	return numSamples;
}

Complex * Signal::buffer()
{
	return samples;
}

const Complex * Signal::buffer()const
{
	return samples;
}

void Signal::setMemory(Complex * mem, int len)
{
	ownMemory	= false;
	samples		= mem;
	numSamples	= len;
}




inline void fft_bottomout(Complex * samples, int len)
{
	Complex* s = samples;
	EE_ASSERT(len == 8);
	
	//float rt = 1.0f/Sqrt(2.0f);
	float rt = 0.70710678f;
	Complex scratch[8];
	
	scratch[0] 	= ((s[0] + s[1]) + (s[2] + s[3])) 			
				+ ((s[4] + s[5]) + (s[6] + s[7]));
	scratch[1] 	= ((s[0] - s[1]) + (s[2] - s[3])*Complex(0.0f, -1.0f))
				+ ((s[4] - s[5]) + (s[6] - s[7])*Complex(0.0f, -1.0f))	* Complex(rt, rt);
	scratch[2] 	= ((s[0] + s[1]) - (s[2] + s[3]))
				+ ((s[4] + s[5]) - (s[6] + s[7]))						* Complex(0.0f, -1.0f);
	scratch[3]	= ((s[0] - s[1]) - (s[2] - s[3])*Complex(0.0f, -1.0f))
				+ ((s[4] - s[5]) - (s[6] - s[7])*Complex(0.0f, -1.0f))	* Complex(-rt, -rt);
	scratch[4]  = ((s[0] + s[1]) + (s[2] + s[3]))
				- ((s[4] + s[5]) + (s[6] + s[7]));
	scratch[5]	= ((s[0] - s[1]) + (s[2] - s[3])*Complex(0.0f, -1.0f))
				- ((s[4] - s[5]) + (s[6] - s[7])*Complex(0.0f, -1.0f))	* Complex(rt, rt);
	scratch[6]	= ((s[0] + s[1]) - (s[2] + s[3]))
				- ((s[4] + s[5]) - (s[6] + s[7]))						* Complex(0.0f, -1.0f);
	scratch[7]	= ((s[0] - s[1]) - (s[2] - s[3])*Complex(0.0f, -1.0f))
				- ((s[4] - s[5]) - (s[6] - s[7])*Complex(0.0f, -1.0f))	* Complex(-rt, -rt);
				
	samples[0] = scratch[0];
	samples[1] = scratch[1];
	samples[2] = scratch[2];
	samples[3] = scratch[3];
	samples[4] = scratch[4];
	samples[5] = scratch[5];
	samples[6] = scratch[6];
	samples[7] = scratch[7];				
}


/*--------------------------------------------------------------------------
**
*///------------------------------------------------------------------------
void fft_recurse_opt(Complex * samples, int len, const Complex *twiddleFactors)
{
	if(len == 8)
	{
		fft_bottomout(samples, len);
		return;
	}

	int half 		= len / 2;
	int twiddleStep	= len / 2;
	
	Complex * buffer= scratch+len;
	
	Complex *even	= buffer;
	Complex *odd	= buffer + half;
	
	//since len > 8 && a power of 2, we can move in bundles of 8 safely
	for(int i = 0; i < half; )
	{
		even[i] 	= samples[2*i++];
		odd [i] 	= samples[2*i++];

		even[i] 	= samples[2*i++];
		odd [i] 	= samples[2*i++];

		even[i] 	= samples[2*i++];
		odd [i] 	= samples[2*i++];

		even[i] 	= samples[2*i++];
		odd [i] 	= samples[2*i++];

		even[i] 	= samples[2*i++];
		odd [i] 	= samples[2*i++];

		even[i] 	= samples[2*i++];
		odd [i] 	= samples[2*i++];

		even[i] 	= samples[2*i++];
		odd [i] 	= samples[2*i++];

		even[i] 	= samples[2*i++];
		odd [i] 	= samples[2*i++];
	}

	fft_recurse(even, half);
	fft_recurse(odd, half);
	
	int index = 0;
	int step = 0;
	
	for(int k = 0; k < half; k += 8)
	{
		Complex w 	= twiddleFactors[step];
		Complex e 	= even[index];
		Complex o 	= w * odd[index];
		samples[index] 		= e + o;
		samples[index+half] = e - o;
		index++;
		step += twiddleStep;
		
		w = twiddleFactors[step];
		e = even[index];
		o = w * odd[index];
		samples[index] 		= e + o;
		samples[index+half] = e - o;
		index++;
		step += twiddleStep;
		
		w = twiddleFactors[step];
		e = even[index];
		o = w * odd[index];
		samples[index] 		= e + o;
		samples[index+half] = e - o;
		index++;
		step += twiddleStep;
		
		w = twiddleFactors[step];
		e = even[index];
		o = w * odd[index];
		samples[index] 		= e + o;
		samples[index+half] = e - o;
		index++;
		step += twiddleStep;
		
		w = twiddleFactors[step];
		e = even[index];
		o = w * odd[index];
		samples[index] 		= e + o;
		samples[index+half] = e - o;
		index++;
		step += twiddleStep;
		
		w = twiddleFactors[step];
		e = even[index];
		o = w * odd[index];
		samples[index] 		= e + o;
		samples[index+half] = e - o;
		index++;
		step += twiddleStep;
		
		w = twiddleFactors[step];
		e = even[index];
		o = w * odd[index];
		samples[index] 		= e + o;
		samples[index+half] = e - o;
		index++;
		step += twiddleStep;
		
		w = twiddleFactors[step];
		e = even[index];
		o = w * odd[index];
		samples[index] 		= e + o;
		samples[index+half] = e - o;
		index++;
		step += twiddleStep;
	}
}



/*--------------------------------------------------------------------------
* Thanks, engineering productivity tools, for the tutorial!
*///------------------------------------------------------------------------
Complex * fft_inline_unrolled(Complex *samples, int len, const Complex *twiddleFactors)
{	
	//pre-step - do hardcoded 8 element fft across all of our samples to remove 
	// the first 3 passes (and their iteration)
	for(int oct = 0; oct < len; oct += 8)
	{
		fft_bottomout(samples + oct, 8);
	}
		
	//initialise pass parameters}
	int numPasses		= (int)log2f(float(len)) - 3;
	int numBlocks		= 1<<(numPasses-1);
	int pointsPerBlock	= 16;
	int twiddlestep		= len/16;

	//perform p passes
	for(int pass = 0; pass < numPasses; pass++)
	{ 
		//pass loop
		int numButterflies = pointsPerBlock/2;
				
		int evenIndex = 0;
		for( int block = 0; block < numBlocks; block++)
		{
			//block loop
			int oddIndex = evenIndex + numButterflies;
			
			int eindex = evenIndex;
			int oindex = oddIndex;
			int step = 0;
			
			//since we did all the blocks <= 8, we can move through the butterflies 16 at a time
			for(int k = 0; k < numButterflies; k += 8)
			{
				//butterfly loop				
				Complex e = samples[eindex];
				Complex o = twiddleFactors[step] * samples[oindex];				
				samples [eindex]	= e + o;
				samples [oindex]	= e - o;
				eindex++, oindex++, step += twiddlestep;
				
				e = samples[eindex];
				o = twiddleFactors[step] * samples[oindex];
				samples [eindex]	= e + o;
				samples [oindex]	= e - o;
				eindex++, oindex++, step += twiddlestep;
				
				e = samples[eindex];
				o = twiddleFactors[step] * samples[oindex];
				samples [eindex]	= e + o;
				samples [oindex]	= e - o;
				eindex++, oindex++, step += twiddlestep;
				
				e = samples[eindex];
				o = twiddleFactors[step] * samples[oindex];
				samples [eindex]	= e + o;
				samples [oindex]	= e - o;
				eindex++, oindex++, step += twiddlestep;
				
				e = samples[eindex];
				o = twiddleFactors[step] * samples[oindex];
				samples [eindex]	= e + o;
				samples [oindex]	= e - o;
				eindex++, oindex++, step += twiddlestep;
				
				e = samples[eindex];
				o = twiddleFactors[step] * samples[oindex];
				samples [eindex]	= e + o;
				samples [oindex]	= e - o;
				eindex++, oindex++, step += twiddlestep;
				
				e = samples[eindex];
				o = twiddleFactors[step] * samples[oindex];
				samples [eindex]	= e + o;
				samples [oindex]	= e - o;
				eindex++, oindex++, step += twiddlestep;
				
				e = samples[eindex];
				o = twiddleFactors[step] * samples[oindex];
				samples [eindex]	= e + o;
				samples [oindex]	= e - o;
				eindex++, oindex++, step += twiddlestep;
			}
			
			evenIndex += pointsPerBlock;
		}
		
		// calc parameters for next pass
		numBlocks		/= 2;
		pointsPerBlock	*= 2;
		twiddlestep		/= 2;
	}
	
	return samples;
}



/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void fft_simple_opt(const Complex *samples, int len, RawSpectra* spectra, Envelope * envelope)
{
	EE_ASSERT(isPowerOfTwo(len));
	
	Complex * buffer	= *spectra;
	
	//TODO: don't dynamically allocate this scratch space?
	//scratch	= new Complex[2*len];
	scratch = (Complex*)entropy::Memory_Alloc(2*len*sizeof(Complex));
	
	for(int i = 0; i < len; i++)
	{
		if(envelope)
			buffer[i] = samples[i] * (*envelope)[i];
		else
			buffer[i] = samples[i];
	}
	
	Complex * ws = createTwiddles(len);
	fft_recurse_opt(buffer, len, ws);
	
	//delete[] scratch;
	entropy::Memory_Free(scratch);
	entropy::Memory_Free(ws);
	scratch = NULL;
}

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void fft_simple_opt(const float *samples, int len, RawSpectra * spectra, Envelope * envelope)
{
	EE_ASSERT(isPowerOfTwo(len));
	
	Complex * buffer = (Complex*)entropy::Memory_Alloc(len * sizeof(Complex));
	
	for(int i = 0; i < len; i++)
	{
		Complex temp = Complex(samples[i]);
		buffer[i] = temp;
	}
	
	fft_simple(buffer, len, spectra, envelope);
	
	entropy::Memory_Free(buffer);
}


/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void fft_opt(const Complex * samples, int len, RawSpectra * spectra, Envelope * envelope)
{
	EE_ASSERT(isPowerOfTwo(len));
	EE_ASSERT(len == spectra->len());

	int * bitRemap = createBitRemap(len);
	Complex * ws = createTwiddles(len);
	
	{
		//handle bit reversal
		char * scratch = (char*)alloca(len);
		entropy::Memory_Zero(scratch, len*sizeof(char));
		
		for(int i = 0; i < len; i++)
		{
			int remap = bitRemap[i];
			if(!scratch[i])
			{
				float envi = 1.0;
				float envr = 1.0;
				if(envelope)
				{
					envi	= (*envelope)[i];
					envr	= (*envelope)[remap];
				}
				
				(*spectra)[i]		= samples[remap] * envr;
				(*spectra)[remap]	= samples[i] * envi;
				
				scratch[i] = 1;
				scratch[remap] = 1;
			}
		}
	}
	
	fft_inline_unrolled(spectra->buffer(), len, ws);
	
	entropy::Memory_Free(bitRemap);
	entropy::Memory_Free(ws);
}


/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
void fft_opt(const float * samples, int len, RawSpectra * spectra, Envelope *envelope)
{
	EE_ASSERT(isPowerOfTwo(len));
	Complex * buffer = (Complex*)malloc(sizeof(Complex)* len);
	
	for(int s = 0; s < len; s++)
	{
		buffer[s] = Complex(samples[s]);
	}
	
	fft_opt(buffer, len, spectra, envelope);
	
	entropy::Memory_Free(buffer);
}

/*

int main()
{
	fftSystemTest();
	return 0;
}
*/