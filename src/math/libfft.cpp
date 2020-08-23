//
//  libfft.cpp
//  fft
//
//  Created by Mike Gonzales on 10/28/14.
//
//
#include "support/native.h"
#if WINDOWS && _DEBUG && MEM_LEAK_CHECKING
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "fft/libfft.h"
#include "fft/fft.h"
#include <string>
#include <stdarg.h>
#include <vector>

#if WINDOWS
    #include <direct.h>
    #define pwd _getcwd
#else
    #include <unistd.h>
    #define pwd getcwd
#endif

const int  strsize = 512;
static char status[strsize] = {0};
static char error [strsize] = {0};

const char * logfile_name = "fft_log.txt";
static bool logging = false;
FILE * logfile = NULL;

//#if WINDOWS

#if LOGGING
/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
void LOGSPRINTF(const char *fmt, ...) 
{
	if(logfile)
	{
		char buf[1024];	//! ATH FIXME should use max command line (8K for xp+; should be a #define somewhere)
	
		va_list argList;
		va_start(argList, fmt);

		int len = vsnprintf(buf, EE_ARRAY_N(buf) - 1, fmt, argList);

		va_end(argList);

		if (len < 0)
			return;

		// surround entire command in quotes to protect args
		buf[len+1] = 0;
	
		fputs(buf,logfile);
		fputs("\n", logfile);
		fflush(logfile);
	}
}
#endif
/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
void ERRSPRINTF(const char *fmt, ...)
{
	va_list argList;
	va_start(argList, fmt);

	int len = vsnprintf(error, EE_ARRAY_N(error) - 1, fmt, argList);

	va_end(argList);

	if (len < 0)
		return;

	// surround entire command in quotes to protect args
	error[len+1] = 0;
}
//#else

//OSX, Linux ...

//#endif

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
void shutdownFFT()
{
#if LOGGING
	LOGSPRINTF("FFT: Closing logfile");
	if(logfile)
		fclose(logfile);
	logfile=  NULL;
#endif	
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
void	initFFTSystem_4k16k(int log)
{
#if MEM_LEAK_CHECKING
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
	_CrtSetBreakAlloc(148);
#endif
	logging = true;//!!logging; 
	initFFTSystem();

#if LOGGING
	if(logging)
	{
		logfile = fopen(logfile_name, "w+");
		if(!logfile)
		{
			char dir[400]= {0};
			pwd(dir, sizeof(dir));
			logging = false;
			ERRSPRINTF("ERROR: FFT::Init() - logging enabled but failed to make logging file %s/%s; turning logging off", dir, logfile_name );
		}
		else
		{
			char dir[400]= {0};
			pwd(dir, sizeof(dir));
			LOGSPRINTF("FFT::Init() - logging enabled in file %s/%s", dir, logfile_name );
			atexit(shutdownFFT);
		}

		LOGSPRINTF("Sizeo of complex %d vs size of Complex %d", sizeof(complex), sizeof(Complex));
	}
#endif	
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------

complex & To(Complex & c)
{
	return *(complex *) &c;
}

Complex & From(complex &c)
{
	return *(Complex*)&c;
}

const complex & To(const Complex & c)
{
	return *(const complex *) &c;
}

const Complex & From( const complex &c)
{
	return *(const Complex*) &c;
}

BEGIN_EXTERN_C

const char * GetError()
{
	return error;
}

void ClearError()
{
	error[0] =0 ;
}

	/*
---	**	- C O M P L E X ----------------------------------------------------------------------
	*/

complex Complex_Add(const complex *l, const complex *r)
{
	complex	ret = To(From(*l) + From(*r));
	return ret;
}

complex Complex_Sub(const complex * l, const complex * r)
{
	complex	ret = To(From(*l) - From(*r));
	return ret;
}

complex Complex_Mul(const complex * l, const complex * r)
{
	complex	ret = To(From(*l) * From(*r));
	return ret;
}

complex Complex_Div(const complex * l, const complex * r)
{
	complex	ret = To(From(*l) / From(*r));
	return ret;
}

complex Complex_Scale(const complex * c, float s)
{
	complex ret = To(From(*c) * s);
	return ret;
}

complex Complex_Conj(const complex *c)
{
	complex ret = To(~From(*c));
	return ret;
}

float	Complex_Mag(const complex *c)
{
	float ret = +From(*c);
	return ret;
}

complex	Complex_Neg(const complex *c)
{
	complex ret = To(-From(*c));
	return ret;
}

	/*
---	**	- R A W - S P E C T R A --------------------------------------------------------------
	*/

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
RawSpectra* 	RawSpectra_Create(int len)
{
	if(!isPowerOfTwo(len))
	{
		ERRSPRINTF("ERROR: RawSpectra::Create() - sample length %d isn't a power of two", len);
		return NULL;
	}

	RawSpectra *raw = new(entropy::Memory_Alloc(sizeof(RawSpectra))) RawSpectra( len);	
	LOGSPRINTF("RawSpectra [0x%p]: Creating raw spectra of length %d", raw, len);
	return raw;
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
RawSpectra* 	RawSpectra_CreatePreallocated(complex * arr, int len)
{
	if(!isPowerOfTwo(len))
	{
		ERRSPRINTF("ERROR: RawSpectra::Create() - preallocated sample length %d isn't a power of two", len);
		return NULL;
	}

	RawSpectra *raw = new(entropy::Memory_Alloc(sizeof(RawSpectra))) RawSpectra((Complex*)arr, len, false);	
	LOGSPRINTF("RawSpectra [0x%p]: Create preallocated (0x%p) raw spectra of length len %d", raw, arr, len);
	return raw; 
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
complex *	RawSpectra_Samples(RawSpectra *raw)
{
	LOGSPRINTF( "RawSpectra [0x%p]: retrieving samples (0x%p)", raw, raw->buffer());
	if(!raw)
	{
		ERRSPRINTF( "ERROR: RawSpectra::Samples() - pointer is null");
		return NULL;
	}
	else
		return (complex*)raw->buffer();
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
int 		RawSpectra_NumSamples(RawSpectra *raw)
{
	LOGSPRINTF("RawSpectra [0x%p]: retrieving length %d", raw, raw->len());
	if(!raw)
	{
		ERRSPRINTF("ERROR: RawSpectra::NumSamples() - pointer is null");
		return -1;
	}
	else
		return raw->len();
}
	
/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
void		RawSpectra_Dispose(RawSpectra *raw)
{
	if(!raw)
		ERRSPRINTF("ERROR: RawSpectra::Dispose() - pointer is null");
	else
	{
		if(raw->ownsMemory())
			LOGSPRINTF("RawSpectra [0x%p]: disposing of raw spectra with dynamically allocated memory (0x%p)", raw, raw->buffer());
		else
			LOGSPRINTF("RawSpectra [0x%p]: disposing of raw spectra with preallocated memory (0x%p)", raw, raw->buffer());
	
		raw->~RawSpectra();
		entropy::Memory_Free(raw);
	}
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
void		RawSpectra_StereoUnpack(RawSpectra *rawIn, RawSpectra * leftOut, RawSpectra * rightOut)
{
	LOGSPRINTF("Raw Spectra [0x%p]: stereo unpacking raw stereo length %d into left (0x%p : %d) and right (0x%p : %d)", rawIn, rawIn->len(), leftOut, leftOut->len(), rightOut, rightOut->len());
	
	if(!rawIn)
		ERRSPRINTF("ERROR: RawSpectra::StereoUnpack() - raw pointer is null");
	if(!leftOut)
		ERRSPRINTF("ERROR: RawSpectra::StereoUnpack() - left pointer is null");
	if(!rightOut)
		ERRSPRINTF("ERROR: RawSpectra::StereoUnpack() - right pointer is null");

	if(rawIn && leftOut && rightOut)
		fft_stereo_unpack(rawIn, leftOut, rightOut);
}

	
	/*
---	**	- S P E C T R A ----------------------------------------------------------------------
	*/

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
Spectra *	Spectra_Create(int len)
{
	if(!isPowerOfTwo(len))
	{
		ERRSPRINTF("ERROR: Spectra::Create() - sample length %d isn't a power of two", len);
		return NULL;
	}

	Spectra * sp = new(entropy::Memory_Alloc(sizeof(Spectra))) Spectra(len);
	LOGSPRINTF("Spectra [0x%p]: Creating spectra of length %d", sp, len);
	return sp;
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
Spectra * Spectra_CreatePreallocated(complex * arr, int len)
{
	if(!isPowerOfTwo(len))
	{
		ERRSPRINTF("ERROR: Spectra::Create() - preallocated sample length %d isn't a power of two", len);
		return NULL;
	}

	Spectra * sp = new(entropy::Memory_Alloc(sizeof(Spectra))) Spectra((Complex*)arr, len);
	LOGSPRINTF("Spectra [0x%p]: creating preallocated spectra (0x%p) of length %d", sp, arr, len);
	return sp;
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
Spectra *	Spectra_CreateFromRaw(RawSpectra *raw)
{
	Spectra * sp = new(entropy::Memory_Alloc(sizeof(Spectra))) Spectra(raw);
	LOGSPRINTF("Spectra [0x%p]: creating spectra from raw spectra (0x%p)", sp, raw);
	return sp;
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
Spectra *	Spectra_CreateFromRawPreallocated(RawSpectra *raw, complex * arr, int len)
{
	if(raw->len() != len)
	{
		Spectra * sp = new(entropy::Memory_Alloc(sizeof(Spectra))) Spectra((Complex*)arr, len);
		sp->process(raw);
		LOGSPRINTF("Spectra [0x%p]: creating preallocated spectra from raw spectra (0x%p)", sp, raw);
		return sp;
	}
	else
	{
		ERRSPRINTF("ERROR: Spectra::CreateFromRawPreallocated(): raw length %d and preallocated length %d don't match", raw->len(), len);
		return NULL;
	}
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
complex	* 	Spectra_Samples(Spectra *spec)
{
	if(spec)
	{
		LOGSPRINTF("Spectra [0x%p]: getting samples buffer (0x%p)", spec, spec->buffer());
		return (complex*)spec->buffer();
	}
	else
	{
		ERRSPRINTF("ERROR: Spectra::GetSamples() - null pointer"); 
		return NULL;
	}
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
int			Spectra_NumSamples(Spectra *spec)
{
	if(spec)
	{
		LOGSPRINTF("Spectra [0x%p]: getting num samples %d", spec, spec->len());
		return spec->len();
	}
	else
	{
		ERRSPRINTF("ERROR: Spectra::NumSamples() - null pointer");
		return -1;
	}
}

	
/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
void Spectra_Dispose(Spectra *spec)
{

	if(!spec)
	{
		ERRSPRINTF("ERROR: Spectra::Dispose() - null pointer");
	}
	else
	{
		if(spec->ownsMemory())
			LOGSPRINTF("Spectra [0x%p]: Disposing of spectra with dynamically allocated memory 0x%p", spec, spec->buffer());
		else
			LOGSPRINTF("Spectra [0x%p]: Disposing of spectra with preallocated memory 0x%p", spec, spec->buffer()); 

		spec->~Spectra();
		entropy::Memory_Free(spec);
	}
}

	/*
---	**	- C H R O M A T I C - S P E C T R A ---------------------------------------------------
	*/


/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
ChromaticSpectra* ChromaticSpectra_StrictProcess(Spectra* spec, float time)
{
	if(spec)
	{
		ChromaticSpectra * chrom = new(entropy::Memory_Alloc(sizeof(ChromaticSpectra))) ChromaticSpectra();
		LOGSPRINTF("ChromaticSpectra [0x%p]: Creating chromatic spectra strict process from spectra (0x%p) w/ time slice %.4f", chrom, spec, time);
		chrom->strictProcess(spec, time);
		return chrom;
	}
	else
	{
		ERRSPRINTF("ERROR: ChromaticSpectra:StrictProcess() - null spectra passed in ");
		return NULL;
	}
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
ChromaticSpectra * ChromaticSpectra_BandProcess(Spectra *spec, float time)
{
	if(spec)
	{
		ChromaticSpectra * chrom = new(entropy::Memory_Alloc(sizeof(ChromaticSpectra))) ChromaticSpectra();
		LOGSPRINTF("ChromaticSpectra [0x%p]: Creating chromatic spectra band process from spectra (0x%p) w/ time slice %.4f", chrom, spec, time);
		chrom->bandProcess(spec, time);
		return chrom;
	}
	else
	{
		ERRSPRINTF("ERROR: ChromaticSpectra:BandProcess() - null spectra passed in");
		return NULL;
	}
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
ChromaticSpectra *ChromaticSpectra_StrictProcessStereo(Spectra *left, Spectra *right, float time)
{
	ChromaticSpectra * chrom = new(entropy::Memory_Alloc(sizeof(ChromaticSpectra))) ChromaticSpectra();
	LOGSPRINTF("ChromaticSpectra [0x%p]: Creating chromatic spectra stereo strict process from left (0x%p) and right (0x%p) w/ time slice %.4f", chrom, left, right, time);
	chrom->strictProcess(left,right, time);
	return chrom;
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
ChromaticSpectra *ChromaticSpectra_BandProcessStereo(Spectra *left, Spectra *right, float time)
{
	ChromaticSpectra * chrom = new(entropy::Memory_Alloc(sizeof(ChromaticSpectra))) ChromaticSpectra();
	LOGSPRINTF("ChromaticSpectra [0x%p]: Creating chromatic spectra stereo band process from left (0x%p) and right (0x%p) w/ time slice %.4f", chrom, left, right, time);
	chrom->bandProcess(left, right, time);
	return chrom;
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
int			ChromaticSpectra_Peak(ChromaticSpectra *chrom)
{
	if(chrom)
	{
		LOGSPRINTF("ChromaticSpectra [0x%p]: peak index %d", chrom, chrom->getPeakIndex());
		return chrom->getPeakIndex();
	}
	else
	{
		ERRSPRINTF("ERROR: ChromaticSpectra::Peak() - null pointer");
		return -1;
	}
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
float			ChromaticSpectra_Max(ChromaticSpectra *chrom)
{
	if(chrom)
	{
		LOGSPRINTF("ChromaticSpectra [0x%p]: peak value %.2f", chrom, chrom->peakValue);
		return chrom->peakValue;
	}
	else
	{
		ERRSPRINTF("ERROR: ChromaticSpectra::Max() - null pointer");
		return -1;
	}
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
int			ChromaticSpectra_NumSpectra(ChromaticSpectra *chrom)
{
	
	if(chrom)
	{
		LOGSPRINTF("ChromaticSpectra [0x%p]: num spectra %d", chrom, chrom->getNumSpectra());
		return chrom->getNumSpectra();
	}
	else
	{
		return ChromaticSpectra::kNumNotes;
		/*
		ERRSPRINTF("ERROR: ChromaticSpectra::NumSpectra() - null pointer");
		return -1;*/
	}
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
float *		ChromaticSpectra_Spectra(ChromaticSpectra *chrom)
{
	if(chrom)
	{
		LOGSPRINTF("ChromaticSpectra [0x%p]: spectra (0x%p)", chrom, chrom->getSpectra());
		return chrom->getSpectra();
	}
	else
	{		
		ERRSPRINTF("ERROR: ChromaticSpectra::Spectra() - null pointer");
		return NULL;	
	}
}


/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
int			ChromaticSpectra_NumBands(ChromaticSpectra *chrom)
{
	if(chrom)
	{
		LOGSPRINTF("ChromaticSpectra [0x%p]: num bands %d", chrom, chrom->getNumBands());
		return chrom->getNumBands();
	}
	else
	{
		return ChromaticSpectra::kBands;
	/*
		ERRSPRINTF("ERROR: ChromaticSpectra::NumBands() - null pointer");
		return -1;
		*/
	}
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
float *		ChromaticSpectra_Bands(ChromaticSpectra * chrom)
{
	if(chrom)
	{
		LOGSPRINTF("ChromaticSpectra [0x%p]: bands (0x%p)", chrom, chrom->getBands());
		return chrom->getBands();
	}
	else
	{
		ERRSPRINTF("ERROR: ChromaticSpectra::Bands() - null pointer");
		return NULL;
	}
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
float *		ChromaticSpectra_Frequencies()
{
	return ChromaticSpectra::getFrequencies();
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
void		ChromaticSpectra_SetA5(float value)
{
	ChromaticSpectra::setA5Frequency(value);
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
float		ChromaticSpectra_GetA5()
{
	return ChromaticSpectra::getA5Frequency();
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
void		ChromaticSpectra_Dispose(ChromaticSpectra *csp)
{
	if(csp)
	{
		LOGSPRINTF("ChromaticSpectra [0x%p]: Disposing of Chromatic spectra", csp);
		//Chromatic Spectra doesn't allocate any memory dynamically, as it is a hardcoded array
		entropy::Memory_Free(csp);
	}
	else
	{
		ERRSPRINTF("ERROR: ChromaticSpectra::Dispose() - null pointer");
	}
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
ChromaticSpectra *ChromaticSpectra_DifferenceProcess(Spectra *n1, Spectra *n0, float time)
{
	ChromaticSpectra * ret = new(entropy::Memory_Alloc(sizeof(ChromaticSpectra))) ChromaticSpectra();
	ret->deltaProcess(n1, n0, time);
	return ret;
}

	/*
---	** -------------------------------------------------------------------------------------
	*/

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
BeatSpectra * BeatSpectra_Create()
{
	BeatSpectra * beat = new(entropy::Memory_Alloc(sizeof(BeatSpectra))) BeatSpectra();
	LOGSPRINTF("BeatSpectra [0x%p]: Creating beat spectra w/ time", beat);
		
	return beat;
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
void			BeatSpectra_Process(BeatSpectra *bt, ChromaticSpectra * sp)
{
	LOGSPRINTF("BeatSpectra [0x%p]: Adding information from 0x%p to the mix", bt, sp);
	bt->Process(*sp);
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
ChromaticSpectra * BeatSpectra_GetBeats(BeatSpectra *bt)
{
	LOGSPRINTF("BeatSpectra [0x%p]: getting sub chromatic spectra at 0x%p", bt, &bt->beat);
	return &bt->beat;
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
ChromaticSpectra * BeatSpectra_GetEnergies(BeatSpectra *bt)
{
	LOGSPRINTF("BeatSpectra [0x%p]: getting energy spectra at 0x%p", bt, &bt->energy);
	return &bt->energy;
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
ChromaticSpectra * BeatSpectra_GetVariances(BeatSpectra *bt)
{
	LOGSPRINTF("BeatSpectra [0x%p]: getting variance spectra at 0x%p", bt, &bt->variance);
	return &bt->variance;
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
void			BeatSpectra_Dispose(BeatSpectra *bt)
{
	LOGSPRINTF("BeatSpectra [0x%p]: disposing");
	entropy::Memory_Free(bt);
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
int				BeatSpectra_Ready(BeatSpectra *bt)
{
	LOGSPRINTF("BeatSpectra [0x%p]: quearying ready status %s", bt->ready()? "true": "false");
	if(bt->ready()) return 1;
	return 0;
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
int			BeatSpectra_NumHistories(BeatSpectra *bt)
{
	return BeatSpectra::kHistory;
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
float *		BeatSpectra_BeatSpectraHistory(BeatSpectra *bt, int index)
{
	return bt->history[index].spectra;
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
float *		BeatSpectra_BeatBandHistory(BeatSpectra *bt, int index)
{
	return bt->history[index].bands;
}

int			BeatSpectra_NumSpectra(ChromaticSpectra *csp)
{
	return ChromaticSpectra::kNumNotes;
}

int			BeatSpectra_NumBands(ChromaticSpectra *csp)
{
	return ChromaticSpectra::kBands;
}


	/*
---	** -------------------------------------------------------------------------------------
	*/

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
int 		Signal_NumSamples(Signal *sig)
{
	if(sig)
	{
		LOGSPRINTF("Signal [0x%p]: num samples %d", sig, sig->len());
		return sig->len();
	}
	else
	{
		ERRSPRINTF("ERROR: Signal::NumSamples() - null pointer");
		return -1;
	}
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
complex *	Signal_Samples(Signal *sig)
{
	if(sig)
	{
		LOGSPRINTF("Signal [0x%p]: samples buffer (0x%p)", sig, sig->buffer());
		return (complex*)sig->buffer();
	}
	else
	{
		ERRSPRINTF("ERROR: Signal::Samples() - null pointer");
		return NULL;
	}
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
void		Signal_Dispose(Signal *sig)
{
	if(sig)
	{
		LOGSPRINTF("Signal [0x%p]: disposing", sig);
		sig->~Signal();
		entropy::Memory_Free(sig);
	}
	else
	{
		ERRSPRINTF("ERROR: Signal::Dispose() - null pointer");
	}
}

	/*
---	** -------------------------------------------------------------------------------------
	*/

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
Envelope *	Envelope_CreateCustom(int len)
{
	if(!isPowerOfTwo(len))
	{
		ERRSPRINTF("ERROR: Envelope::CreateCustom() - sample length %d isn't a power of two", len);
		return NULL;
	}

	SampleEnvelope * env = new (entropy::Memory_Alloc(sizeof(SampleEnvelope))) SampleEnvelope(len);
	LOGSPRINTF("Envelope [0x%p]: Create custom (0x%p) with len %d", env, env->buffer(), len );
	return env;
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
float * Envelope_CustomSamples(Envelope *e)
{
	SampleEnvelope * env = (SampleEnvelope*)e;
	LOGSPRINTF("Envelope [0x%p]: custom samples (0x%p)", env, env->buffer());
	return env->buffer();
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
int Envelope_CustomNumSamples(Envelope * e)
{
	SampleEnvelope * env = (SampleEnvelope*)e;
	LOGSPRINTF("Envelope [0x%p]: Retrieving custom num samples %d", env, env->getLen());
	return env->getLen();
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
Envelope *	Envelope_CreateHamming(int len)
{
	if(!isPowerOfTwo(len))
	{
		ERRSPRINTF("ERROR: Envelope::CreateHamming() - sample length %d isn't a power of two", len);
		return NULL;
	}

	Hamming *ham = new (entropy::Memory_Alloc(sizeof(Hamming))) Hamming(len);
	LOGSPRINTF("Envelope [0x%p]: Create hamming with len %d:0x%p", ham, len, len );
	return ham;
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
Envelope *	Envelope_CreateHamming4k()
{
	Hamming4k *ham = new (entropy::Memory_Alloc(sizeof(Hamming4k))) Hamming4k();
	LOGSPRINTF("Envelope [0x%p]: Create hamming 4k (0x%p)", ham, ham->samples);
	return ham;
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
Envelope * 	Envelope_CreateHamming16k()
{
	Hamming16k *ham = new (entropy::Memory_Alloc(sizeof(Hamming16k))) Hamming16k();
	LOGSPRINTF("Envelope [0x%p]: Create hamming 16k (0x%p)", ham, ham->samples );
	return ham;
}

/*----------------------------------------------------------------------------
*///--------------------------------------------------------------------------
void		Envelope_Dispose(Envelope *env)
{
	if(env)
	{
		LOGSPRINTF("Envelope [0x%p]: disposing", env);
		env->~Envelope();
		entropy::Memory_Free(env);
	}
	else
	{
		ERRSPRINTF("ERROR: Envelope::Dispose - null pointer");
	}
}

	/*
---	** -------------------------------------------------------------------------------------
	*/

//for any power of two
void	fft_mono_float	(const float * samples, int len, RawSpectra *spectraOut, Envelope * env)
{
	LOGSPRINTF("FFT: Mono samples: 0x%p of len %d; RawSpectra out 0x%p with Envelope 0x%p", samples, len, spectraOut, env);
	fft(samples, len, spectraOut, env);
}

void	fft_mono_complex(const complex * samples, int len, RawSpectra * spectraOut, Envelope * env)
{
	LOGSPRINTF("FFT: Mono complex samples: 0x%p of len %d; RawSpectra out 0x%p with Envelope 0x%p", samples, len, spectraOut, env);
	fft((const Complex*)samples, len, spectraOut, env);
}

void	ifft_mono		(const RawSpectra * spectra, Signal * signalOut)
{
	LOGSPRINTF("IFFT: Mono RawSpectra 0x%p with Signal out 0x%p", spectra, signalOut);
	ifft(spectra, signalOut); 
}

//for hardcoded 4k and 16k most common use cases
void	fft16k_mono		(const float samples[16*1024], RawSpectra *spectraOut, Envelope *env)
{
	LOGSPRINTF("FFT 16K: Mono samples 0x%p with RawSpectra out 0x%p and envelope 0x%p", samples, spectraOut, env);
	fft16k(samples, spectraOut, env);
}

void	ifft16k_mono	(const RawSpectra * spectra, Signal * signalOut)
{
	LOGSPRINTF("IFFT 16k: Mono RawSpectra 0x%p with Signal out 0x%p", spectra, signalOut );
	ifft16k(spectra, signalOut);
}

void	fft4k_mono		(const float samples[4*1024], RawSpectra *spectraOut, Envelope *env)
{
	LOGSPRINTF("FFT 4k: Mono samples 0x%p with RawSpectra out 0x%p and Envelope 0x%p", samples, spectraOut, env );
	fft4k(samples, spectraOut, env);
}

void	ifft4k_mono		(const RawSpectra * spectra, Signal *signalOut)
{
	LOGSPRINTF("IFFT 4k: Mono RawSpectra 0x%p with Signal out 0x%p", spectra, signalOut);
	ifft4k(spectra, signalOut);
}

//void	fft_chromatic(const float *samples, int len, float time, ChromaticSpectra* spectraOut, Envelope * envelope = NULL);

//convert signal back to raw samples
void	ifft_finalize_mono(const Signal *signal, float * samplesOut)
{
	LOGSPRINTF("IFFT Finalize: Signal 0x%p with float samples out", signal, samplesOut);
	ifft_finalize(signal, samplesOut);
}

//stereo routines that leverage above; when the routine takes or returns a single value, it is an interleaved value
void	fft_stereo_2in1out	(const float * left, const float *right, int len, RawSpectra* out, Envelope * env)
{
	LOGSPRINTF("FFT: Stereo with samples left 0x%p, right 0x%p of len %d, RawSpectra out 0x%p and Envelope 0x%p", left, right, len, out, env);
	fft_stereo(left, right, len, out, env);
}

void	fft_stereo_2in2out	(const float * left, const float *right, int len, RawSpectra* leftOut, RawSpectra* rightOut, Envelope * env)
{
	LOGSPRINTF("FFT: Stereo with samples left 0x%p, right 0x%p of len %d with RawSpectra left 0x%p, right 0x%p and Envelope 0x%p", left, right, len, leftOut, rightOut, env);
	fft_stereo(left, right, len, leftOut, rightOut, env);
}

void	fft_stereo_1in1out	(const float * interleaved, int numFrames, RawSpectra *out, Envelope * env)
{
	LOGSPRINTF("FFT: Stereo interleaved samples 0x%p of len %d with RawSpectra out 0x%p and Envelope 0x%p", interleaved, numFrames, out, env);
	fft_stereo(interleaved, numFrames, out, env);
}

void	fft_stereo_1in2out	(const float * interleaved, int numFrames, RawSpectra * leftOut, RawSpectra * rightOut, Envelope * env)
{
	LOGSPRINTF("FFT: Stereo interleaved samples 0x%p of len %d with RawSpectra out left 0x%p, right 0x%p and Envelope 0x%p", interleaved, numFrames, leftOut, rightOut, env);
	fft_stereo(interleaved, numFrames, leftOut, rightOut, env);
}

void	ifft_stereo_1out	(const RawSpectra* in, Signal * out)
{
	LOGSPRINTF("IFFT: Stereo with Rawspectra in 0x%p and Signal 0x%p out", in, out);
	ifft_stereo(in, out);
}

void	ifft_stereo_2out	(const RawSpectra * left, const RawSpectra * right, Signal * out)
{
	LOGSPRINTF("IFFT: Stereo with RawSpectra in left 0x%p, right 0x%p and Signal out 0x%p", left, right, out);
	ifft_stereo(left,right , out);
}

void	fft16k_stereo_2in1out(const float left[16*1024], const float right[16*1024], RawSpectra * spectraOut, Envelope * env)
{
	LOGSPRINTF("FFT 16k: Stereo with samples left 0x%p, right 0x%p RawSpectra out 0x%p and Envelope 0x%p", left, right, spectraOut, env);
	fft16k_stereo(left, right, spectraOut, env);
}

void	fft16k_stereo_2in2out(const float left[16*1024], const float right[16*1024], RawSpectra * leftOut, RawSpectra* rightOut, Envelope * env)
{
	LOGSPRINTF("FFT 16k: Stereo with samples left 0x%p, right 0x%p RawSpectra out left 0x%p, right 0x%p and Envelope 0x%p", left, right, leftOut, rightOut, env);
	fft16k_stereo(left, right, leftOut, rightOut, env);
}

void	fft16k_stereo_1in1out(const float interleaved[2*16*1024], RawSpectra * spectraOut, Envelope * env)
{
	LOGSPRINTF("FFT 16k: Stereo with interleaved 0x%p and RawSpectra out 0x%p and Envelope 0x%p", interleaved, spectraOut, env);
	fft16k_stereo(interleaved, spectraOut, spectraOut, env);
}

void	fft16k_stereo_1in2out(const float interleaved[2*16*1024], RawSpectra * leftOut, RawSpectra* rightOut, Envelope * env)
{
	LOGSPRINTF("FFT 16k: Stereo with interleaved 0x%p and RawSpectra out left 0x%p, right 0x%p and Envelope 0x%p", interleaved, leftOut, rightOut, env);
	fft16k_stereo(interleaved, leftOut, rightOut, env);
}

void	fft4k_stereo_2in1out(const float left[4*1024], const float right[4*1024], RawSpectra *spectraOut, Envelope * env)
{
	LOGSPRINTF("FFT 4k: Stereo samples left 0x%p, right 0x%p with RawSpectra out 0x%p and Envelope 0x%p", left, right, spectraOut, env);
	fft4k_stereo(left, right, spectraOut, env);
}

void	fft4k_stereo_2in2out(const float left[4*1024], const float right[4*1024], RawSpectra *leftOut, RawSpectra * rightOut, Envelope * env)
{
	LOGSPRINTF("FFT 4k: Stereo samples left 0x%p, right 0x%p and RawSpectra out left 0x%p, right 0x%p and Envelope 0x%p", left, right, leftOut, rightOut, env);
	fft4k_stereo(left, right, leftOut, rightOut, env);
}

void	fft4k_stereo_1in1out(const float interleaved[2*4*1024], RawSpectra *spectraOut, Envelope * env)
{
	LOGSPRINTF("FFT 4k: Stereo interleaved 0x%p with RawSpectra out 0x%p and Envelope 0x%p", interleaved, spectraOut, env);
	fft4k_stereo(interleaved, spectraOut, env);
}

void	fft4k_stereo_1in2out(const float interleaved[2*4*1024], RawSpectra *leftOut, RawSpectra * rightOut, Envelope * env)
{
	LOGSPRINTF("FFT 4k: Stereo interleaved 0x%p with RawSpectra out left 0x%p, right 0x%p and Envelope 0x%p", interleaved, leftOut, rightOut, env);
	fft4k_stereo(interleaved, leftOut, rightOut, env);
}

void	ifft16k_stereo_1out	(const RawSpectra * spectra, Signal * signalOut)
{
	LOGSPRINTF("IFFT 16k: RawSpectra in 0x%p and Signal out 0x%p", spectra, signalOut);
	ifft16k_stereo(spectra, signalOut);
}

void	ifft16k_stereo_2out	(const RawSpectra * left, const RawSpectra * right, Signal * signalOut)
{
	LOGSPRINTF("IFFT 16k: RawSpectra left 0x%p, right 0x%p and Signal out 0x%p", left, right, signalOut);
	ifft16k_stereo(left, right, signalOut);
}

void	ifft4k_stereo_1out	(const RawSpectra * spectra, Signal *signalOut)
{
	LOGSPRINTF("IFFT 4k: Stereo RawSpectra in 0x%p and Signal out 0x%p", spectra, signalOut);
	ifft4k_stereo(spectra, signalOut);
}

void	ifft4k_stereo_2out	(const RawSpectra * left, const RawSpectra* right, Signal *signalOut)
{
	LOGSPRINTF("IFFT 4k: Stereo RawSpectra left 0x%p, right 0x%p and Signal out 0x%p", left, right, signalOut);
	ifft4k_stereo(left, right, signalOut);
}

//convert stereo signal back to raw samples
void	ifft_finalize_stereo_2out(const Signal * signal, float *left, float *right)
{
	LOGSPRINTF("IFFT: Stereo Finalize with Signal 0x%p and sample buffers left 0x%p, right 0x%p", signal, left, right);
	ifft_stereofinalize(signal, left, right);
}

void	ifft_finalize_stereo_1out(const Signal * signal, float *interleaved)
{
	LOGSPRINTF("IFFT: Stereo Finalize with Signal 0x%p and sample interleaved buffer 0x%p", signal, interleaved);
	ifft_stereofinalize(signal, interleaved);
}

int aclen = 0;
std::vector<float *> aclist;
std::vector<float> acgains;
float * acout = NULL;

void		prefft_addsamples(float * samples, int len, float gain)
{
	if(aclen == 0)
		aclen = len;
	else
	{
		if(aclen != len)
		{
			ERRSPRINTF("ERROR: Pre-FFT: accumulation buffer of len %d doesn't match previous buffer's len of %d", len, aclen);
			return;
		}
	}

	aclist.push_back(samples);
	acgains.push_back(gain);
}

void		prefft_setoutput(float * samples, int len)
{
	if(aclen == 0)
		aclen = len;
	else
	{
		if(aclen != len)
		{
			ERRSPRINTF("ERROR: Pre-FFT: output buffer of len %d doesn't match previous buffer's len of %d", len, aclen);
			return;
		}
	}

	acout = samples;
}


void prefft_batchaccumulate()
{
	if(aclen == 0 || aclist.size() == 0)
	{
		ERRSPRINTF("ERROR: pre-FFT sample accumulate - sample buffers not set");
		return;
	}

	if(!acout)
	{
		ERRSPRINTF("ERROR: pre-FFT sample accumulate - output buffer not set");
		return;
	}
	int num = (int)aclist.size();

#if WINDOWS
	float** list = (float**)alloca(sizeof(float*)*num);
	float* gains = (float*)alloca(sizeof(float)*num);
#else
	//OSX, Linux
	float** list = (float**)alloca(sizeof(float*)*num);
	float* gains = (float*)alloca(sizeof(float)*num);
#endif

	//we're going to be hitting the aclist vector a lot, so just
	// cache the pointers here on the stack and avoid going through
	// a function call for every dereference
	for(int i = 0 ; i < num; i++)
	{
		list[i] = aclist[i];
		gains[i] = acgains[i];
	}

	for(int i = 0; i < aclen; i++)
	{
		acout[i] = list[0][i] * gains[0];
		for(int j=1; j < num; j++)
		{
			acout[i] += list[j][i] * gains[j];
		}
	}

	acout = NULL;
	aclen = 0;
	aclist.clear();
	acgains.clear();
}

void		prefft_stereoizemono(float * mono, int monolen, float * stereo)
{
	for(int c = monolen-1; c >= 0; c--)
	{
		stereo[2*c+1] = mono[c];
		stereo[2*c] = mono[c];
	}
}

void		prefft_monoizestereo(float *stereo, int stereolen, float *mono)
{
	for(int c = 0; c < stereolen; c+=2)
	{
		mono[c/2] = (stereo[c] + stereo[c+1])/2.0f;
	}
}

END_EXTERN_C
