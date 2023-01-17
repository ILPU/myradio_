#include <windows.h>
#include <mmsystem.h>

#include "main_out.h"
#include "windows_waveformat.h"

static GUID pawin_ksDataFormatSubtypeGuidBase =
{ (USHORT)(WAVE_FORMAT_PCM), 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 };

int MRWin_SampleFormatToLinearWaveFormatTag(MRSampleFormat sampleFormat)
{
    if(sampleFormat == mrFloat32)
        return MR_WAVE_FORMAT_IEEE_FLOAT;
    return MR_WAVE_FORMAT_PCM;
}

void MRWin_InitializeWaveFormatExtensible(MRWinWaveFormat *waveFormat,
        unsigned short numChannels, MRSampleFormat sampleFormat, int waveFormatTag, double sampleRate,
        MRWinWaveFormatChannelMask channelMask)
{
    WAVEFORMATEX *waveFormatEx = (WAVEFORMATEX*)waveFormat;
    int bytesPerSample = MR_GetSampleSize(sampleFormat);
    unsigned long bytesPerFrame = numChannels * bytesPerSample;
    GUID guid;

    waveFormatEx->wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    waveFormatEx->nChannels = (WORD)numChannels;
    waveFormatEx->nSamplesPerSec = (DWORD)sampleRate;
    waveFormatEx->nAvgBytesPerSec = waveFormatEx->nSamplesPerSec * bytesPerFrame;
    waveFormatEx->nBlockAlign = (WORD)bytesPerFrame;
    waveFormatEx->wBitsPerSample = bytesPerSample * 8;
    waveFormatEx->cbSize = 22;

    *((WORD*)&waveFormat->fields[MR_INDEXOF_WVALIDBITSPERSAMPLE]) =
        waveFormatEx->wBitsPerSample;

    *((DWORD*)&waveFormat->fields[MR_INDEXOF_DWCHANNELMASK]) = channelMask;

    guid = pawin_ksDataFormatSubtypeGuidBase;
    guid.Data1 = (USHORT)waveFormatTag;
    *((GUID*)&waveFormat->fields[MR_INDEXOF_SUBFORMAT]) = guid;
}

void MRWin_InitializeWaveFormatEx(MRWinWaveFormat *waveFormat,
                                  unsigned short numChannels, MRSampleFormat sampleFormat, int waveFormatTag, double sampleRate)
{
    WAVEFORMATEX *waveFormatEx = (WAVEFORMATEX*)waveFormat;
    int bytesPerSample = MR_GetSampleSize(sampleFormat);
    unsigned long bytesPerFrame = numChannels * bytesPerSample;

    waveFormatEx->wFormatTag = waveFormatTag;
    waveFormatEx->nChannels = (WORD)numChannels;
    waveFormatEx->nSamplesPerSec = (DWORD)sampleRate;
    waveFormatEx->nAvgBytesPerSec = waveFormatEx->nSamplesPerSec * bytesPerFrame;
    waveFormatEx->nBlockAlign = (WORD)bytesPerFrame;
    waveFormatEx->wBitsPerSample = bytesPerSample * 8;
    waveFormatEx->cbSize = 0;
}

MRWinWaveFormatChannelMask MRWin_DefaultChannelMask(unsigned short numChannels)
{
    switch(numChannels)
    {
    case 1:
        return MR_SPEAKER_MONO;
    case 2:
        return MR_SPEAKER_STEREO;
    case 3:
        return MR_SPEAKER_FRONT_LEFT | MR_SPEAKER_FRONT_CENTER | MR_SPEAKER_FRONT_RIGHT;
    case 4:
        return MR_SPEAKER_QUAD;
    case 5:
        return MR_SPEAKER_QUAD | MR_SPEAKER_FRONT_CENTER;
    case 6:
        /* The meaning of the PAWIN_SPEAKER_5POINT1 flag has changed over time:
            http://msdn2.microsoft.com/en-us/library/aa474707.aspx
           We use PAWIN_SPEAKER_5POINT1 (not PAWIN_SPEAKER_5POINT1_SURROUND)
           because on some cards (eg Audigy) PAWIN_SPEAKER_5POINT1_SURROUND
           results in a virtual mixdown placing the rear output in the
           front _and_ rear speakers.
        */
        return MR_SPEAKER_5POINT1;
    /* case 7: */
    case 8:
        /* PAWIN_SPEAKER_7POINT1_SURROUND fits normal surround sound setups better than PAWIN_SPEAKER_7POINT1, f.i. NVidia HDMI Audio
           output is silent on channels 5&6 with NVidia drivers, and channel 7&8 with Micrsoft HD Audio driver using PAWIN_SPEAKER_7POINT1.
           With PAWIN_SPEAKER_7POINT1_SURROUND both setups work OK. */
        return MR_SPEAKER_7POINT1_SURROUND;
    }

    /* Apparently some Audigy drivers will output silence
       if the direct-out constant (0) is used. So this is not ideal.
        NVidia driver seem to output garbage instead. Again not very ideal.
    */
    return  MR_SPEAKER_DIRECTOUT;
}
