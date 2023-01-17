/*
 * Mpeg Layer-1 audio decoder
 * --------------------------
 * copyright (c) 1995 by Michael Hipp, All rights reserved. See also 'README'
 * near unoptimzed ...
 *
 * may have a few bugs after last optimization ...
 *
 */

#include "mympg123.h"
#include "mympglib.h"
#include "MinMax.h"

void __fastcall mpg123::I_step_one(unsigned int balloc[], unsigned int scale_index[2][SBLIMIT])
{
    unsigned int *ba=balloc;
    unsigned int *sca = (unsigned int *) scale_index;

    if(fr.stereo)
    {
        int i;
        int jsbound = fr.jsbound;
        for (i=0; i<jsbound; i++)
        {
            *ba++ = ReservoirBuff->getbits_fast(4);
            *ba++ = ReservoirBuff->getbits_fast(4);
        }
        for (i=jsbound; i<SBLIMIT; i++)
            *ba++ = ReservoirBuff->getbits_fast(4);

        ba = balloc;

        for (i=0; i<jsbound; i++)
        {
            if ((*ba++))
                *sca++ = ReservoirBuff->getbits_fast(6);
            if ((*ba++))
                *sca++ = ReservoirBuff->getbits_fast(6);
        }
        for (i=jsbound; i<SBLIMIT; i++)
            if ((*ba++))
            {
                *sca++ =  ReservoirBuff->getbits_fast(6);
                *sca++ =  ReservoirBuff->getbits_fast(6);
            }
    }
    else
    {
        int i;
        for (i=0; i<SBLIMIT; i++)
            *ba++ = ReservoirBuff->getbits_fast(4);
        ba = balloc;
        for (i=0; i<SBLIMIT; i++)
            if ((*ba++))
                *sca++ = ReservoirBuff->getbits_fast(6);
    }
}

void __fastcall mpg123::I_step_two(
    real fraction[2][SBLIMIT],
    unsigned int balloc[2 * SBLIMIT],
    unsigned int scale_index[2][SBLIMIT])
{
    int i,n;
    int smpb[2*SBLIMIT]; /* values: 0-65535 */
    int *sample;
    /*register*/ unsigned int *ba;
    /*register*/ unsigned int *sca = (unsigned int *) scale_index;

    if(fr.stereo)
    {
        int jsbound = fr.jsbound;
        /*register*/ real *f0 = fraction[0];
        /*register*/ real *f1 = fraction[1];
        ba = balloc;
        for (sample=smpb,i=0; i<jsbound; i++)
        {
            if ((n = *ba++))
                *sample++ = ReservoirBuff->getbits(n+1);
            if ((n = *ba++))
                *sample++ = ReservoirBuff->getbits(n+1);
        }
        for (i=jsbound; i<SBLIMIT; i++)
            if ((n = *ba++))
                *sample++ = ReservoirBuff->getbits(n+1);

        ba = balloc;
        for (sample=smpb,i=0; i<jsbound; i++)
        {
            if((n=*ba++))
                *f0++ = (real) ( ((-1)<<n) + (*sample++) + 1) * muls[n+1][*sca++];
            else
                *f0++ = 0.0;
            if((n=*ba++))
                *f1++ = (real) ( ((-1)<<n) + (*sample++) + 1) * muls[n+1][*sca++];
            else
                *f1++ = 0.0;
        }
        for (i=jsbound; i<SBLIMIT; i++)
        {
            if ((n=*ba++))
            {
                real samp = ( ((-1)<<n) + (*sample++) + 1);
                *f0++ = samp * muls[n+1][*sca++];
                *f1++ = samp * muls[n+1][*sca++];
            }
            else
                *f0++ = *f1++ = 0.0;
        }
    }
    else
    {
        /*register*/ real *f0 = fraction[0];
        ba = balloc;
        for (sample=smpb,i=0; i<SBLIMIT; i++)
            if ((n = *ba++))
                *sample++ = ReservoirBuff->getbits(n+1);
        ba = balloc;
        for (sample=smpb,i=0; i<SBLIMIT; i++)
        {
            if((n=*ba++))
                *f0++ = (real) ( ((-1)<<n) + (*sample++) + 1) * muls[n+1][*sca++];
            else
                *f0++ = 0.0;
        }
    }
}

int __fastcall mpg123::do_layer1(unsigned char* pcm_sample, int* pcm_point)
{
    int i, stereo = fr.stereo;
    unsigned int balloc[2*SBLIMIT];
    unsigned int scale_index[2][SBLIMIT];
    real fraction[2][SBLIMIT];

    fr.jsbound = (fr.mode == MPG_MD_JOINT_STEREO) ? Min((fr.mode_ext << 2) + 4, SBLIMIT) : SBLIMIT;

    I_step_one(balloc,scale_index);

    int		p1 = *pcm_point;

    for(i = 0; i < SCALE_BLOCK; i++)
    {
        I_step_two(fraction, balloc, scale_index);

        if(stereo == 1)
        {
#if !defined(MPG123_USE_ONLY_FLOAT64) || defined(MPG123_USE_GAIN)
            (this->*synth_1to1_mono_func)(fraction[0], pcm_sample, pcm_point);
#else
            synth_1to1_mono_float64(fraction[0], pcm_sample, pcm_point);
#endif
        }
        else
        {
#if !defined(MPG123_USE_ONLY_FLOAT64) || defined(MPG123_USE_GAIN)
            (this->*synth_1to1_func)(fraction[0], 0, pcm_sample, &p1);
            (this->*synth_1to1_func)(fraction[1], 1, pcm_sample, pcm_point);
#else
            synth_1to1_float64(fraction[0], 0, pcm_sample, &p1);
            synth_1to1_float64(fraction[1], 1, pcm_sample, pcm_point);
#endif
        }
    }

    return MP3_OK;
}

