//
//  audio_writers.h
//  audiosample
//
//  Created by Mike Gonzales on 8/21/20.
//

#ifndef audio_writers_h
#define audio_writers_h

#include <stdint.h>

void SinFunction(float * buffer, int32_t numChannels, int32_t numFrames, int32_t hertz, float startTime);

#endif /* audio_writers_h */
