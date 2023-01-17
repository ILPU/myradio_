#include "mympg123.h"
#include "mympglib.h"
#include "MinMax.h"

const struct parameter param = { 1, 1, 0, 0 };

const int tabsel_123[2][3][16] =
{
    {   {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},
        {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},
        {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,}
    },

    {   {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,},
        {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},
        {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,}
    }
};

const long freqs[9] = {44100, 48000, 32000,
                       22050, 24000, 16000,
                       11025, 12000, 8000
                      };

//int bitindex;
//unsigned char *wordpointer;
//unsigned char *pcm_sample;
//int pcm_point = 0;

/*
 * the code a header and write the information
 * into the frame structure
 */

bool __fastcall decode_header(frame* fr, unsigned long newhead)
{
    if(newhead & (1 << 20))
    {
        fr->mpeg25 = false;
        fr->lsf = (newhead & (1 << 19)) ? 0 : 1;
    }
    else
    {
        fr->mpeg25 = true;
        fr->lsf = 1;
    }

    const int	freq = (newhead >> 10) & 0x3;

    if(freq == 0x3)
    {
//		errorstring = "decode_header error 1";
//		fprintf(stderr,"Stream error\n");
//		exit(1);
        return false;
    }

    fr->lay                = 4 - ((newhead >> 17) & 3);
    fr->frequency          = freq + (fr->mpeg25 ? 6 : fr->lsf * 3);
    fr->error_protection   = ((newhead >> 16) & 0x1) ^ 0x1;
    fr->bitrate            = (newhead >> 12) & 0xf;
    fr->padding            = (newhead >> 9) & 0x1;
    fr->extension          = (newhead >> 8) & 0x1;
    fr->mode               = (newhead >> 6) & 0x3;
    fr->mode_ext           = (newhead >> 4) & 0x3;
    fr->copyright          = (newhead >> 3) & 0x1;
    fr->original           = (newhead >> 2) & 0x1;
    fr->emphasis           = newhead & 0x3;
    fr->stereo             = (fr->mode == MPG_MD_MONO) ? 1 : 2;

    if((fr->bitrate == 0x0) || (fr->bitrate == 0xf))
    {
//		errorstring = "free format not supported";
//		fprintf(stderr,"Free format not supported.\n");
        return false;
    }

    switch(fr->lay)
    {
    case 3:
        if(fr->lsf)
        {
            fr->sideInfoSize = (fr->stereo == 1) ? 9 : 17;
        }
        else
        {
            fr->sideInfoSize = (fr->stereo == 1) ? 17 : 32;
        }
//		if(fr->error_protection) fr->sideInfoSize += 2;
        fr->framesize = tabsel_123[fr->lsf][2][fr->bitrate] * 144000;
        fr->framesize /= freqs[fr->frequency] << fr->lsf;
        fr->framesize += fr->padding - 4;
        break;
    case 2:
        fr->sideInfoSize = 0;
        fr->framesize = tabsel_123[fr->lsf][1][fr->bitrate] * 144000;
        fr->framesize /= freqs[fr->frequency];
        fr->framesize += fr->padding - 4;
        break;
    case 1:
        fr->sideInfoSize = 0;
        fr->framesize = tabsel_123[fr->lsf][0][fr->bitrate] * 12000;
        fr->framesize /= freqs[fr->frequency];
        fr->framesize = ((fr->framesize + fr->padding) * 4) - 4;
        break;
    default:
//		errorstring = "Sorry, unknown layer type.";
        return false;
    }

    return true;
}

RingBuff_8bit::RingBuff_8bit(void)
{
    Buff = new unsigned char[1 << 8];

    Init();
}

RingBuff_8bit::~RingBuff_8bit(void)
{
    delete[] Buff;
}

void __fastcall RingBuff_8bit::Init(void)
{
//	memset(Buff, 0, 1 << 8);

    BuffWrite = 0;
    BuffEnd = 0;
}

void __fastcall RingBuff_8bit::Add(unsigned char* AddBuff, const unsigned int Size)
{
    const unsigned int	MaxCopy = BuffEnd + Size;

    if(MaxCopy <= (1 << 8))
    {
        memcpy(Buff + BuffEnd, AddBuff, Size);
    }
    else
    {
        const unsigned int	CopySize1 = (1 << 8) - BuffEnd;
        const unsigned int	CopySize2 = MaxCopy - (1 << 8);

        memcpy(Buff + BuffEnd, AddBuff, CopySize1);
        memcpy(Buff, AddBuff + CopySize1, CopySize2);
    }

    BuffEnd = MaxCopy;
    BuffWrite = Min(BuffWrite + Size, (1U << 8));
}

RingBuff_16bit::RingBuff_16bit(void)
{
    Buff = new unsigned char[1 << 16];

    Init();
}

RingBuff_16bit::~RingBuff_16bit(void)
{
    delete[] Buff;
}

void __fastcall RingBuff_16bit::Init(void)
{
    memset(Buff, 0, 1 << 16);

    BuffWrite = 0;
    BuffEnd = 0;
}

void __fastcall RingBuff_16bit::Add(unsigned char* AddBuff, const unsigned int Size)
{
    const unsigned int	MaxCopy = BuffEnd + Size;

    if(MaxCopy <= (1 << 16))
    {
        memcpy(Buff + BuffEnd, AddBuff, Size);
    }
    else
    {
        const unsigned int	CopySize1 = (1 << 16) - BuffEnd;
        const unsigned int	CopySize2 = MaxCopy - (1 << 16);

        memcpy(Buff + BuffEnd, AddBuff, CopySize1);
        memcpy(Buff, AddBuff + CopySize1, CopySize2);
    }

    BuffEnd = MaxCopy;
    BuffWrite = Min(BuffWrite + Size, (1U << 16));
}

