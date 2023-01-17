#ifndef MAIN_OUT_PROC_H_INCLUDED
#define MAIN_OUT_PROC_H_INCLUDED

#include "main_out.h"
#include "main_out_conv.h"
#include "main_out_dither.h"

typedef enum
{
    /** The host buffer size is a fixed known size. */
    mrUtilFixedHostBufferSize,
    /** The host buffer size may vary, but has a known maximum size. */
    mrUtilBoundedHostBufferSize,
    /** Nothing is known about the host buffer size. */
    mrUtilUnknownHostBufferSize,
    //paUtilVariableHostBufferSizePartialUsageAllowed
} MRUtilHostBufferSizeMode;

typedef struct MRUtilChannelDescriptor
{
    void *data;
    unsigned int stride;  /**< stride in samples, not bytes */
} MRUtilChannelDescriptor;

typedef struct
{
    unsigned long framesPerUserBuffer;
    unsigned long framesPerHostBuffer;

    MRUtilHostBufferSizeMode hostBufferSizeMode;
    int useNonAdaptingProcess;
    int userOutputSampleFormatIsEqualToHost;
    unsigned long framesPerTempBuffer;

    unsigned int outputChannelCount;
    unsigned int bytesPerHostOutputSample;
    unsigned int bytesPerUserOutputSample;
    int userOutputIsInterleaved;
    MRUtilConverter *outputConverter;
    MRUtilZeroer *outputZeroer;

    unsigned long initialFramesInTempOutputBuffer;

    void *tempOutputBuffer;         /**< used for slips, block adaption, and conversion. */
    void **tempOutputBufferPtrs;    /**< storage for non-interleaved buffer pointers, NULL for interleaved user output */
    unsigned long framesInTempOutputBuffer; /**< frames remaining in input buffer from previous adaption iteration */

    MRStreamCallbackTimeInfo *timeInfo;

    unsigned long callbackStatusFlags; //typedef unsigned long PaStreamCallbackFlags;

    int hostOutputIsInterleaved;
    unsigned long hostOutputFrameCount[2];
    MRUtilChannelDescriptor *hostOutputChannels[2]; /**< pointers to arrays of channel descriptors.
                                                         pointers are NULL for half-duplex input processing.
                                                         hostOutputChannels[i].data is NULL when the caller
                                                         calls PaUtil_SetNoOutput()
                                                         */

    MRUtilTriangularDitherGenerator ditherGenerator;

    double samplePeriod;

    MRStreamCallback *streamCallback;
    void *userData;
} MRUtilBufferProcessor;


int MRUtil_InitializeBufferProcessor(MRUtilBufferProcessor* bp,
            int outputChannelCount, MRSampleFormat userOutputSampleFormat,
            MRSampleFormat hostOutputSampleFormat,
            double sampleRate,
            unsigned long streamFlags,
            unsigned long framesPerUserBuffer, /* 0 indicates don't care */
            unsigned long framesPerHostBuffer,
            MRUtilHostBufferSizeMode hostBufferSizeMode,
            MRStreamCallback *streamCallback, void *userData);

unsigned long MRUtil_GetBufferProcessorOutputLatencyFrames(MRUtilBufferProcessor* bp);

#endif // MAIN_OUT_PROC_H_INCLUDED
