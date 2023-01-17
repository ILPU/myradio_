
/*
 * Mpeg Layer-1,2,3 audio decoder
 * ------------------------------
 * copyright (c) 1995,1996,1997 by Michael Hipp, All rights reserved.
 * See also 'README'
 *
 * slighlty optimized for machines without autoincrement/decrement.
 * The performance is highly compiler dependend. Maybe
 * the decode.c version for 'normal' processor may be faster
 * even for Intel processors.
 */

#include "mympg123.h"
#include "mympglib.h"

inline void __fastcall WRITE_SAMPLE_FLOAT64(double* samples, real sum)
{
    *samples = sum;
}

#define	SUM_WINDOW1 \
	sum  = window[0x0] * b0[0x0];\
	sum -= window[0x1] * b0[0x1];\
	sum += window[0x2] * b0[0x2];\
	sum -= window[0x3] * b0[0x3];\
	sum += window[0x4] * b0[0x4];\
	sum -= window[0x5] * b0[0x5];\
	sum += window[0x6] * b0[0x6];\
	sum -= window[0x7] * b0[0x7];\
	sum += window[0x8] * b0[0x8];\
	sum -= window[0x9] * b0[0x9];\
	sum += window[0xA] * b0[0xA];\
	sum -= window[0xB] * b0[0xB];\
	sum += window[0xC] * b0[0xC];\
	sum -= window[0xD] * b0[0xD];\
	sum += window[0xE] * b0[0xE];\
	sum -= window[0xF] * b0[0xF];\
\
	WRITE_SAMPLE_FLOAT64(samples, sum);\
\
	b0 += 0x10;\
	window += 0x20;\
	samples += step;

#define	SUM_WINDOW2 \
	sum = -window[-0x1] * b0[0x0];\
	sum -= window[-0x2] * b0[0x1];\
	sum -= window[-0x3] * b0[0x2];\
	sum -= window[-0x4] * b0[0x3];\
	sum -= window[-0x5] * b0[0x4];\
	sum -= window[-0x6] * b0[0x5];\
	sum -= window[-0x7] * b0[0x6];\
	sum -= window[-0x8] * b0[0x7];\
	sum -= window[-0x9] * b0[0x8];\
	sum -= window[-0xA] * b0[0x9];\
	sum -= window[-0xB] * b0[0xA];\
	sum -= window[-0xC] * b0[0xB];\
	sum -= window[-0xD] * b0[0xC];\
	sum -= window[-0xE] * b0[0xD];\
	sum -= window[-0xF] * b0[0xE];\
	sum -= window[-0x0] * b0[0xF];\
\
	WRITE_SAMPLE_FLOAT64(samples, sum);\
\
	b0 -= 0x10;\
	window -= 0x20;\
	samples += step;

#define	SUM_WINDOW2_END \
	sum = -window[-0x1] * b0[0x0];\
	sum -= window[-0x2] * b0[0x1];\
	sum -= window[-0x3] * b0[0x2];\
	sum -= window[-0x4] * b0[0x3];\
	sum -= window[-0x5] * b0[0x4];\
	sum -= window[-0x6] * b0[0x5];\
	sum -= window[-0x7] * b0[0x6];\
	sum -= window[-0x8] * b0[0x7];\
	sum -= window[-0x9] * b0[0x8];\
	sum -= window[-0xA] * b0[0x9];\
	sum -= window[-0xB] * b0[0xA];\
	sum -= window[-0xC] * b0[0xB];\
	sum -= window[-0xD] * b0[0xC];\
	sum -= window[-0xE] * b0[0xD];\
	sum -= window[-0xF] * b0[0xE];\
	sum -= window[-0x0] * b0[0xF];\
\
	WRITE_SAMPLE_FLOAT64(samples, sum);

void __fastcall mpg123::synth_1to1_mono_float64(real* bandPtr, unsigned char* samples, int* pnt)
{
    double samples_tmp[64];
    double* tmp1 = samples_tmp;
    int i;
    int pnt1 = 0;

    synth_1to1_float64(bandPtr, 0, (unsigned char*)samples_tmp, &pnt1);
    samples += *pnt;

    for(i = 0; i < 32; i++)
    {
        *((double*)samples) = *tmp1;
        samples += 8;
        tmp1 += 2;
    }

    (*pnt) += 256;
}

void __fastcall mpg123::synth_1to1_float64(real* bandPtr, int channel, unsigned char* out, int* pnt)
{
    const int step = 2;
    double *samples = (double*)(out + *pnt);

    real *b0, (*buf)[0x110];
    int bo1;

    if(!channel)
    {
        synth_bo--;
        synth_bo &= 0xf;
        buf = synth_buffs[0];
    }
    else
    {
        samples++;
        buf = synth_buffs[1];
    }

    if(synth_bo & 0x1)
    {
        b0 = buf[0];
        bo1 = synth_bo;
        dct64(buf[1] + ((synth_bo + 1) & 0xf), buf[0] + synth_bo, bandPtr);
    }
    else
    {
        b0 = buf[1];
        bo1 = synth_bo + 1;
        dct64(buf[0] + synth_bo, buf[1] + synth_bo + 1, bandPtr);
    }

    real sum;
    real *window = decwin + 16 - bo1;

    SUM_WINDOW1  // 1
    SUM_WINDOW1  // 2
    SUM_WINDOW1  // 3
    SUM_WINDOW1  // 4
    SUM_WINDOW1  // 5
    SUM_WINDOW1  // 6
    SUM_WINDOW1  // 7
    SUM_WINDOW1  // 8
    SUM_WINDOW1  // 9
    SUM_WINDOW1  // 10
    SUM_WINDOW1  // 11
    SUM_WINDOW1  // 12
    SUM_WINDOW1  // 13
    SUM_WINDOW1  // 14
    SUM_WINDOW1  // 15
    SUM_WINDOW1  // 16

    sum  = window[0x0] * b0[0x0];
    sum += window[0x2] * b0[0x2];
    sum += window[0x4] * b0[0x4];
    sum += window[0x6] * b0[0x6];
    sum += window[0x8] * b0[0x8];
    sum += window[0xA] * b0[0xA];
    sum += window[0xC] * b0[0xC];
    sum += window[0xE] * b0[0xE];

    WRITE_SAMPLE_FLOAT64(samples, sum);

    b0 -= 0x10;
    window = window + (bo1 << 1) - 0x20;
    samples += step;

    SUM_WINDOW2  // 1
    SUM_WINDOW2  // 2
    SUM_WINDOW2  // 3
    SUM_WINDOW2  // 4
    SUM_WINDOW2  // 5
    SUM_WINDOW2  // 6
    SUM_WINDOW2  // 7
    SUM_WINDOW2  // 8
    SUM_WINDOW2  // 9
    SUM_WINDOW2  // 10
    SUM_WINDOW2  // 11
    SUM_WINDOW2  // 12
    SUM_WINDOW2  // 13
    SUM_WINDOW2  // 14
    SUM_WINDOW2_END  // 15

    (*pnt) += 512;
}

