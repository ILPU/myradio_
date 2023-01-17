#ifndef WINDOWS_WAVEFORMAT_H_INCLUDED
#define WINDOWS_WAVEFORMAT_H_INCLUDED

//#include "../types.h"

/*
	The following #defines for speaker channel masks are the same
	as those in ksmedia.h, except with MR_ prepended, KSAUDIO_ removed
	in some cases, and casts to MRWinWaveFormatChannelMask added.
*/

typedef unsigned long MRWinWaveFormatChannelMask;

/* Speaker Positions: */
#define MR_SPEAKER_FRONT_LEFT				((MRWinWaveFormatChannelMask)0x1)
#define MR_SPEAKER_FRONT_RIGHT				((MRWinWaveFormatChannelMask)0x2)
#define MR_SPEAKER_FRONT_CENTER				((MRWinWaveFormatChannelMask)0x4)
#define MR_SPEAKER_LOW_FREQUENCY			((MRWinWaveFormatChannelMask)0x8)
#define MR_SPEAKER_BACK_LEFT				((MRWinWaveFormatChannelMask)0x10)
#define MR_SPEAKER_BACK_RIGHT				((MRWinWaveFormatChannelMask)0x20)
#define MR_SPEAKER_FRONT_LEFT_OF_CENTER		((MRWinWaveFormatChannelMask)0x40)
#define MR_SPEAKER_FRONT_RIGHT_OF_CENTER	((MRWinWaveFormatChannelMask)0x80)
#define MR_SPEAKER_BACK_CENTER				((MRWinWaveFormatChannelMask)0x100)
#define MR_SPEAKER_SIDE_LEFT				((MRWinWaveFormatChannelMask)0x200)
#define MR_SPEAKER_SIDE_RIGHT				((MRWinWaveFormatChannelMask)0x400)
#define MR_SPEAKER_TOP_CENTER				((MRWinWaveFormatChannelMask)0x800)
#define MR_SPEAKER_TOP_FRONT_LEFT			((MRWinWaveFormatChannelMask)0x1000)
#define MR_SPEAKER_TOP_FRONT_CENTER			((MRWinWaveFormatChannelMask)0x2000)
#define MR_SPEAKER_TOP_FRONT_RIGHT			((MRWinWaveFormatChannelMask)0x4000)
#define MR_SPEAKER_TOP_BACK_LEFT			((MRWinWaveFormatChannelMask)0x8000)
#define MR_SPEAKER_TOP_BACK_CENTER			((MRWinWaveFormatChannelMask)0x10000)
#define MR_SPEAKER_TOP_BACK_RIGHT			((MRWinWaveFormatChannelMask)0x20000)

/* Bit mask locations reserved for future use */
#define MR_SPEAKER_RESERVED					((MRWinWaveFormatChannelMask)0x7FFC0000)

/* Used to specify that any possible permutation of speaker configurations */
#define MR_SPEAKER_ALL						((MRWinWaveFormatChannelMask)0x80000000)

/* DirectSound Speaker Config */
#define MR_SPEAKER_DIRECTOUT					0
#define MR_SPEAKER_MONO						(MR_SPEAKER_FRONT_CENTER)
#define MR_SPEAKER_STEREO					(MR_SPEAKER_FRONT_LEFT | MR_SPEAKER_FRONT_RIGHT)
#define MR_SPEAKER_QUAD						(MR_SPEAKER_FRONT_LEFT | MR_SPEAKER_FRONT_RIGHT | \
												MR_SPEAKER_BACK_LEFT  | MR_SPEAKER_BACK_RIGHT)
#define MR_SPEAKER_SURROUND					(MR_SPEAKER_FRONT_LEFT | MR_SPEAKER_FRONT_RIGHT | \
												MR_SPEAKER_FRONT_CENTER | MR_SPEAKER_BACK_CENTER)
#define MR_SPEAKER_5POINT1					(MR_SPEAKER_FRONT_LEFT | MR_SPEAKER_FRONT_RIGHT | \
												MR_SPEAKER_FRONT_CENTER | MR_SPEAKER_LOW_FREQUENCY | \
												MR_SPEAKER_BACK_LEFT  | MR_SPEAKER_BACK_RIGHT)
#define MR_SPEAKER_7POINT1					(MR_SPEAKER_FRONT_LEFT | MR_SPEAKER_FRONT_RIGHT | \
												MR_SPEAKER_FRONT_CENTER | MR_SPEAKER_LOW_FREQUENCY | \
												MR_SPEAKER_BACK_LEFT | MR_SPEAKER_BACK_RIGHT | \
												MR_SPEAKER_FRONT_LEFT_OF_CENTER | MR_SPEAKER_FRONT_RIGHT_OF_CENTER)
#define MR_SPEAKER_5POINT1_SURROUND			(MR_SPEAKER_FRONT_LEFT | MR_SPEAKER_FRONT_RIGHT | \
												MR_SPEAKER_FRONT_CENTER | MR_SPEAKER_LOW_FREQUENCY | \
												MR_SPEAKER_SIDE_LEFT  | MR_SPEAKER_SIDE_RIGHT)
#define MR_SPEAKER_7POINT1_SURROUND			(MR_SPEAKER_FRONT_LEFT | MR_SPEAKER_FRONT_RIGHT | \
												MR_SPEAKER_FRONT_CENTER | MR_SPEAKER_LOW_FREQUENCY | \
												MR_SPEAKER_BACK_LEFT | MR_SPEAKER_BACK_RIGHT | \
												MR_SPEAKER_SIDE_LEFT | MR_SPEAKER_SIDE_RIGHT)
/*
 According to the Microsoft documentation:
 The following are obsolete 5.1 and 7.1 settings (they lack side speakers).  Note this means
 that the default 5.1 and 7.1 settings (KSAUDIO_SPEAKER_5POINT1 and KSAUDIO_SPEAKER_7POINT1 are
 similarly obsolete but are unchanged for compatibility reasons).
*/
#define MR_SPEAKER_5POINT1_BACK				MR_SPEAKER_5POINT1
#define MR_SPEAKER_7POINT1_WIDE				MR_SPEAKER_7POINT1

/* DVD Speaker Positions */
#define MR_SPEAKER_GROUND_FRONT_LEFT		MR_SPEAKER_FRONT_LEFT
#define MR_SPEAKER_GROUND_FRONT_CENTER		MR_SPEAKER_FRONT_CENTER
#define MR_SPEAKER_GROUND_FRONT_RIGHT		MR_SPEAKER_FRONT_RIGHT
#define MR_SPEAKER_GROUND_REAR_LEFT			MR_SPEAKER_BACK_LEFT
#define MR_SPEAKER_GROUND_REAR_RIGHT		MR_SPEAKER_BACK_RIGHT
#define MR_SPEAKER_TOP_MIDDLE				MR_SPEAKER_TOP_CENTER
#define MR_SPEAKER_SUPER_WOOFER				MR_SPEAKER_LOW_FREQUENCY


/*
	MR_WinWaveFormat is defined here to provide compatibility with
	compilation environments which don't have headers defining
	WAVEFORMATEXTENSIBLE (e.g. older versions of MSVC, Borland C++ etc.

	The fields for WAVEFORMATEX and WAVEFORMATEXTENSIBLE are declared as an
    unsigned char array here to avoid clients who include this file having
    a dependency on windows.h and mmsystem.h, and also to to avoid having
    to write separate packing pragmas for each compiler.
*/
#define MR_SIZEOF_WAVEFORMATEX          18
#define MR_SIZEOF_WAVEFORMATEXTENSIBLE  (MR_SIZEOF_WAVEFORMATEX + 22)

typedef struct
{
    unsigned char fields[MR_SIZEOF_WAVEFORMATEXTENSIBLE];
    unsigned long extraLongForAlignment; /* ensure that compiler aligns struct to DWORD */
} MRWinWaveFormat;

/*
    WAVEFORMATEXTENSIBLE fields:

    union  {
	    WORD  wValidBitsPerSample;
	    WORD  wSamplesPerBlock;
	    WORD  wReserved;
    } Samples;
    DWORD  dwChannelMask;
    GUID  SubFormat;
*/

#define MR_INDEXOF_WVALIDBITSPERSAMPLE      (MR_SIZEOF_WAVEFORMATEX+0)
#define MR_INDEXOF_DWCHANNELMASK			(MR_SIZEOF_WAVEFORMATEX+2)
#define MR_INDEXOF_SUBFORMAT				(MR_SIZEOF_WAVEFORMATEX+6)


/*
    Valid values to pass for the waveFormatTag MR_Win_InitializeWaveFormatEx and
    MR_Win_InitializeWaveFormatExtensible functions below. These must match
    the standard Windows WAVE_FORMAT_* values.
*/
#define MR_WAVE_FORMAT_PCM               (1)
#define MR_WAVE_FORMAT_IEEE_FLOAT        (3)
#define MR_WAVE_FORMAT_DOLBY_AC3_SPDIF   (0x0092)
#define MR_WAVE_FORMAT_WMA_SPDIF         (0x0164)


/*
    returns MR_WAVE_FORMAT_PCM or MR_WAVE_FORMAT_IEEE_FLOAT
    depending on the sampleFormat parameter.
*/
int MRWin_SampleFormatToLinearWaveFormatTag(MRSampleFormat sampleFormat);

/*
	Use the following two functions to initialize the waveformat structure.
*/

void MRWin_InitializeWaveFormatEx(MRWinWaveFormat *waveFormat,
                                  unsigned short numChannels, MRSampleFormat sampleFormat, int waveFormatTag, double sampleRate);


void MRWin_InitializeWaveFormatExtensible(MRWinWaveFormat *waveFormat,
        unsigned short numChannels, MRSampleFormat sampleFormat, int waveFormatTag, double sampleRate,
        MRWinWaveFormatChannelMask channelMask);


/* Map a channel count to a speaker channel mask */
MRWinWaveFormatChannelMask MRWin_DefaultChannelMask(unsigned short numChannels);

#endif // WINDOWS_WAVEFORMAT_H_INCLUDED
