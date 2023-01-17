#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <mmsystem.h>
#include <malloc.h>
#include <memory.h>

#include "output_wmme.h"

#define MR_MME_USE_HIGH_DEFAULT_LATENCY_    (0)  /* For debugging glitches. */

#if MR_MME_USE_HIGH_DEFAULT_LATENCY_
#define MR_MME_WIN_9X_DEFAULT_LATENCY_                              (0.4)
#define MR_MME_MIN_HOST_OUTPUT_BUFFER_COUNT_                        (4)
#define MR_MME_HOST_BUFFER_GRANULARITY_FRAMES_WHEN_UNSPECIFIED_	    (16)
#define MR_MME_MAX_HOST_BUFFER_SECS_				                (0.3)       /* Do not exceed unless user buffer exceeds */
#define MR_MME_MAX_HOST_BUFFER_BYTES_				                (32 * 1024) /* Has precedence over PA_MME_MAX_HOST_BUFFER_SECS_, some drivers are known to crash with buffer sizes > 32k */
#else
#define MR_MME_WIN_9X_DEFAULT_LATENCY_                             (0.2)
#define MR_MME_MIN_HOST_OUTPUT_BUFFER_COUNT_                       (2)
#define MR_MME_HOST_BUFFER_GRANULARITY_FRAMES_WHEN_UNSPECIFIED_	(16)
#define MR_MME_MAX_HOST_BUFFER_SECS_				                (0.1)       /* Do not exceed unless user buffer exceeds */
#define MR_MME_MAX_HOST_BUFFER_BYTES_				                (32 * 1024) /* Has precedence over PA_MME_MAX_HOST_BUFFER_SECS_, some drivers are known to crash with buffer sizes > 32k */
#endif


#define MR_MME_WIN_NT_DEFAULT_LATENCY_              (0.4)
#define MR_MME_WIN_WDM_DEFAULT_LATENCY_             (0.090)
#define MR_MME_TARGET_HOST_BUFFER_COUNT_            8
#define MR_MME_MIN_TIMEOUT_MSEC_                    (1000)

char *StrTCpyToC(char *to, const TCHAR *from)
{
    int count = wcslen(from);
    if (count != 0)
        if (WideCharToMultiByte(CP_ACP, 0, from, count, to, count, NULL, NULL) == 0)
            return NULL;
    return to;
}

size_t StrTLen(const TCHAR *str)
{
    return wcslen(str);
}

#define PA_MME_SET_LAST_WAVEOUT_ERROR( mmresult ) \
    {                                                                   \
        wchar_t mmeErrorTextWide[ MAXERRORLENGTH ];                     \
        waveOutGetErrorText( mmresult, mmeErrorTextWide, MAXERRORLENGTH );  \
        set_last_error_code(mmresult, mmeErrorText );   \
    }

int output_wmme::CalculateMaxHostSampleFrameSizeBytes(
    unsigned short channelCount,
    MRSampleFormat hostSampleFormat,
    const MRWinMmeStreamInfo *streamInfo,
    int *hostSampleFrameSizeBytes)
{
    unsigned int i;
    /* PA WMME streams may aggregate multiple WMME devices. When the stream addresses
       more than one device in a single direction, maxDeviceChannelCount is the maximum
       number of channels used by a single device.
    */
    int maxDeviceChannelCount = channelCount;
    int hostSampleSizeBytes = MR_GetSampleSize(hostSampleFormat);
    if(hostSampleSizeBytes < 0)
    {
        return hostSampleSizeBytes; /* the value of hostSampleSize here is an error code, not a sample size */
    }

    *hostSampleFrameSizeBytes = hostSampleSizeBytes * maxDeviceChannelCount;

    return outer_NoError;
}

unsigned long output_wmme::ComputeHostBufferCountForFixedBufferSizeFrames(
    unsigned long suggestedLatencyFrames,
    unsigned long hostBufferSizeFrames,
    unsigned long minimumBufferCount)
{
    /* Calculate the number of buffers of length hostFramesPerBuffer
       that fit in suggestedLatencyFrames, rounding up to the next integer.

       The value (hostBufferSizeFrames - 1) below is to ensure the buffer count is rounded up.
    */
    unsigned long resultBufferCount = ((suggestedLatencyFrames + (hostBufferSizeFrames - 1)) / hostBufferSizeFrames);

    /* We always need one extra buffer for processing while the rest are queued/playing.
       i.e. latency is framesPerBuffer * (bufferCount - 1)
    */
    resultBufferCount += 1;

    if( resultBufferCount < minimumBufferCount ) /* clamp to minimum buffer count */
        resultBufferCount = minimumBufferCount;

    return resultBufferCount;
}

unsigned long output_wmme::ComputeHostBufferSizeGivenHardUpperLimit(
    unsigned long userFramesPerBuffer,
    unsigned long absoluteMaximumBufferSizeFrames)
{
    static unsigned long primes_[] = {2, 3, 5, 7, 11, 13, 17, 19, 23,
                                      29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 0
                                     }; /* zero terminated */

    unsigned long result = userFramesPerBuffer;
    int i;

    /* search for the largest integer factor of userFramesPerBuffer less
       than or equal to absoluteMaximumBufferSizeFrames */

    /* repeatedly divide by smallest prime factors until a buffer size
       smaller than absoluteMaximumBufferSizeFrames is found */
    while(result > absoluteMaximumBufferSizeFrames)
    {

        /* search for the smallest prime factor of result */
        for(i=0; primes_[i] != 0; ++i)
        {
            unsigned long p = primes_[i];
            unsigned long divided = result / p;
            if(divided*p == result)
            {
                result = divided;
                break; /* continue with outer while loop */
            }
        }
        if(primes_[i] == 0)
        {
            /* loop failed to find a prime factor, return an approximate result */
            unsigned long d = (userFramesPerBuffer + (absoluteMaximumBufferSizeFrames-1))
                              / absoluteMaximumBufferSizeFrames;
            return userFramesPerBuffer / d;
        }
    }

    return result;
}

int output_wmme::SelectHostBufferSizeFramesAndHostBufferCount(
    unsigned long suggestedLatencyFrames,
    unsigned long userFramesPerBuffer,
    unsigned long minimumBufferCount,
    unsigned long preferredMaximumBufferSizeFrames, /* try not to exceed this. for example, don't exceed when coalescing buffers */
    unsigned long absoluteMaximumBufferSizeFrames,  /* never exceed this, a hard limit */
    unsigned long *hostBufferSizeFrames,
    unsigned long *hostBufferCount )
{
    unsigned long effectiveUserFramesPerBuffer;
    unsigned long numberOfUserBuffersPerHostBuffer;


    if(userFramesPerBuffer == mrFramesPerBufferUnspecified)
    {
        effectiveUserFramesPerBuffer = MR_MME_HOST_BUFFER_GRANULARITY_FRAMES_WHEN_UNSPECIFIED_;
    }
    else
    {
        if(userFramesPerBuffer > absoluteMaximumBufferSizeFrames)
        {
            effectiveUserFramesPerBuffer = ComputeHostBufferSizeGivenHardUpperLimit(userFramesPerBuffer, absoluteMaximumBufferSizeFrames);
            if(suggestedLatencyFrames < userFramesPerBuffer)
                suggestedLatencyFrames = userFramesPerBuffer;
        }
        else
        {
            effectiveUserFramesPerBuffer = userFramesPerBuffer;
        }
    }

    /* compute a host buffer count based on suggestedLatencyFrames and our granularity */

    *hostBufferSizeFrames = effectiveUserFramesPerBuffer;
    *hostBufferCount = ComputeHostBufferCountForFixedBufferSizeFrames(
                           suggestedLatencyFrames, *hostBufferSizeFrames, minimumBufferCount);

    if(*hostBufferSizeFrames >= userFramesPerBuffer)
    {
        /*
            If there are too many host buffers we would like to coalesce
            them by packing an integer number of user buffers into each host buffer.
            We try to coalesce such that hostBufferCount will lie between
            PA_MME_TARGET_HOST_BUFFER_COUNT_ and (PA_MME_TARGET_HOST_BUFFER_COUNT_*2)-1.
            We limit coalescing to avoid exceeding either absoluteMaximumBufferSizeFrames and
            preferredMaximumBufferSizeFrames.

            First, compute a coalescing factor: the number of user buffers per host buffer.
            The goal is to achieve PA_MME_TARGET_HOST_BUFFER_COUNT_ total buffer count.
            Since our latency is computed based on (*hostBufferCount - 1) we compute a
            coalescing factor based on (*hostBufferCount - 1) and (PA_MME_TARGET_HOST_BUFFER_COUNT_-1).

            The + (PA_MME_TARGET_HOST_BUFFER_COUNT_-2) term below is intended to round up.
        */
        numberOfUserBuffersPerHostBuffer = ((*hostBufferCount - 1) + (MR_MME_TARGET_HOST_BUFFER_COUNT_-2)) / (MR_MME_TARGET_HOST_BUFFER_COUNT_ - 1);

        if(numberOfUserBuffersPerHostBuffer > 1)
        {
            unsigned long maxCoalescedBufferSizeFrames = (absoluteMaximumBufferSizeFrames < preferredMaximumBufferSizeFrames) /* minimum of our limits */
                    ? absoluteMaximumBufferSizeFrames
                    : preferredMaximumBufferSizeFrames;

            unsigned long maxUserBuffersPerHostBuffer = maxCoalescedBufferSizeFrames / effectiveUserFramesPerBuffer; /* don't coalesce more than this */

            if( numberOfUserBuffersPerHostBuffer > maxUserBuffersPerHostBuffer )
                numberOfUserBuffersPerHostBuffer = maxUserBuffersPerHostBuffer;

            *hostBufferSizeFrames = effectiveUserFramesPerBuffer * numberOfUserBuffersPerHostBuffer;

            /* recompute hostBufferCount to approximate suggestedLatencyFrames now that hostBufferSizeFrames is larger */
            *hostBufferCount = ComputeHostBufferCountForFixedBufferSizeFrames(
                                   suggestedLatencyFrames, *hostBufferSizeFrames, minimumBufferCount);
        }
    }

    return outer_NoError;
}


int output_wmme::CalculateBufferSettings(
    unsigned long *hostFramesPerOutputBuffer, unsigned long *hostOutputBufferCount,
    unsigned short outputChannelCount, MRSampleFormat hostOutputSampleFormat,
    double suggestedOutputLatency, const MRWinMmeStreamInfo *outputStreamInfo,
    double sampleRate, unsigned long userFramesPerBuffer)
{
    int result = outer_NoError;

    if(outputStreamInfo
            && (outputStreamInfo->flags & mrWinMmeUseLowLevelLatencyParameters))
    {
        /* output - using low level latency parameters */

        if( outputStreamInfo->bufferCount <= 0
                || outputStreamInfo->framesPerBuffer <= 0 )
        {
            result = outer_IncompatibleHostApiSpecificStreamInfo;
            goto error;
        }

        *hostFramesPerOutputBuffer = outputStreamInfo->framesPerBuffer;
        *hostOutputBufferCount = outputStreamInfo->bufferCount;
    }
    else
    {
        /* output - no low level latency parameters, so compute hostFramesPerOutputBuffer and hostOutputBufferCount
            based on userFramesPerBuffer and suggestedOutputLatency. */

        int hostOutputFrameSizeBytes;
        result = CalculateMaxHostSampleFrameSizeBytes(
                     outputChannelCount, hostOutputSampleFormat, outputStreamInfo, &hostOutputFrameSizeBytes);
        if(result != outer_NoError)
            goto error;

        /* compute the output buffer size and count */

        result = SelectHostBufferSizeFramesAndHostBufferCount(
                     (unsigned long)(suggestedOutputLatency * sampleRate), /* (truncate) */
                     userFramesPerBuffer,
                     MR_MME_MIN_HOST_OUTPUT_BUFFER_COUNT_,
                     (unsigned long)(MR_MME_MAX_HOST_BUFFER_SECS_ * sampleRate), /* in frames. preferred maximum */
                     (MR_MME_MAX_HOST_BUFFER_BYTES_ / hostOutputFrameSizeBytes),  /* in frames. a hard limit. note truncation due to
                                                                                 division is intentional here to limit max bytes */
                     hostFramesPerOutputBuffer,
                     hostOutputBufferCount);
        if(result != outer_NoError)
            goto error;

    }
error:
    return result;
}

int output_wmme::ValidateWinMmeSpecificStreamInfo(
    const MRWinMmeStreamInfo *streamInfo,
    unsigned long *winMmeSpecificFlags)
{
    if(streamInfo)
    {
        if(streamInfo->size != sizeof(MRWinMmeStreamInfo)
                || streamInfo->version != 1)
        {
            return outer_IncompatibleHostApiSpecificStreamInfo;
        }

        *winMmeSpecificFlags = streamInfo->flags;
    }

    return outer_NoError;
}

int output_wmme::SampleFormatAndWinWmmeSpecificFlagsToLinearWaveFormatTag(MRSampleFormat sampleFormat, unsigned long winMmeSpecificFlags)
{
    int waveFormatTag = 0;

    if(winMmeSpecificFlags & mrWinMmeWaveFormatDolbyAc3Spdif)
        waveFormatTag = MR_WAVE_FORMAT_DOLBY_AC3_SPDIF;
    else if(winMmeSpecificFlags & mrWinMmeWaveFormatWmaSpdif)
        waveFormatTag = MR_WAVE_FORMAT_WMA_SPDIF;
    else
        waveFormatTag = MRWin_SampleFormatToLinearWaveFormatTag(sampleFormat);

    return waveFormatTag;
}

int output_wmme::QueryFormatSupported(MRWinMmeDeviceInfo* deviceInfo,
                                      int (*waveFormatExQueryFunction)(int, WAVEFORMATEX*),
                                      unsigned short winMmeDeviceId, unsigned short channels, double sampleRate, unsigned long winMmeSpecificFlags)
{
    MRWinWaveFormat waveFormat;
    MRSampleFormat sampleFormat;
    int waveFormatTag;

    /* @todo at the moment we only query with 16 bit sample format and directout speaker config*/

    sampleFormat = mrInt16;
    waveFormatTag = SampleFormatAndWinWmmeSpecificFlagsToLinearWaveFormatTag(sampleFormat, winMmeSpecificFlags);

    if(waveFormatTag == MRWin_SampleFormatToLinearWaveFormatTag(mrInt16))
    {

        /* attempt bypass querying the device for linear formats */

        if(sampleRate == 11025.0
                && ( (channels == 1 && (deviceInfo->dwFormats & WAVE_FORMAT_1M16))
                     || (channels == 2 && (deviceInfo->dwFormats & WAVE_FORMAT_1S16))))
        {
            return outer_NoError;
        }

        if(sampleRate == 22050.0
                && ( (channels == 1 && (deviceInfo->dwFormats & WAVE_FORMAT_2M16))
                     || (channels == 2 && (deviceInfo->dwFormats & WAVE_FORMAT_2S16))))
        {
            return outer_NoError;
        }

        if(sampleRate == 44100.0
                && ( (channels == 1 && (deviceInfo->dwFormats & WAVE_FORMAT_4M16))
                     || (channels == 2 && (deviceInfo->dwFormats & WAVE_FORMAT_4S16))))
        {
            return outer_NoError;
        }
    }


    /* first, attempt to query the device using WAVEFORMATEXTENSIBLE,
       if this fails we fall back to WAVEFORMATEX */

    MRWin_InitializeWaveFormatExtensible(&waveFormat, channels, sampleFormat, waveFormatTag,
                                         sampleRate, MR_SPEAKER_DIRECTOUT);

    if(waveFormatExQueryFunction(winMmeDeviceId, (WAVEFORMATEX*)&waveFormat ) == outer_NoError)
        return outer_NoError;

    MRWin_InitializeWaveFormatEx(&waveFormat, channels, sampleFormat, waveFormatTag, sampleRate);

    return waveFormatExQueryFunction(winMmeDeviceId, (WAVEFORMATEX*)&waveFormat);
}

#define MR_DEFAULTSAMPLERATESEARCHORDER_COUNT_  (13)
static double defaultSampleRateSearchOrder_[] =
{
    44100.0, 48000.0, 32000.0, 24000.0, 22050.0, 88200.0, 96000.0, 192000.0,
    16000.0, 12000.0, 11025.0, 9600.0, 8000.0
};

void output_wmme::DetectDefaultSampleRate(MRDeviceInfo *outdevinfo, unsigned short winMmeDeviceId,
        int (*waveFormatExQueryFunction)(int, WAVEFORMATEX*), unsigned short maxChannels)
{
    int i;

    MRWinMmeDeviceInfo* deviceInfo = (MRWinMmeDeviceInfo*)outdevinfo->host_devinfo;
    outdevinfo->defaultSampleRate = 0.;

    for(i=0; i < MR_DEFAULTSAMPLERATESEARCHORDER_COUNT_; ++i)
    {
        double sampleRate = defaultSampleRateSearchOrder_[i];
        int mrerror = QueryFormatSupported(deviceInfo, waveFormatExQueryFunction, winMmeDeviceId, maxChannels, sampleRate, 0);
        if(mrerror == outer_NoError)
        {
            outdevinfo->defaultSampleRate = sampleRate;
            break;
        }
    }
}

static int QueryOutputWaveFormatEx(int deviceId, WAVEFORMATEX *waveFormatEx)
{
    MMRESULT mmresult;

    switch(mmresult = waveOutOpen(NULL, deviceId, waveFormatEx, 0, 0, WAVE_FORMAT_QUERY))
    {
    case MMSYSERR_NOERROR:
        return outer_NoError;
    case MMSYSERR_ALLOCATED:    /* Specified resource is already allocated. */
        return outer_DeviceUnavailable;
    case MMSYSERR_NODRIVER:	    /* No device driver is present. */
        return outer_DeviceUnavailable;
    case MMSYSERR_NOMEM:	    /* Unable to allocate or lock memory. */
        return outer_InsufficientMemory;
    case WAVERR_BADFORMAT:      /* Attempted to open with an unsupported waveform-audio format. */
        return outer_SampleFormatNotSupported;

    case MMSYSERR_BADDEVICEID:	/* Specified device identifier is out of range. */
    /* falls through */
    default:
        //PA_MME_SET_LAST_WAVEOUT_ERROR(mmresult);
        return outer_UnanticipatedHostError;
    }
}

int output_wmme::CloseHandleWithMrError(HANDLE handle)
{
    int result = outer_NoError;

    if(handle)
    {
        if(CloseHandle(handle) == 0)
        {
            result = outer_UnanticipatedHostError;
            //PA_MME_SET_LAST_SYSTEM_ERROR(GetLastError());
        }
    }

    return result;
}

int output_wmme::CreateEventWithMrError(HANDLE *handle,
                                        LPSECURITY_ATTRIBUTES lpEventAttributes,
                                        BOOL bManualReset,
                                        BOOL bInitialState,
                                        LPCTSTR lpName)
{
    int result = outer_NoError;

    *handle = NULL;

    *handle = CreateEvent(lpEventAttributes, bManualReset, bInitialState, lpName);
    if(*handle == NULL )
    {
        result = outer_UnanticipatedHostError;
        //PA_MME_SET_LAST_SYSTEM_ERROR( GetLastError() );
    }

    return result;
}

void output_wmme::getdefaultlatencies(sys_info_t *sysi, double *defaultLowLatency, double *defaultHighLatency)
{
    /* Check for NT */
    if((sysi->dwMajorVersion == 4) && (sysi->dwPlatformId == 2))
    {
        *defaultLowLatency = MR_MME_WIN_NT_DEFAULT_LATENCY_;
    }
    else if(sysi->dwMajorVersion >= 5)
    {
        *defaultLowLatency  = MR_MME_WIN_WDM_DEFAULT_LATENCY_;
    }
    else
    {
        *defaultLowLatency  = MR_MME_WIN_9X_DEFAULT_LATENCY_;
    }

    *defaultHighLatency = *defaultLowLatency * 2;
}

int output_wmme::TerminateWaveHandles(MRWinMmeSingleDirectionHandlesAndBuffers *handlesAndBuffers,
                                      int currentlyProcessingAnError)
{
    int result = outer_NoError;
    MMRESULT mmresult;
    signed int i;

    if(handlesAndBuffers->waveHandles)
    {
        mmresult = waveOutClose(handlesAndBuffers->waveHandles);

        if(mmresult != MMSYSERR_NOERROR &&
                !currentlyProcessingAnError) /* don't update the error state if we're already processing an error */
        {
            result = outer_UnanticipatedHostError;
            //PA_MME_SET_LAST_WAVEOUT_ERROR( mmresult );
            /* note that we don't break here, we try to continue closing devices */
        }

        MROutput_FreeMemory(handlesAndBuffers->waveHandles);
        handlesAndBuffers->waveHandles = 0;
    }
    else
        mmresult = MMSYSERR_NOERROR;

    if(handlesAndBuffers->bufferEvent)
    {
        result = CloseHandleWithMrError(handlesAndBuffers->bufferEvent);
        handlesAndBuffers->bufferEvent = 0;
    }

    return result;
}

int output_wmme::InitializeWaveHandles(MRWinMmeSingleDirectionHandlesAndBuffers *handlesAndBuffers,
                                       unsigned long winMmeSpecificFlags,
                                       unsigned long bytesPerHostSample,
                                       double sampleRate,
                                       unsigned short channelCount,
                                       MRWinWaveFormatChannelMask channelMask)
{
    int result;
    MMRESULT mmresult;
    signed int j;
    MRSampleFormat sampleFormat;
    int waveFormatTag;

    result = CreateEventWithMrError(&handlesAndBuffers->bufferEvent, NULL, FALSE, FALSE, NULL);
    if(result != outer_NoError)
    {
        TerminateWaveHandles(handlesAndBuffers, 1);
        return result;
    }

    /*handlesAndBuffers->waveHandles = (void*)MROutput_AllocateMemory(sizeof(HWAVEOUT));
    if( !handlesAndBuffers->waveHandles )
    {
        result = mrInsufficientMemory;
        goto error_1;
    }*/

    //handlesAndBuffers->deviceCount = deviceCount;

    /*for( i = 0; i < (signed int)deviceCount; ++i )
    {
        ((HWAVEOUT*)handlesAndBuffers->waveHandles)[i] = 0;
    }*/

    /* @todo at the moment we only use 16 bit sample format */
    sampleFormat = mrInt16;
    waveFormatTag = SampleFormatAndWinWmmeSpecificFlagsToLinearWaveFormatTag(sampleFormat, winMmeSpecificFlags);

    MRWinWaveFormat waveFormat;
    UINT winMmeDeviceId = (UINT)((curdev_idx == 0) ? WAVE_MAPPER : (curdev_idx - 1));

    /* @todo: consider providing a flag or #define to not try waveformat extensible
       this could just initialize j to 1 the first time round. */

    for( j = 0; j < 2; ++j )
    {
        switch(j)
        {
        case 0:
            /* first, attempt to open the device using WAVEFORMATEXTENSIBLE,
                if this fails we fall back to WAVEFORMATEX */

        {
            MRWin_InitializeWaveFormatExtensible(&waveFormat, channelCount,
                                                 sampleFormat, waveFormatTag, sampleRate, channelMask);
        }
        break;

        case 1:
            /* retry with WAVEFORMATEX */

        {
            MRWin_InitializeWaveFormatEx(&waveFormat, channelCount,
                                         sampleFormat, waveFormatTag, sampleRate);
        }
        break;
        }

        /* REVIEW: consider not firing an event for input when a full duplex
            stream is being used. this would probably depend on the
            neverDropInput flag. */

        mmresult = waveOutOpen((HWAVEOUT*)handlesAndBuffers->waveHandles, winMmeDeviceId,
                               (WAVEFORMATEX*)&waveFormat,
                               (DWORD_PTR)handlesAndBuffers->bufferEvent, (DWORD_PTR)0, CALLBACK_EVENT);

        if(mmresult == MMSYSERR_NOERROR)
        {
            break; /* success */
        }
        else if(j == 0)
        {
            continue; /* try again with WAVEFORMATEX */
        }
        else
        {
            switch( mmresult )
            {
            case MMSYSERR_ALLOCATED:    /* Specified resource is already allocated. */
            {
                result = outer_DeviceUnavailable;
            }
            break;
            case MMSYSERR_NODRIVER:	    /* No device driver is present. */
            {
                result = outer_DeviceUnavailable;
            }
            break;
            case MMSYSERR_NOMEM:	    /* Unable to allocate or lock memory. */
            {
                result = outer_InsufficientMemory;
            }
            break;

            case MMSYSERR_BADDEVICEID:	/* Specified device identifier is out of range. */
            /* falls through */
            case WAVERR_BADFORMAT:      /* Attempted to open with an unsupported waveform-audio format. */
            /* This can also occur if we try to open the device with an unsupported
             * number of channels. This is attempted when device*ChannelCountIsKnown is
             * set to 0.
             */
            /* falls through */
            default:
                result = outer_UnanticipatedHostError;
                //PA_MME_SET_LAST_WAVEOUT_ERROR( mmresult );
            }
            {
                TerminateWaveHandles(handlesAndBuffers, 1);
                return result;
            }
        }
    }

    return result;
}

void output_wmme::InitializeSingleDirectionHandlesAndBuffers(MRWinMmeSingleDirectionHandlesAndBuffers *handlesAndBuffers)
{
    handlesAndBuffers->bufferEvent = 0;
    handlesAndBuffers->waveHandles = 0;
    handlesAndBuffers->waveHeaders = 0;
    handlesAndBuffers->bufferCount = 0;
}

output_wmme::output_wmme()
{
    curdev_idx                      = 0;
    outputDeviceCount               = 0;
    winMmeSpecificOutputFlags       = 0;
}

output_wmme::~output_wmme()
{

}

int output_wmme::getdevcount()
{
    int result = 0;
    DWORD waveOutPreferredDevice;
    DWORD preferredDeviceStatusFlags;

#if !defined(DRVM_MAPPER_PREFERRED_GET)
    /* DRVM_MAPPER_PREFERRED_GET is defined in mmddk.h but we avoid a dependency on the DDK by defining it here */
#define DRVM_MAPPER_PREFERRED_GET    (0x2000+21)
#endif

    preferredDeviceStatusFlags = 0;
    waveOutPreferredDevice = -1;
    waveOutMessage((HWAVEOUT)WAVE_MAPPER, DRVM_MAPPER_PREFERRED_GET, (DWORD_PTR)&waveOutPreferredDevice, (DWORD_PTR)&preferredDeviceStatusFlags);

    result = waveOutGetNumDevs();
    if(result > 0)
        result++; // add WAVE_MAPPER
    return result;
}

int output_wmme::getdevinfo(sys_info_t *sysi, unsigned short devidx, MRDeviceInfo *outdevinfo, int *success)
{
    int ret = outer_NoError;
    MMRESULT mmresult;
    WAVEOUTCAPS woc;

    *success = 0;

    UINT winMmeDeviceId = (UINT)((devidx==0) ? WAVE_MAPPER : (devidx-1));
    mmresult = waveOutGetDevCaps(winMmeDeviceId, &woc, sizeof(WAVEOUTCAPS));

    if(mmresult == MMSYSERR_NOMEM)
    {
        ret = outer_InsufficientMemory;
        goto error;
    }
    else if(mmresult != MMSYSERR_NOERROR)
    {
        return outer_NoError;
    }

    MRWinMmeDeviceInfo *win_info;
    win_info = new MRWinMmeDeviceInfo;

    ZeroMemory(outdevinfo, sizeof(MRDeviceInfo));
    outdevinfo->devtype = mrMME;
    outdevinfo->name = new wstring(woc.szPname);

    if(woc.wChannels == 0xFFFF || woc.wChannels < 1 || woc.wChannels > 255)
    {
        outdevinfo->maxOutputChannels = 2;
        win_info->deviceOutputChannelCountIsKnown = 0;
    }
    else
    {
        outdevinfo->maxOutputChannels = woc.wChannels;
        win_info->deviceOutputChannelCountIsKnown = 1;
    }

    win_info->dwFormats = woc.dwFormats;

    //DetectDefaultSampleRate(outdevinfo, winMmeDeviceId,
    //                        QueryOutputWaveFormatEx, outdevinfo->maxOutputChannels);

    outdevinfo->host_devinfo = win_info;
    *success = 1;
error:
    return ret;
}

int output_wmme::freehostapi(MRDeviceInfo *outdevinfo)
{
    if(outdevinfo->host_devinfo)
    {
        delete(outdevinfo->name);
        delete(outdevinfo->host_devinfo);
        outdevinfo->host_devinfo = NULL;
        return 1;
    }
    else
        return 0;
}

int output_wmme::openaudio(const MRStreamParameters *outParam, unsigned long framesPerBuffer, unsigned long audioFlags,
                           MRStreamCallback *streamCallback,
                           void *userData)
{
    MRWinMmeStreamInfo *outputStreamInfo;
    double suggestedOutputLatency;
    MRSampleFormat hostOutputSampleFormat;
    unsigned long framesPerHostOutputBuffer;
    unsigned long hostOutputBufferCount;

    int bufferProcessorIsInitialized = 0;
    unsigned long winMmeSpecificOutputFlags = 0;
    MRWinWaveFormatChannelMask outputChannelMask;

    int streamRepresentationIsInitialized = 0;
    unsigned long framesPerBufferProcessorCall;

    unsigned long streamFlags = 0;

    int result = outer_NoError;

    curdev_idx = outParam->device;

    outputStreamInfo = (MRWinMmeStreamInfo*)outParam->host_streaminfo;
    result = ValidateWinMmeSpecificStreamInfo(outputStreamInfo, &winMmeSpecificOutputFlags);
    if(result != outer_NoError) return result;

    hostOutputSampleFormat = MRUtil_SelectClosestAvailableFormat(mrInt16, outParam->sampleFormat);

    if(outputStreamInfo && outputStreamInfo->flags & mrWinMmeUseChannelMask)
        outputChannelMask = outputStreamInfo->channelMask;
    else
        outputChannelMask = MRWinWaveFormatChannelMask(outParam->channelCount);

    suggestedOutputLatency = outParam->suggestedLatency;

    result = CalculateBufferSettings(&framesPerHostOutputBuffer, &hostOutputBufferCount,
                                     outParam->channelCount, hostOutputSampleFormat, suggestedOutputLatency, outputStreamInfo,
                                     outParam->sampleRate, framesPerBuffer);

    InitializeSingleDirectionHandlesAndBuffers(&output);
    abortEvent = 0;
    processingThread = 0;

    MRUtil_InitializeStreamRepresentation(&streamRepresentation, streamCallback, userData);
    streamRepresentationIsInitialized = 1;

    framesPerBufferProcessorCall = framesPerHostOutputBuffer;
    output.framesPerBuffer = framesPerHostOutputBuffer;

    result =  MRUtil_InitializeBufferProcessor(&bufferProcessor,
              outParam->channelCount, outParam->sampleFormat, hostOutputSampleFormat,
              outParam->sampleRate, streamFlags, framesPerBuffer,
              framesPerBufferProcessorCall, mrUtilFixedHostBufferSize,
              streamCallback, userData);
    if(result != outer_NoError) goto error;

    bufferProcessorIsInitialized = 1;

    streamRepresentation.streamInfo.outputLatency =
        (double)(MRUtil_GetBufferProcessorOutputLatencyFrames(&bufferProcessor)
                 + (framesPerHostOutputBuffer * (hostOutputBufferCount-1))) / outParam->sampleRate;
    streamRepresentation.streamInfo.sampleRate = outParam->sampleRate;

    //stream->primeStreamUsingCallback = ( (streamFlags&paPrimeOutputBuffersUsingStreamCallback) && streamCallback ) ? 1 : 0;

    isStopped = 1;
    isActive = 0;

    result = InitializeWaveHandles(&output,
                                    winMmeSpecificOutputFlags,
                                    bufferProcessor.bytesPerHostOutputSample, outParam->sampleRate,
                                    outParam->channelCount, outputChannelMask);
    if(result != outer_NoError) goto error;

error:

    return result;
}

void output_wmme::push()
{

}

void output_wmme::play()
{

}

void output_wmme::pause()
{

}

void output_wmme::stop()
{

}
