#ifndef OUTPUT_WMME_H_INCLUDED
#define OUTPUT_WMME_H_INCLUDED

#include "main_out.h"
#include "windows_waveformat.h"
#include "main_out_proc.h"

#define mrWinMmeUseLowLevelLatencyParameters            (0x01)
#define mrWinMmeUseMultipleDevices                      (0x02)  /* use mme specific multiple device feature */
#define mrWinMmeUseChannelMask                          (0x04)

/*  Flags for non-PCM spdif passthrough.
*/
#define mrWinMmeWaveFormatDolbyAc3Spdif                 (0x10)
#define mrWinMmeWaveFormatWmaSpdif                      (0x20)

typedef struct MRWinMmeStreamInfo
{
    unsigned long size;             /**< sizeof(MRWinMmeStreamInfo) */
    MRHostApiTypeId hostApiType;    /**< paMME */
    unsigned long version;          /**< 1 */

    unsigned long flags;
    unsigned long framesPerBuffer;
    unsigned long bufferCount;  /* formerly numBuffers */
    unsigned long deviceCount;
    MRWinWaveFormatChannelMask channelMask;
} MRWinMmeStreamInfo;

typedef struct
{
    //MRDeviceInfo inheritedDeviceInfo;
    DWORD dwFormats; /**<< standard formats bitmask from the WAVEINCAPS and WAVEOUTCAPS structures */
    char deviceOutputChannelCountIsKnown; /**<< if the system returns 0xFFFF then we don't really know the number of supported channels (1=>known, 0=>unknown)*/
}
MRWinMmeDeviceInfo;

/*typedef struct MRWinMmeDeviceAndChannelCount
{
    int device;
    int channelCount;
} MRWinMmeDeviceAndChannelCount;*/

typedef struct
{
    HANDLE bufferEvent;
    //void *waveHandles;
    HWAVEOUT waveHandles;
    //unsigned int deviceCount;
    /* unsigned int channelCount; */
    WAVEHDR **waveHeaders;                  /* waveHeaders[device][buffer] */
    unsigned int bufferCount;
    unsigned int currentBufferIndex;
    unsigned int framesPerBuffer;
    unsigned int framesUsedInCurrentBuffer;
} MRWinMmeSingleDirectionHandlesAndBuffers;

class output_wmme : public main_output
{
public:
    output_wmme();
    ~output_wmme();
    int getdevcount();
    int getdevinfo(sys_info_t *sysi, unsigned short devidx, MRDeviceInfo *outdevinfo, int *success);
    int openaudio(const MRStreamParameters *outParam, unsigned long framesPerBuffer, unsigned long audioFlags,
                  MRStreamCallback *streamCallback,
                  void *userData);
    int freehostapi(MRDeviceInfo *outdevinfo);
    void getdefaultlatencies(sys_info_t *sysi, double *defaultLowLatency, double *defaultHighLatency);
    void push();
    void play();
    void pause();
    void stop();
private:

/*
PaUtilStreamRepresentation streamRepresentation;
    PaUtilCpuLoadMeasurer cpuLoadMeasurer;
    PaUtilBufferProcessor bufferProcessor;

    int primeStreamUsingCallback;

    PaWinMmeSingleDirectionHandlesAndBuffers input;
    PaWinMmeSingleDirectionHandlesAndBuffers output;

    // Processing thread management --------------
    HANDLE abortEvent;
    HANDLE processingThread;
    PA_THREAD_ID processingThreadId;

    char throttleProcessingThreadOnOverload; // 0 -> don't throtte, non-0 -> throttle
    int processingThreadPriority;
    int highThreadPriority;
    int throttledThreadPriority;
    unsigned long throttledSleepMsecs;

    int isStopped;
    volatile int isActive;
    volatile int stopProcessing; // stop thread once existing buffers have been returned
    volatile int abortProcessing; // stop thread immediately

*/

//    MRWinMmeDeviceAndChannelCount *outputDevices;
    unsigned long winMmeSpecificOutputFlags;
    //
    MRUtilStreamRepresentation streamRepresentation;
    MRUtilBufferProcessor bufferProcessor;

    MRWinMmeSingleDirectionHandlesAndBuffers output;
    HANDLE abortEvent;
    HANDLE processingThread;
    //

    int ValidateWinMmeSpecificStreamInfo(
        const MRWinMmeStreamInfo *streamInfo,
        unsigned long *winMmeSpecificFlags);
    int CalculateMaxHostSampleFrameSizeBytes(
        unsigned short channelCount,
        MRSampleFormat hostSampleFormat,
        const MRWinMmeStreamInfo *streamInfo,
        int *hostSampleFrameSizeBytes);
    unsigned long ComputeHostBufferCountForFixedBufferSizeFrames(
        unsigned long suggestedLatencyFrames,
        unsigned long hostBufferSizeFrames,
        unsigned long minimumBufferCount);
    unsigned long ComputeHostBufferSizeGivenHardUpperLimit(
        unsigned long userFramesPerBuffer,
        unsigned long absoluteMaximumBufferSizeFrames);
    int SelectHostBufferSizeFramesAndHostBufferCount(
        unsigned long suggestedLatencyFrames,
        unsigned long userFramesPerBuffer,
        unsigned long minimumBufferCount,
        unsigned long preferredMaximumBufferSizeFrames, /* try not to exceed this. for example, don't exceed when coalescing buffers */
        unsigned long absoluteMaximumBufferSizeFrames,  /* never exceed this, a hard limit */
        unsigned long *hostBufferSizeFrames,
        unsigned long *hostBufferCount);
    int CalculateBufferSettings(
        unsigned long *hostFramesPerOutputBuffer, unsigned long *hostOutputBufferCount,
        unsigned short outputChannelCount, MRSampleFormat hostOutputSampleFormat,
        double suggestedOutputLatency, const MRWinMmeStreamInfo *outputStreamInfo,
        double sampleRate, unsigned long userFramesPerBuffer);
    void DetectDefaultSampleRate(MRDeviceInfo *outdevinfo, unsigned short winMmeDeviceId,
                                 int (*waveFormatExQueryFunction)(int, WAVEFORMATEX*), unsigned short maxChannels);
    int QueryFormatSupported(MRWinMmeDeviceInfo* deviceInfo,
                             int (*waveFormatExQueryFunction)(int, WAVEFORMATEX*),
                             unsigned short winMmeDeviceId, unsigned short channels, double sampleRate, unsigned long winMmeSpecificFlags);
    int SampleFormatAndWinWmmeSpecificFlagsToLinearWaveFormatTag(MRSampleFormat sampleFormat, unsigned long winMmeSpecificFlags);
    int InitializeWaveHandles(MRWinMmeSingleDirectionHandlesAndBuffers *handlesAndBuffers,
                              unsigned long winMmeSpecificFlags,
                              unsigned long bytesPerHostSample,
                              double sampleRate,
                              unsigned short channelCount,
                              MRWinWaveFormatChannelMask channelMask);
    int TerminateWaveHandles(MRWinMmeSingleDirectionHandlesAndBuffers *handlesAndBuffers,
                             int currentlyProcessingAnError);
    void InitializeSingleDirectionHandlesAndBuffers(MRWinMmeSingleDirectionHandlesAndBuffers *handlesAndBuffers);
    //
    int CreateEventWithMrError(HANDLE *handle,
                               LPSECURITY_ATTRIBUTES lpEventAttributes,
                               BOOL bManualReset,
                               BOOL bInitialState,
                               LPCTSTR lpName);
    int CloseHandleWithMrError(HANDLE handle);
    //
};

#endif // OUTPUT_WMME_H_INCLUDED
