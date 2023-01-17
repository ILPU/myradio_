#include "main_out_proc.h"

#define MR_FRAMES_PER_TEMP_BUFFER_WHEN_HOST_BUFFER_SIZE_IS_UNKNOWN_    1024

/* greatest common divisor - PGCD in French */
static unsigned long GCD(unsigned long a, unsigned long b)
{
    return (b==0) ? a : GCD( b, a%b);
}
/* least common multiple - PPCM in French */
static unsigned long LCM(unsigned long a, unsigned long b)
{
    return (a*b) / GCD(a,b);
}

#define MR_MAX_( a, b ) (((a) > (b)) ? (a) : (b))

static unsigned long CalculateFrameShift(unsigned long M, unsigned long N)
{
    unsigned long result = 0;
    unsigned long i;
    unsigned long lcm;

    lcm = LCM( M, N );
    for( i = M; i < lcm; i += M )
        result = MR_MAX_( result, i % N );

    return result;
}

int MRUtil_InitializeBufferProcessor(MRUtilBufferProcessor* bp,
                                     int outputChannelCount, MRSampleFormat userOutputSampleFormat,
                                     MRSampleFormat hostOutputSampleFormat,
                                     double sampleRate,
                                     unsigned long streamFlags,
                                     unsigned long framesPerUserBuffer, /* 0 indicates don't care */
                                     unsigned long framesPerHostBuffer,
                                     MRUtilHostBufferSizeMode hostBufferSizeMode,
                                     MRStreamCallback *streamCallback, void *userData)
{
    int result = outer_NoError;
    int bytesPerSample;
    unsigned long tempOutputBufferSize;

    /**if(streamFlags & outflag_NeverDropInput)
    {
        // paNeverDropInput is only valid for full-duplex callback streams, with an unspecified number of frames per buffer.
        if(!streamCallback || !(outputChannelCount > 0) ||
                framesPerUserBuffer != mrFramesPerBufferUnspecified)
            return outer_InvalidFlag;
    }
    */
    /* initialize buffer ptrs to zero so they can be freed if necessary in error */
    bp->tempOutputBuffer = 0;
    bp->tempOutputBufferPtrs = 0;

    bp->framesPerUserBuffer = framesPerUserBuffer;
    bp->framesPerHostBuffer = framesPerHostBuffer;

    bp->outputChannelCount = outputChannelCount;

    bp->hostBufferSizeMode = hostBufferSizeMode;

    bp->hostOutputChannels[0] = bp->hostOutputChannels[1] = 0;

    if(framesPerUserBuffer == 0) /* streamCallback will accept any buffer size */
    {
        bp->useNonAdaptingProcess = 1;
        bp->initialFramesInTempOutputBuffer = 0;

        if(hostBufferSizeMode == mrUtilFixedHostBufferSize
                || hostBufferSizeMode == mrUtilBoundedHostBufferSize)
        {
            bp->framesPerTempBuffer = framesPerHostBuffer;
        }
        else /* unknown host buffer size */
        {
            bp->framesPerTempBuffer = MR_FRAMES_PER_TEMP_BUFFER_WHEN_HOST_BUFFER_SIZE_IS_UNKNOWN_;
        }
    }
    else
    {
        bp->framesPerTempBuffer = framesPerUserBuffer;

        if(hostBufferSizeMode == mrUtilFixedHostBufferSize
                && framesPerHostBuffer % framesPerUserBuffer == 0)
        {
            bp->useNonAdaptingProcess = 1;
            bp->initialFramesInTempOutputBuffer = 0;
        }
        else
        {
            bp->useNonAdaptingProcess = 0;

            if(outputChannelCount > 0)
            {
                /* full duplex */
                if(hostBufferSizeMode == mrUtilFixedHostBufferSize)
                {
                    unsigned long frameShift =
                        CalculateFrameShift(framesPerHostBuffer, framesPerUserBuffer);

                    if(framesPerUserBuffer > framesPerHostBuffer)
                    {
                        bp->initialFramesInTempOutputBuffer = 0;
                    }
                    else
                    {
                        bp->initialFramesInTempOutputBuffer = frameShift;
                    }
                }
                else /* variable host buffer size, add framesPerUserBuffer latency */
                {
                    bp->initialFramesInTempOutputBuffer = framesPerUserBuffer;
                }
            }
            else
            {
                /* half duplex */
                bp->initialFramesInTempOutputBuffer = 0;
            }

        }
    }

    bp->framesInTempOutputBuffer = bp->initialFramesInTempOutputBuffer;

    if(outputChannelCount > 0)
    {
        bytesPerSample = MR_GetSampleSize(hostOutputSampleFormat);
        if( bytesPerSample > 0)
        {
            bp->bytesPerHostOutputSample = bytesPerSample;
        }
        else
        {
            result = bytesPerSample;
            goto error;
        }

        bytesPerSample = MR_GetSampleSize(userOutputSampleFormat);
        if( bytesPerSample > 0 )
        {
            bp->bytesPerUserOutputSample = bytesPerSample;
        }
        else
        {
            result = bytesPerSample;
            goto error;
        }

        bp->outputConverter =
            MRUtil_SelectConverter(userOutputSampleFormat, hostOutputSampleFormat, streamFlags);

        bp->outputZeroer = MRUtil_SelectZeroer(hostOutputSampleFormat);

        bp->userOutputIsInterleaved = (userOutputSampleFormat & mrNonInterleaved)?0:1;

        bp->hostOutputIsInterleaved = (hostOutputSampleFormat & mrNonInterleaved)?0:1;

        bp->userOutputSampleFormatIsEqualToHost = ((userOutputSampleFormat & ~mrNonInterleaved) == (hostOutputSampleFormat & ~mrNonInterleaved));

        tempOutputBufferSize =
            bp->framesPerTempBuffer * bp->bytesPerUserOutputSample * outputChannelCount;

        bp->tempOutputBuffer = MROutput_AllocateMemory(tempOutputBufferSize);
        if(bp->tempOutputBuffer == 0)
        {
            result = outer_InsufficientMemory;
            goto error;
        }

        if(bp->framesInTempOutputBuffer > 0)
            memset(bp->tempOutputBuffer, 0, tempOutputBufferSize);

        if(userOutputSampleFormat & mrNonInterleaved)
        {
            bp->tempOutputBufferPtrs =
                (void **)MROutput_AllocateMemory(sizeof(void*)*outputChannelCount);
            if(bp->tempOutputBufferPtrs == 0)
            {
                result = outer_InsufficientMemory;
                goto error;
            }
        }

        bp->hostOutputChannels[0] = (MRUtilChannelDescriptor*)
                                    MROutput_AllocateMemory(sizeof(MRUtilChannelDescriptor)*outputChannelCount * 2);
        if(bp->hostOutputChannels[0] == 0)
        {
            result = outer_InsufficientMemory;
            goto error;
        }

        bp->hostOutputChannels[1] = &bp->hostOutputChannels[0][outputChannelCount];
    }

    MRUtil_InitializeTriangularDitherState(&bp->ditherGenerator);

    bp->samplePeriod = 1. / sampleRate;

    bp->streamCallback = streamCallback;
    bp->userData = userData;

    return result;

error:

    if(bp->tempOutputBuffer )
        MROutput_FreeMemory(bp->tempOutputBuffer);

    if(bp->tempOutputBufferPtrs)
        MROutput_FreeMemory(bp->tempOutputBufferPtrs);

    if( bp->hostOutputChannels[0] )
        MROutput_FreeMemory( bp->hostOutputChannels[0]);

    return result;
}

unsigned long MRUtil_GetBufferProcessorOutputLatencyFrames(MRUtilBufferProcessor* bp)
{
    return bp->initialFramesInTempOutputBuffer;
}
