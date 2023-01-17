#ifndef MAIN_OUT_H_INCLUDED
#define MAIN_OUT_H_INCLUDED

#include "../common.h"

#define WAVE_FORMAT_96M08   0x00010000
#define WAVE_FORMAT_96S08   0x00020000
#define WAVE_FORMAT_96M16   0x00040000
#define WAVE_FORMAT_96S16   0x00080000

#define outer_NoError                                       0
#define outer_IncompatibleHostApiSpecificStreamInfo         1
#define outer_SampleFormatNotSupported                      2
#define outer_InsufficientMemory                            3
#define outer_DeviceUnavailable                             4
#define outer_UnanticipatedHostError                        5
#define outer_InvalidFlag                                   6

#define mrFramesPerBufferUnspecified                    0

#define outflag_NeverDropInput  ((unsigned long) 0x00000004)

#define   mrNoFlag          ((unsigned long) 0)
#define   mrClipOff         ((unsigned long) 0x00000001)
#define   mrDitherOff       ((unsigned long) 0x00000002)

//typedef int MRDeviceIndex;

//#define mrNoDevice ((MRDeviceIndex)-1)

typedef struct MRStreamInfo
{
    int structVersion;
    double outputLatency;
    double sampleRate;

} MRStreamInfo;

typedef void MRStreamFinishedCallback(void *userData);

typedef struct MRStreamCallbackTimeInfo
{
    double inputBufferAdcTime;  /**< The time when the first sample of the input buffer was captured at the ADC input */
    double currentTime;         /**< The time when the stream callback was invoked */
    double outputBufferDacTime; /**< The time when the first sample of the output buffer will output the DAC */
} MRStreamCallbackTimeInfo;

typedef int MRStreamCallback(
    void *output,
    unsigned long frameCount,
    const MRStreamCallbackTimeInfo* timeInfo,
    unsigned long statusFlags,
    void *userData);

typedef struct MRUtilStreamRepresentation
{
    unsigned long magic;    /**< set to MAGIC */
    struct MRUtilStreamRepresentation *nextOpenStream; /**< field used by multi-api code */
    //MRUtilStreamInterface *streamInterface;
    MRStreamCallback *streamCallback;
    MRStreamFinishedCallback *streamFinishedCallback;
    void *userData;
    MRStreamInfo streamInfo;
} MRUtilStreamRepresentation;

class main_output: public chain
{
public:
    main_output()
    {
        last_error_code = 0;
        isStopped       = 0;
        isActive        = 0;
    }
    virtual ~main_output()
    {
    }

    virtual void push() = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual int openaudio(const MRStreamParameters *outParam, unsigned long framesPerBuffer, unsigned long audioFlags,
                          MRStreamCallback *streamCallback,
                          void *userData) = 0;
    virtual int getdevcount() = 0;
    virtual int getdevinfo(sys_info_t *sysi, unsigned short devidx, MRDeviceInfo *outdevinfo, int *success) = 0;
    virtual int freehostapi(MRDeviceInfo *outdevinfo) =0;
    virtual void getdefaultlatencies(sys_info_t *sysi, double *defaultLowLatency, double *defaultHighLatency) = 0;

    size_t get_buffer_size()
    {
        return _data.size();
    }

    void set_parameters(unsigned short samplerate, unsigned short channel)
    {

    }

    double samples_to_seconds()
    {
        return 0;
    }

    void set_last_error_code(int ErrorCode, const wchar_t *errorText)
    {
        last_error_code = ErrorCode;
        last_error_code_str.assign(errorText);
    }

protected:
    unsigned short curdev_idx;
    stream _data;
    wstring last_error_code_str;
    int last_error_code;

    int isStopped;
    int isActive;

    unsigned long outputDeviceCount;
private:

};

main_output *output_create(MRHostApiTypeId outputype);

unsigned short MR_GetSampleSize(MRSampleFormat format);
MRSampleFormat MRUtil_SelectClosestAvailableFormat(MRSampleFormat availableFormats, MRSampleFormat format);
void MRUtil_InitializeStreamRepresentation(MRUtilStreamRepresentation *streamRepresentation,
        MRStreamCallback *streamCallback,
        void *userData);

#endif // MAIN_OUT_H_INCLUDED
