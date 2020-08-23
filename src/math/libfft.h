//
//  libfft.h
//  fft
//
//  Created by Mike Gonzales on 10/28/14.
//
//

#ifndef _PLUGIN_LIBFFT_H_
#define _PLUGIN_LIBFFT_H_

#include "../support/native.h"


BEGIN_EXTERN_C

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
struct complex
{
 	float r, i;
};

DLLEXPORT const char * GetError();
DLLEXPORT void ClearError();

DLLEXPORT complex Complex_Add(const complex *l, const complex *r);
DLLEXPORT complex Complex_Sub(const complex * l, const complex * r);
DLLEXPORT complex Complex_Mul(const complex * l, const complex * r);
DLLEXPORT complex Complex_Div(const complex * l, const complex * r);

DLLEXPORT complex Complex_Scale(const complex * c, float s);

DLLEXPORT complex Complex_Conj(const complex *c);
DLLEXPORT float	Complex_Mag(const complex *c);
DLLEXPORT complex	Complex_Neg(const complex *c);

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
class RawSpectra;

DLLEXPORT RawSpectra*	RawSpectra_Create(int len);
DLLEXPORT RawSpectra* 	RawSpectra_CreatePreallocated(complex * arr, int len);
DLLEXPORT complex *		RawSpectra_Samples(RawSpectra *raw);
DLLEXPORT int 			RawSpectra_NumSamples(RawSpectra *raw);
DLLEXPORT void			RawSpectra_Dispose(RawSpectra *raw);
DLLEXPORT void			RawSpectra_StereoUnpack(RawSpectra *rawIn, RawSpectra * leftOut, RawSpectra * rightOut); 

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
class Spectra;

DLLEXPORT Spectra *		Spectra_Create(int len);
DLLEXPORT Spectra *		Spectra_CreatePreallocated(complex * arr, int len);
DLLEXPORT Spectra *		Spectra_CreateFromRaw(RawSpectra *raw);
DLLEXPORT Spectra *		Spectra_CreateFromRawPreallocated(RawSpectra *raw, complex * arr, int len);
DLLEXPORT complex	* 	Spectra_Samples(Spectra *sp);
DLLEXPORT int			Spectra_NumSamples(Spectra *sp);
DLLEXPORT void			Spectra_Dispose(Spectra *sp);

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
class ChromaticSpectra;

DLLEXPORT ChromaticSpectra *ChromaticSpectra_DifferenceProcess(Spectra *n1, Spectra *n0, float time);
DLLEXPORT ChromaticSpectra *ChromaticSpectra_StrictProcess(Spectra *sp, float time);
DLLEXPORT ChromaticSpectra *ChromaticSpectra_BandProcess(Spectra *sp, float time);

DLLEXPORT ChromaticSpectra *ChromaticSpectra_StrictProcessStereo(Spectra* l, Spectra* r, float time);
DLLEXPORT ChromaticSpectra *ChromaticSpectra_BandProcessStereo(Spectra* l, Spectra* r, float time);

DLLEXPORT int			ChromaticSpectra_Peak(ChromaticSpectra *csp);
DLLEXPORT float			ChromaticSpectra_Max(ChromaticSpectra *csp);
DLLEXPORT int			ChromaticSpectra_NumSpectra(ChromaticSpectra *csp);
DLLEXPORT float *		ChromaticSpectra_Spectra(ChromaticSpectra *csp);
DLLEXPORT int			ChromaticSpectra_NumBands(ChromaticSpectra *csp);
DLLEXPORT float *		ChromaticSpectra_Bands(ChromaticSpectra * csp);
DLLEXPORT float *		ChromaticSpectra_Frequencies();
DLLEXPORT void			ChromaticSpectra_SetA5(float value);
DLLEXPORT float			ChromaticSpectra_GetA5();
DLLEXPORT void			ChromaticSpectra_Dispose(ChromaticSpectra *csp);


/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
class BeatSpectra;

DLLEXPORT BeatSpectra * BeatSpectra_Create();
DLLEXPORT void			BeatSpectra_Process(BeatSpectra *bt, ChromaticSpectra * sp);
DLLEXPORT ChromaticSpectra * BeatSpectra_GetBeats(BeatSpectra *bt);
DLLEXPORT ChromaticSpectra * BeatSpectra_GetEnergies(BeatSpectra *bt);
DLLEXPORT ChromaticSpectra * BeatSpectra_GetVariances(BeatSpectra *bt);
DLLEXPORT void			BeatSpectra_Dispose(BeatSpectra *bt);
DLLEXPORT int			BeatSpectra_Ready(BeatSpectra *bt);
DLLEXPORT int			BeatSpectra_NumHistories(BeatSpectra *bt);
DLLEXPORT float *		BeatSpectra_BeatSpectraHistory(BeatSpectra *bt, int index);
DLLEXPORT int			BeatSpectra_NumSpectra(ChromaticSpectra *csp);
DLLEXPORT float *		BeatSpectra_BeatBandHistory(BeatSpectra *bt, int index);
DLLEXPORT int			BeatSpectra_NumBands(ChromaticSpectra *csp);

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
class Signal;

DLLEXPORT int 			Signal_NumSamples(Signal *s);
DLLEXPORT complex *		Signal_Samples(Signal *s);
DLLEXPORT void			Signal_Dispose(Signal *s);

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------

class Envelope;

DLLEXPORT Envelope * 	Envelope_CreateCustom(int len);
DLLEXPORT float * 		Envelope_CustomSamples(Envelope *e);
DLLEXPORT int			Envelope_CustomNumSamples(Envelope *e);

DLLEXPORT Envelope *	Envelope_CreateHamming(int len);
DLLEXPORT Envelope *	Envelope_CreateHamming4k();
DLLEXPORT Envelope * 	Envelope_CreateHamming16k();
DLLEXPORT void			Envelope_Dispose(Envelope *e);

/*--------------------------------------------------------------------------
*///------------------------------------------------------------------------
DLLEXPORT void	initFFTSystem_4k16k(int debugLogging);

//for any power of two
DLLEXPORT void	fft_mono_float	(const float * samples, int len, RawSpectra *spectraOut, Envelope * env);
DLLEXPORT void	fft_mono_complex(const complex * samples, int len, RawSpectra * spectraOut, Envelope * env);
DLLEXPORT void	ifft_mono		(const RawSpectra * spectra, Signal * signalOut);

//for hardcoded 4k and 16k most common use cases
DLLEXPORT void	fft16k_mono		(const float samples[16*1024], RawSpectra *spectraOut, Envelope *envelope);
DLLEXPORT void	ifft16k_mono	(const RawSpectra * spectra, Signal * signalOut);
DLLEXPORT void	fft4k_mono		(const float samples[4*1024], RawSpectra *spectraOut, Envelope *envelope);
DLLEXPORT void	ifft4k_mono		(const RawSpectra * spectra, Signal *signalOut);

//DLLEXPORT void	fft_chromatic(const float *samples, int len, float time, ChromaticSpectra* spectraOut, Envelope * envelope = NULL);

//convert signal back to raw samples
DLLEXPORT void	ifft_finalize_mono(const Signal *signal, float * samplesOut);

//stereo routines that leverage above; when the routine takes or returns a single value, it is an interleaved value
DLLEXPORT void	fft_stereo_2in1out	(const float * left, const float *right, int len, RawSpectra* out, Envelope * envelope);
DLLEXPORT void	fft_stereo_2in2out	(const float * left, const float *right, int len, RawSpectra* leftOut, RawSpectra* rightOut, Envelope * envelope);
DLLEXPORT void	fft_stereo_1in1out	(const float * interleaved, int numFrames, RawSpectra *out, Envelope * envelope);
DLLEXPORT void	fft_stereo_1in2out	(const float * interleaved, int numFrames, RawSpectra * leftOut, RawSpectra * rightOut, Envelope * env);

DLLEXPORT void	ifft_stereo_1out	(const RawSpectra* left, Signal * out);
DLLEXPORT void	ifft_stereo_2out	(const RawSpectra * left, const RawSpectra * right, Signal * out);

DLLEXPORT void	fft16k_stereo_2in1out(const float left[16*1024], const float right[16*1024], RawSpectra * spectraOut, Envelope * envelope);
DLLEXPORT void	fft16k_stereo_2in2out(const float left[16*1024], const float right[16*1024], RawSpectra * leftOut, RawSpectra* rightOut, Envelope * envelope);
DLLEXPORT void	fft16k_stereo_1in1out(const float interleaved[2*16*1024], RawSpectra * spectraOut, Envelope * envelope);
DLLEXPORT void	fft16k_stereo_1in2out(const float interleaved[2*16*1024], RawSpectra * leftOut, RawSpectra* rightOut, Envelope * envelope);

DLLEXPORT void	fft4k_stereo_2in1out(const float left[4*1024], const float right[4*1024], RawSpectra *spectraOut, Envelope * envelope);
DLLEXPORT void	fft4k_stereo_2in2out(const float left[4*1024], const float right[4*1024], RawSpectra *leftOut, RawSpectra * rightOut, Envelope * envelope);
DLLEXPORT void	fft4k_stereo_1in1out(const float interleaved[2*4*1024], RawSpectra *spectraOut, Envelope * envelope);
DLLEXPORT void	fft4k_stereo_1in2out(const float interleaved[2*4*1024], RawSpectra *leftOut, RawSpectra * rightOut, Envelope * envelope);


DLLEXPORT void	ifft16k_stereo_1out	(const RawSpectra * spectra, Signal * signalOut);
DLLEXPORT void	ifft16k_stereo_2out	(const RawSpectra * left, const RawSpectra * right, Signal * signalOut);

DLLEXPORT void	ifft4k_stereo_1out	(const RawSpectra * spectra, Signal *signalOut);
DLLEXPORT void	ifft4k_stereo_2out	(const RawSpectra * left, const RawSpectra* right, Signal *signalOut);

//convert stereo signal back to raw samples
DLLEXPORT void	ifft_finalize_stereo_2out(const Signal * signal, float *left, float *right);
DLLEXPORT void	ifft_finalize_stereo_1out(const Signal * signal, float *interleaved);

//I am told that math is super slow in C#, so implement this in C to be fast
DLLEXPORT void		prefft_addsamples(float * samples, int len, float gain);
DLLEXPORT void		prefft_setoutput(float * samples, int len);
DLLEXPORT void		prefft_batchaccumulate();
DLLEXPORT void		prefft_samplesdispose(float * samples);  
DLLEXPORT void		prefft_stereoizemono(float * mono, int monolen, float * stereo);
DLLEXPORT void		prefft_monoizestereo(float *stereo, int stereolen, float *mono);
END_EXTERN_C

#endif 
