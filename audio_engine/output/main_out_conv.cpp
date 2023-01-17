
#include "main_out_conv.h"
#include "main_out_dither.h"

#define MR_LITTLE_ENDIAN

//typedef signed short signed short;
//typedef unsigned short unsigned short;
//typedef signed int signed int;
//typedef unsigned int unsigned int;


#define MR_SELECT_FORMAT_( format, float32, int32, int24, int16, int8, uint8 ) \
    switch( format & ~mrNonInterleaved ){                                      \
    case mrFloat32:                                                            \
        float32                                                                \
    case mrInt32:                                                              \
        int32                                                                  \
    case mrInt24:                                                              \
        int24                                                                  \
    case mrInt16:                                                              \
        int16                                                                  \
    case mrInt8:                                                               \
        int8                                                                   \
    case mrUInt8:                                                              \
        uint8                                                                  \
    default: return 0;                                                         \
    }

#define MR_SELECT_CONVERTER_DITHER_CLIP_( flags, source, destination )         \
    if( flags & mrClipOff ){ /* no clip */                                     \
        if( flags & mrDitherOff ){ /* no dither */                             \
            return mrConverters. source ## _To_ ## destination;                \
        }else{ /* dither */                                                    \
            return mrConverters. source ## _To_ ## destination ## _Dither;     \
        }                                                                      \
    }else{ /* clip */                                                          \
        if( flags & mrDitherOff ){ /* no dither */                             \
            return mrConverters. source ## _To_ ## destination ## _Clip;       \
        }else{ /* dither */                                                    \
            return mrConverters. source ## _To_ ## destination ## _DitherClip; \
        }                                                                      \
    }

#define MR_SELECT_CONVERTER_DITHER_( flags, source, destination )              \
    if( flags & mrDitherOff ){ /* no dither */                                 \
        return mrConverters. source ## _To_ ## destination;                    \
    }else{ /* dither */                                                        \
        return mrConverters. source ## _To_ ## destination ## _Dither;         \
    }

#define MR_USE_CONVERTER_( source, destination )\
    return mrConverters. source ## _To_ ## destination;

#define MR_UNITY_CONVERSION_( wordlength )\
    return mrConverters. Copy_ ## wordlength ## _To_ ## wordlength;

MRUtilConverter* MRUtil_SelectConverter(MRSampleFormat sourceFormat,
                                        MRSampleFormat destinationFormat, unsigned long flags)
{
    MR_SELECT_FORMAT_( sourceFormat,
                       /* paFloat32: */
                       MR_SELECT_FORMAT_( destinationFormat,
                                          /* paFloat32: */        MR_UNITY_CONVERSION_( 32 ),
                                          /* signed int: */          MR_SELECT_CONVERTER_DITHER_CLIP_( flags, Float32, Int32 ),
                                          /* paInt24: */          MR_SELECT_CONVERTER_DITHER_CLIP_( flags, Float32, Int24 ),
                                          /* signed short: */          MR_SELECT_CONVERTER_DITHER_CLIP_( flags, Float32, Int16 ),
                                          /* paInt8: */           MR_SELECT_CONVERTER_DITHER_CLIP_( flags, Float32, Int8 ),
                                          /* paUInt8: */          MR_SELECT_CONVERTER_DITHER_CLIP_( flags, Float32, UInt8 )
                                        ),
                       /* signed int: */
                       MR_SELECT_FORMAT_( destinationFormat,
                                          /* paFloat32: */        MR_USE_CONVERTER_( Int32, Float32 ),
                                          /* signed int: */          MR_UNITY_CONVERSION_( 32 ),
                                          /* paInt24: */          MR_SELECT_CONVERTER_DITHER_( flags, Int32, Int24 ),
                                          /* signed short: */          MR_SELECT_CONVERTER_DITHER_( flags, Int32, Int16 ),
                                          /* paInt8: */           MR_SELECT_CONVERTER_DITHER_( flags, Int32, Int8 ),
                                          /* paUInt8: */          MR_SELECT_CONVERTER_DITHER_( flags, Int32, UInt8 )
                                        ),
                       /* paInt24: */
                       MR_SELECT_FORMAT_( destinationFormat,
                                          /* paFloat32: */        MR_USE_CONVERTER_( Int24, Float32 ),
                                          /* signed int: */          MR_USE_CONVERTER_( Int24, Int32 ),
                                          /* paInt24: */          MR_UNITY_CONVERSION_( 24 ),
                                          /* signed short: */          MR_SELECT_CONVERTER_DITHER_( flags, Int24, Int16 ),
                                          /* paInt8: */           MR_SELECT_CONVERTER_DITHER_( flags, Int24, Int8 ),
                                          /* paUInt8: */          MR_SELECT_CONVERTER_DITHER_( flags, Int24, UInt8 )
                                        ),
                       /* signed short: */
                       MR_SELECT_FORMAT_( destinationFormat,
                                          /* paFloat32: */        MR_USE_CONVERTER_( Int16, Float32 ),
                                          /* signed int: */          MR_USE_CONVERTER_( Int16, Int32 ),
                                          /* paInt24: */          MR_USE_CONVERTER_( Int16, Int24 ),
                                          /* signed short: */          MR_UNITY_CONVERSION_( 16 ),
                                          /* paInt8: */           MR_SELECT_CONVERTER_DITHER_( flags, Int16, Int8 ),
                                          /* paUInt8: */          MR_SELECT_CONVERTER_DITHER_( flags, Int16, UInt8 )
                                        ),
                       /* paInt8: */
                       MR_SELECT_FORMAT_( destinationFormat,
                                          /* paFloat32: */        MR_USE_CONVERTER_( Int8, Float32 ),
                                          /* signed int: */          MR_USE_CONVERTER_( Int8, Int32 ),
                                          /* paInt24: */          MR_USE_CONVERTER_( Int8, Int24 ),
                                          /* signed short: */          MR_USE_CONVERTER_( Int8, Int16 ),
                                          /* paInt8: */           MR_UNITY_CONVERSION_( 8 ),
                                          /* paUInt8: */          MR_USE_CONVERTER_( Int8, UInt8 )
                                        ),
                       /* paUInt8: */
                       MR_SELECT_FORMAT_( destinationFormat,
                                          /* paFloat32: */        MR_USE_CONVERTER_( UInt8, Float32 ),
                                          /* signed int: */          MR_USE_CONVERTER_( UInt8, Int32 ),
                                          /* paInt24: */          MR_USE_CONVERTER_( UInt8, Int24 ),
                                          /* signed short: */          MR_USE_CONVERTER_( UInt8, Int16 ),
                                          /* paInt8: */           MR_USE_CONVERTER_( UInt8, Int8 ),
                                          /* paUInt8: */          MR_UNITY_CONVERSION_( 8 )
                                        )
                     )
}


MRUtilZeroer* MRUtil_SelectZeroer(MRSampleFormat destinationFormat)
{

}

#define MR_CLIP_( val, min, max )\
    { val = ((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)); }

static const float const_1_div_128_ = 1.0f / 128.0f;  /* 8 bit multiplier */
static const float const_1_div_32768_ = 1.0f / 32768.f; /* 16 bit multiplier */
static const double const_1_div_2147483648_ = 1.0 / 2147483648.0; /* 32 bit multiplier */

static void Float32_To_Int32(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    signed int *dest =  (signed int*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        /* REVIEW */
#ifdef MR_USE_C99_LRINTF
        float scaled = *src * 0x7FFFFFFF;
        *dest = lrintf(scaled-0.5f);
#else
        double scaled = *src * 0x7FFFFFFF;
        *dest = (signed int) scaled;
#endif

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int32_Dither(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    signed int *dest =  (signed int*)destinationBuffer;

    while( count-- )
    {
        /* REVIEW */
#ifdef MR_USE_C99_LRINTF
        float dither  = MRUtil_GenerateFloatTriangularDither( ditherGenerator );
        /* use smaller scaler to prevent overflow when we add the dither */
        float dithered = ((float)*src * (2147483646.0f)) + dither;
        *dest = lrintf(dithered - 0.5f);
#else
        double dither  = MRUtil_GenerateFloatTriangularDither( ditherGenerator );
        /* use smaller scaler to prevent overflow when we add the dither */
        double dithered = ((double)*src * (2147483646.0)) + dither;
        *dest = (signed int) dithered;
#endif
        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int32_Clip(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    signed int *dest =  (signed int*)destinationBuffer;
    (void) ditherGenerator; /* unused parameter */

    while( count-- )
    {
        /* REVIEW */
#ifdef MR_USE_C99_LRINTF
        float scaled = *src * 0x7FFFFFFF;
        MR_CLIP_( scaled, -2147483648.f, 2147483647.f  );
        *dest = lrintf(scaled-0.5f);
#else
        double scaled = *src * 0x7FFFFFFF;
        MR_CLIP_( scaled, -2147483648., 2147483647.  );
        *dest = (signed int) scaled;
#endif

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int32_DitherClip(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    signed int *dest =  (signed int*)destinationBuffer;

    while( count-- )
    {
        /* REVIEW */
#ifdef MR_USE_C99_LRINTF
        float dither  = MRUtil_GenerateFloatTriangularDither( ditherGenerator );
        /* use smaller scaler to prevent overflow when we add the dither */
        float dithered = ((float)*src * (2147483646.0f)) + dither;
        MR_CLIP_( dithered, -2147483648.f, 2147483647.f  );
        *dest = lrintf(dithered-0.5f);
#else
        double dither  = MRUtil_GenerateFloatTriangularDither( ditherGenerator );
        /* use smaller scaler to prevent overflow when we add the dither */
        double dithered = ((double)*src * (2147483646.0)) + dither;
        MR_CLIP_( dithered, -2147483648., 2147483647.  );
        *dest = (signed int) dithered;
#endif

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int24(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    unsigned char *dest = (unsigned char*)destinationBuffer;
    signed int temp;

    (void) ditherGenerator; /* unused parameter */

    while( count-- )
    {
        /* convert to 32 bit and drop the low 8 bits */
        double scaled = (double)(*src) * 2147483647.0;
        temp = (signed int) scaled;

#if defined(MR_LITTLE_ENDIAN)
        dest[0] = (unsigned char)(temp >> 8);
        dest[1] = (unsigned char)(temp >> 16);
        dest[2] = (unsigned char)(temp >> 24);
#elif defined(MR_BIG_ENDIAN)
        dest[0] = (unsigned char)(temp >> 24);
        dest[1] = (unsigned char)(temp >> 16);
        dest[2] = (unsigned char)(temp >> 8);
#endif

        src += sourceStride;
        dest += destinationStride * 3;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int24_Dither(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    unsigned char *dest = (unsigned char*)destinationBuffer;
    signed int temp;

    while( count-- )
    {
        /* convert to 32 bit and drop the low 8 bits */

        double dither  = MRUtil_GenerateFloatTriangularDither( ditherGenerator );
        /* use smaller scaler to prevent overflow when we add the dither */
        double dithered = ((double)*src * (2147483646.0)) + dither;

        temp = (signed int) dithered;

#if defined(MR_LITTLE_ENDIAN)
        dest[0] = (unsigned char)(temp >> 8);
        dest[1] = (unsigned char)(temp >> 16);
        dest[2] = (unsigned char)(temp >> 24);
#elif defined(MR_BIG_ENDIAN)
        dest[0] = (unsigned char)(temp >> 24);
        dest[1] = (unsigned char)(temp >> 16);
        dest[2] = (unsigned char)(temp >> 8);
#endif

        src += sourceStride;
        dest += destinationStride * 3;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int24_Clip(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    unsigned char *dest = (unsigned char*)destinationBuffer;
    signed int temp;

    (void) ditherGenerator; /* unused parameter */

    while( count-- )
    {
        /* convert to 32 bit and drop the low 8 bits */
        double scaled = *src * 0x7FFFFFFF;
        MR_CLIP_( scaled, -2147483648., 2147483647.  );
        temp = (signed int) scaled;

#if defined(MR_LITTLE_ENDIAN)
        dest[0] = (unsigned char)(temp >> 8);
        dest[1] = (unsigned char)(temp >> 16);
        dest[2] = (unsigned char)(temp >> 24);
#elif defined(MR_BIG_ENDIAN)
        dest[0] = (unsigned char)(temp >> 24);
        dest[1] = (unsigned char)(temp >> 16);
        dest[2] = (unsigned char)(temp >> 8);
#endif

        src += sourceStride;
        dest += destinationStride * 3;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int24_DitherClip(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    unsigned char *dest = (unsigned char*)destinationBuffer;
    signed int temp;

    while( count-- )
    {
        /* convert to 32 bit and drop the low 8 bits */

        double dither  = MRUtil_GenerateFloatTriangularDither( ditherGenerator );
        /* use smaller scaler to prevent overflow when we add the dither */
        double dithered = ((double)*src * (2147483646.0)) + dither;
        MR_CLIP_( dithered, -2147483648., 2147483647.  );

        temp = (signed int) dithered;

#if defined(MR_LITTLE_ENDIAN)
        dest[0] = (unsigned char)(temp >> 8);
        dest[1] = (unsigned char)(temp >> 16);
        dest[2] = (unsigned char)(temp >> 24);
#elif defined(MR_BIG_ENDIAN)
        dest[0] = (unsigned char)(temp >> 24);
        dest[1] = (unsigned char)(temp >> 16);
        dest[2] = (unsigned char)(temp >> 8);
#endif

        src += sourceStride;
        dest += destinationStride * 3;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int16(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    signed short *dest =  (signed short*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
#ifdef MR_USE_C99_LRINTF
        float tempf = (*src * (32767.0f)) ;
        *dest = lrintf(tempf-0.5f);
#else
        short samp = (short) (*src * (32767.0f));
        *dest = samp;
#endif

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int16_Dither(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    signed short *dest = (signed short*)destinationBuffer;

    while( count-- )
    {

        float dither  = MRUtil_GenerateFloatTriangularDither( ditherGenerator );
        /* use smaller scaler to prevent overflow when we add the dither */
        float dithered = (*src * (32766.0f)) + dither;

#ifdef MR_USE_C99_LRINTF
        *dest = lrintf(dithered-0.5f);
#else
        *dest = (signed short) dithered;
#endif

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int16_Clip(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    signed short *dest =  (signed short*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
#ifdef MR_USE_C99_LRINTF
        long samp = lrintf((*src * (32767.0f)) -0.5f);
#else
        long samp = (signed int) (*src * (32767.0f));
#endif
        MR_CLIP_( samp, -0x8000, 0x7FFF );
        *dest = (signed short) samp;

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int16_DitherClip(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    signed short *dest =  (signed short*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {

        float dither  = MRUtil_GenerateFloatTriangularDither( ditherGenerator );
        /* use smaller scaler to prevent overflow when we add the dither */
        float dithered = (*src * (32766.0f)) + dither;
        signed int samp = (signed int) dithered;
        MR_CLIP_( samp, -0x8000, 0x7FFF );
#ifdef MR_USE_C99_LRINTF
        *dest = lrintf(samp-0.5f);
#else
        *dest = (signed short) samp;
#endif

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int8(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    signed char *dest =  (signed char*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        signed char samp = (signed char) (*src * (127.0f));
        *dest = samp;

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int8_Dither(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    signed char *dest =  (signed char*)destinationBuffer;

    while( count-- )
    {
        float dither  = MRUtil_GenerateFloatTriangularDither( ditherGenerator );
        /* use smaller scaler to prevent overflow when we add the dither */
        float dithered = (*src * (126.0f)) + dither;
        signed int samp = (signed int) dithered;
        *dest = (signed char) samp;

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int8_Clip(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    signed char *dest =  (signed char*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        signed int samp = (signed int)(*src * (127.0f));
        MR_CLIP_( samp, -0x80, 0x7F );
        *dest = (signed char) samp;

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int8_DitherClip(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    signed char *dest =  (signed char*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        float dither  = MRUtil_GenerateFloatTriangularDither( ditherGenerator );
        /* use smaller scaler to prevent overflow when we add the dither */
        float dithered = (*src * (126.0f)) + dither;
        signed int samp = (signed int) dithered;
        MR_CLIP_( samp, -0x80, 0x7F );
        *dest = (signed char) samp;

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_UInt8(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    unsigned char *dest =  (unsigned char*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        unsigned char samp = (unsigned char)(128 + ((unsigned char) (*src * (127.0f))));
        *dest = samp;

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_UInt8_Dither(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    unsigned char *dest =  (unsigned char*)destinationBuffer;

    while( count-- )
    {
        float dither  = MRUtil_GenerateFloatTriangularDither( ditherGenerator );
        /* use smaller scaler to prevent overflow when we add the dither */
        float dithered = (*src * (126.0f)) + dither;
        signed int samp = (signed int) dithered;
        *dest = (unsigned char) (128 + samp);

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_UInt8_Clip(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    unsigned char *dest =  (unsigned char*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        signed int samp = 128 + (signed int)(*src * (127.0f));
        MR_CLIP_( samp, 0x0000, 0x00FF );
        *dest = (unsigned char) samp;

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_UInt8_DitherClip(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    float *src = (float*)sourceBuffer;
    unsigned char *dest =  (unsigned char*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        float dither  = MRUtil_GenerateFloatTriangularDither( ditherGenerator );
        /* use smaller scaler to prevent overflow when we add the dither */
        float dithered = (*src * (126.0f)) + dither;
        signed int samp = 128 + (signed int) dithered;
        MR_CLIP_( samp, 0x0000, 0x00FF );
        *dest = (unsigned char) samp;

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int32_To_Float32(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed int *src = (signed int*)sourceBuffer;
    float *dest =  (float*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        *dest = (float) ((double)*src * const_1_div_2147483648_);

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int32_To_Int24(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed int *src    = (signed int*)sourceBuffer;
    unsigned char *dest = (unsigned char*)destinationBuffer;
    (void) ditherGenerator; /* unused parameter */

    while( count-- )
    {
        /* REVIEW */
#if defined(MR_LITTLE_ENDIAN)
        dest[0] = (unsigned char)(*src >> 8);
        dest[1] = (unsigned char)(*src >> 16);
        dest[2] = (unsigned char)(*src >> 24);
#elif defined(MR_BIG_ENDIAN)
        dest[0] = (unsigned char)(*src >> 24);
        dest[1] = (unsigned char)(*src >> 16);
        dest[2] = (unsigned char)(*src >> 8);
#endif
        src += sourceStride;
        dest += destinationStride * 3;
    }
}

/* -------------------------------------------------------------------------- */

static void Int32_To_Int24_Dither(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    (void) destinationBuffer; /* unused parameters */
    (void) destinationStride; /* unused parameters */
    (void) sourceBuffer; /* unused parameters */
    (void) sourceStride; /* unused parameters */
    (void) count; /* unused parameters */
    (void) ditherGenerator; /* unused parameters */
    /* IMPLEMENT ME */
}

/* -------------------------------------------------------------------------- */

static void Int32_To_Int16(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed int *src = (signed int*)sourceBuffer;
    signed short *dest =  (signed short*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        *dest = (signed short) ((*src) >> 16);

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int32_To_Int16_Dither(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed int *src = (signed int*)sourceBuffer;
    signed short *dest =  (signed short*)destinationBuffer;
    signed int dither;

    while( count-- )
    {
        /* REVIEW */
        dither = MRUtil_Generate16BitTriangularDither( ditherGenerator );
        *dest = (signed short) ((((*src)>>1) + dither) >> 15);

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int32_To_Int8(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed int *src = (signed int*)sourceBuffer;
    signed char *dest =  (signed char*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        *dest = (signed char) ((*src) >> 24);

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int32_To_Int8_Dither(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed int *src = (signed int*)sourceBuffer;
    signed char *dest =  (signed char*)destinationBuffer;
    signed int dither;

    while( count-- )
    {
        /* REVIEW */
        dither = MRUtil_Generate16BitTriangularDither( ditherGenerator );
        *dest = (signed char) ((((*src)>>1) + dither) >> 23);

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int32_To_UInt8(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed int *src = (signed int*)sourceBuffer;
    unsigned char *dest =  (unsigned char*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        (*dest) = (unsigned char)(((*src) >> 24) + 128);

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int32_To_UInt8_Dither(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed int *src = (signed int*)sourceBuffer;
    unsigned char *dest =  (unsigned char*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        /* IMPLEMENT ME */

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int24_To_Float32(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    unsigned char *src = (unsigned char*)sourceBuffer;
    float *dest = (float*)destinationBuffer;
    signed int temp;

    (void) ditherGenerator; /* unused parameter */

    while( count-- )
    {

#if defined(MR_LITTLE_ENDIAN)
        temp = (((signed int)src[0]) << 8);
        temp = temp | (((signed int)src[1]) << 16);
        temp = temp | (((signed int)src[2]) << 24);
#elif defined(MR_BIG_ENDIAN)
        temp = (((signed int)src[0]) << 24);
        temp = temp | (((signed int)src[1]) << 16);
        temp = temp | (((signed int)src[2]) << 8);
#endif

        *dest = (float) ((double)temp * const_1_div_2147483648_);

        src += sourceStride * 3;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int24_To_Int32(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    unsigned char *src  = (unsigned char*)sourceBuffer;
    signed int *dest = (signed int*)  destinationBuffer;
    signed int temp;

    (void) ditherGenerator; /* unused parameter */

    while( count-- )
    {

#if defined(MR_LITTLE_ENDIAN)
        temp = (((signed int)src[0]) << 8);
        temp = temp | (((signed int)src[1]) << 16);
        temp = temp | (((signed int)src[2]) << 24);
#elif defined(MR_BIG_ENDIAN)
        temp = (((signed int)src[0]) << 24);
        temp = temp | (((signed int)src[1]) << 16);
        temp = temp | (((signed int)src[2]) << 8);
#endif

        *dest = temp;

        src += sourceStride * 3;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int24_To_Int16(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    unsigned char *src = (unsigned char*)sourceBuffer;
    signed short *dest = (signed short*)destinationBuffer;

    signed short temp;

    (void) ditherGenerator; /* unused parameter */

    while( count-- )
    {

#if defined(MR_LITTLE_ENDIAN)
        /* src[0] is discarded */
        temp = (((signed short)src[1]));
        temp = temp | (signed short)(((signed short)src[2]) << 8);
#elif defined(MR_BIG_ENDIAN)
        /* src[2] is discarded */
        temp = (signed short)(((signed short)src[0]) << 8);
        temp = temp | (((signed short)src[1]));
#endif

        *dest = temp;

        src += sourceStride * 3;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int24_To_Int16_Dither(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    unsigned char *src = (unsigned char*)sourceBuffer;
    signed short *dest = (signed short*)destinationBuffer;

    signed int temp, dither;

    while( count-- )
    {

#if defined(MR_LITTLE_ENDIAN)
        temp = (((signed int)src[0]) << 8);
        temp = temp | (((signed int)src[1]) << 16);
        temp = temp | (((signed int)src[2]) << 24);
#elif defined(MR_BIG_ENDIAN)
        temp = (((signed int)src[0]) << 24);
        temp = temp | (((signed int)src[1]) << 16);
        temp = temp | (((signed int)src[2]) << 8);
#endif

        /* REVIEW */
        dither = MRUtil_Generate16BitTriangularDither( ditherGenerator );
        *dest = (signed short) (((temp >> 1) + dither) >> 15);

        src  += sourceStride * 3;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int24_To_Int8(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    unsigned char *src = (unsigned char*)sourceBuffer;
    signed char  *dest = (signed char*)destinationBuffer;

    (void) ditherGenerator; /* unused parameter */

    while( count-- )
    {

#if defined(MR_LITTLE_ENDIAN)
        /* src[0] is discarded */
        /* src[1] is discarded */
        *dest = src[2];
#elif defined(MR_BIG_ENDIAN)
        /* src[2] is discarded */
        /* src[1] is discarded */
        *dest = src[0];
#endif

        src += sourceStride * 3;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int24_To_Int8_Dither(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    unsigned char *src = (unsigned char*)sourceBuffer;
    signed char  *dest = (signed char*)destinationBuffer;

    signed int temp, dither;

    while( count-- )
    {

#if defined(MR_LITTLE_ENDIAN)
        temp = (((signed int)src[0]) << 8);
        temp = temp | (((signed int)src[1]) << 16);
        temp = temp | (((signed int)src[2]) << 24);
#elif defined(MR_BIG_ENDIAN)
        temp = (((signed int)src[0]) << 24);
        temp = temp | (((signed int)src[1]) << 16);
        temp = temp | (((signed int)src[2]) << 8);
#endif

        /* REVIEW */
        dither = MRUtil_Generate16BitTriangularDither( ditherGenerator );
        *dest = (signed char) (((temp >> 1) + dither) >> 23);

        src += sourceStride * 3;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int24_To_UInt8(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    unsigned char *src = (unsigned char*)sourceBuffer;
    unsigned char *dest = (unsigned char*)destinationBuffer;

    (void) ditherGenerator; /* unused parameter */

    while( count-- )
    {

#if defined(MR_LITTLE_ENDIAN)
        /* src[0] is discarded */
        /* src[1] is discarded */
        *dest = (unsigned char)(src[2] + 128);
#elif defined(MR_BIG_ENDIAN)
        *dest = (unsigned char)(src[0] + 128);
        /* src[1] is discarded */
        /* src[2] is discarded */
#endif

        src += sourceStride * 3;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int24_To_UInt8_Dither(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    (void) destinationBuffer; /* unused parameters */
    (void) destinationStride; /* unused parameters */
    (void) sourceBuffer; /* unused parameters */
    (void) sourceStride; /* unused parameters */
    (void) count; /* unused parameters */
    (void) ditherGenerator; /* unused parameters */
    /* IMPLEMENT ME */
}

/* -------------------------------------------------------------------------- */

static void Int16_To_Float32(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed short *src = (signed short*)sourceBuffer;
    float *dest =  (float*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        float samp = *src * const_1_div_32768_; /* FIXME: i'm concerned about this being asymetrical with float->int16 -rb */
        *dest = samp;

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int16_To_Int32(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed short *src = (signed short*)sourceBuffer;
    signed int *dest =  (signed int*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        /* REVIEW: we should consider something like
            (*src << 16) | (*src & 0xFFFF)
        */

        *dest = *src << 16;

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int16_To_Int24(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed short *src   = (signed short*) sourceBuffer;
    unsigned char *dest = (unsigned char*)destinationBuffer;
    signed short temp;

    (void) ditherGenerator; /* unused parameter */

    while( count-- )
    {
        temp = *src;

#if defined(MR_LITTLE_ENDIAN)
        dest[0] = 0;
        dest[1] = (unsigned char)(temp);
        dest[2] = (unsigned char)(temp >> 8);
#elif defined(MR_BIG_ENDIAN)
        dest[0] = (unsigned char)(temp >> 8);
        dest[1] = (unsigned char)(temp);
        dest[2] = 0;
#endif

        src += sourceStride;
        dest += destinationStride * 3;
    }
}

/* -------------------------------------------------------------------------- */

static void Int16_To_Int8(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed short *src = (signed short*)sourceBuffer;
    signed char *dest =  (signed char*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        (*dest) = (signed char)((*src) >> 8);

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int16_To_Int8_Dither(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed short *src = (signed short*)sourceBuffer;
    signed char *dest =  (signed char*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        /* IMPLEMENT ME */

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int16_To_UInt8(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed short *src = (signed short*)sourceBuffer;
    unsigned char *dest =  (unsigned char*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        (*dest) = (unsigned char)(((*src) >> 8) + 128);

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int16_To_UInt8_Dither(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed short *src = (signed short*)sourceBuffer;
    unsigned char *dest =  (unsigned char*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        /* IMPLEMENT ME */

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int8_To_Float32(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed char *src = (signed char*)sourceBuffer;
    float *dest =  (float*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        float samp = *src * const_1_div_128_;
        *dest = samp;

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int8_To_Int32(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed char *src = (signed char*)sourceBuffer;
    signed int *dest =  (signed int*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        (*dest) = (*src) << 24;

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int8_To_Int24(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed char *src = (signed char*)sourceBuffer;
    unsigned char *dest =  (unsigned char*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {

#if defined(MR_LITTLE_ENDIAN)
        dest[0] = 0;
        dest[1] = 0;
        dest[2] = (*src);
#elif defined(MR_BIG_ENDIAN)
        dest[0] = (*src);
        dest[1] = 0;
        dest[2] = 0;
#endif

        src += sourceStride;
        dest += destinationStride * 3;
    }
}

/* -------------------------------------------------------------------------- */

static void Int8_To_Int16(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed char *src = (signed char*)sourceBuffer;
    signed short *dest =  (signed short*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        (*dest) = (signed short)((*src) << 8);

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Int8_To_UInt8(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    signed char *src = (signed char*)sourceBuffer;
    unsigned char *dest =  (unsigned char*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        (*dest) = (unsigned char)(*src + 128);

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void UInt8_To_Float32(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    unsigned char *src = (unsigned char*)sourceBuffer;
    float *dest =  (float*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        float samp = (*src - 128) * const_1_div_128_;
        *dest = samp;

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void UInt8_To_Int32(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    unsigned char *src = (unsigned char*)sourceBuffer;
    signed int *dest = (signed int*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        (*dest) = (*src - 128) << 24;

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void UInt8_To_Int24(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    unsigned char *src  = (unsigned char*)sourceBuffer;
    unsigned char *dest = (unsigned char*)destinationBuffer;
    (void) ditherGenerator; /* unused parameters */

    while( count-- )
    {

#if defined(MR_LITTLE_ENDIAN)
        dest[0] = 0;
        dest[1] = 0;
        dest[2] = (unsigned char)(*src - 128);
#elif defined(MR_BIG_ENDIAN)
        dest[0] = (unsigned char)(*src - 128);
        dest[1] = 0;
        dest[2] = 0;
#endif

        src += sourceStride;
        dest += destinationStride * 3;
    }
}

/* -------------------------------------------------------------------------- */

static void UInt8_To_Int16(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    unsigned char *src = (unsigned char*)sourceBuffer;
    signed short *dest =  (signed short*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        (*dest) = (signed short)((*src - 128) << 8);

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void UInt8_To_Int8(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    unsigned char *src = (unsigned char*)sourceBuffer;
    signed char  *dest = (signed char*)destinationBuffer;
    (void)ditherGenerator; /* unused parameter */

    while( count-- )
    {
        (*dest) = (signed char)(*src - 128);

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Copy_8_To_8(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    unsigned char *src = (unsigned char*)sourceBuffer;
    unsigned char *dest = (unsigned char*)destinationBuffer;

    (void) ditherGenerator; /* unused parameter */

    while( count-- )
    {
        *dest = *src;

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Copy_16_To_16(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    unsigned short *src = (unsigned short *)sourceBuffer;
    unsigned short *dest = (unsigned short *)destinationBuffer;

    (void) ditherGenerator; /* unused parameter */

    while( count-- )
    {
        *dest = *src;

        src += sourceStride;
        dest += destinationStride;
    }
}

/* -------------------------------------------------------------------------- */

static void Copy_24_To_24(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    unsigned char *src = (unsigned char*)sourceBuffer;
    unsigned char *dest = (unsigned char*)destinationBuffer;

    (void) ditherGenerator; /* unused parameter */

    while( count-- )
    {
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];

        src += sourceStride * 3;
        dest += destinationStride * 3;
    }
}

/* -------------------------------------------------------------------------- */

static void Copy_32_To_32(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator )
{
    unsigned int *dest = (unsigned int *)destinationBuffer;
    unsigned int *src = (unsigned int *)sourceBuffer;

    (void) ditherGenerator; /* unused parameter */

    while( count-- )
    {
        *dest = *src;

        src += sourceStride;
        dest += destinationStride;
    }
}

MRUtilConverterTable mrConverters =
{
    Float32_To_Int32,              /* PaUtilConverter *Float32_To_Int32; */
    Float32_To_Int32_Dither,       /* PaUtilConverter *Float32_To_Int32_Dither; */
    Float32_To_Int32_Clip,         /* PaUtilConverter *Float32_To_Int32_Clip; */
    Float32_To_Int32_DitherClip,   /* PaUtilConverter *Float32_To_Int32_DitherClip; */

    Float32_To_Int24,              /* PaUtilConverter *Float32_To_Int24; */
    Float32_To_Int24_Dither,       /* PaUtilConverter *Float32_To_Int24_Dither; */
    Float32_To_Int24_Clip,         /* PaUtilConverter *Float32_To_Int24_Clip; */
    Float32_To_Int24_DitherClip,   /* PaUtilConverter *Float32_To_Int24_DitherClip; */

    Float32_To_Int16,              /* PaUtilConverter *Float32_To_Int16; */
    Float32_To_Int16_Dither,       /* PaUtilConverter *Float32_To_Int16_Dither; */
    Float32_To_Int16_Clip,         /* PaUtilConverter *Float32_To_Int16_Clip; */
    Float32_To_Int16_DitherClip,   /* PaUtilConverter *Float32_To_Int16_DitherClip; */

    Float32_To_Int8,               /* PaUtilConverter *Float32_To_Int8; */
    Float32_To_Int8_Dither,        /* PaUtilConverter *Float32_To_Int8_Dither; */
    Float32_To_Int8_Clip,          /* PaUtilConverter *Float32_To_Int8_Clip; */
    Float32_To_Int8_DitherClip,    /* PaUtilConverter *Float32_To_Int8_DitherClip; */

    Float32_To_UInt8,              /* PaUtilConverter *Float32_To_UInt8; */
    Float32_To_UInt8_Dither,       /* PaUtilConverter *Float32_To_UInt8_Dither; */
    Float32_To_UInt8_Clip,         /* PaUtilConverter *Float32_To_UInt8_Clip; */
    Float32_To_UInt8_DitherClip,   /* PaUtilConverter *Float32_To_UInt8_DitherClip; */

    Int32_To_Float32,              /* PaUtilConverter *Int32_To_Float32; */
    Int32_To_Int24,                /* PaUtilConverter *Int32_To_Int24; */
    Int32_To_Int24_Dither,         /* PaUtilConverter *Int32_To_Int24_Dither; */
    Int32_To_Int16,                /* PaUtilConverter *Int32_To_Int16; */
    Int32_To_Int16_Dither,         /* PaUtilConverter *Int32_To_Int16_Dither; */
    Int32_To_Int8,                 /* PaUtilConverter *Int32_To_Int8; */
    Int32_To_Int8_Dither,          /* PaUtilConverter *Int32_To_Int8_Dither; */
    Int32_To_UInt8,                /* PaUtilConverter *Int32_To_UInt8; */
    Int32_To_UInt8_Dither,         /* PaUtilConverter *Int32_To_UInt8_Dither; */

    Int24_To_Float32,              /* PaUtilConverter *Int24_To_Float32; */
    Int24_To_Int32,                /* PaUtilConverter *Int24_To_Int32; */
    Int24_To_Int16,                /* PaUtilConverter *Int24_To_Int16; */
    Int24_To_Int16_Dither,         /* PaUtilConverter *Int24_To_Int16_Dither; */
    Int24_To_Int8,                 /* PaUtilConverter *Int24_To_Int8; */
    Int24_To_Int8_Dither,          /* PaUtilConverter *Int24_To_Int8_Dither; */
    Int24_To_UInt8,                /* PaUtilConverter *Int24_To_UInt8; */
    Int24_To_UInt8_Dither,         /* PaUtilConverter *Int24_To_UInt8_Dither; */

    Int16_To_Float32,              /* PaUtilConverter *Int16_To_Float32; */
    Int16_To_Int32,                /* PaUtilConverter *Int16_To_Int32; */
    Int16_To_Int24,                /* PaUtilConverter *Int16_To_Int24; */
    Int16_To_Int8,                 /* PaUtilConverter *Int16_To_Int8; */
    Int16_To_Int8_Dither,          /* PaUtilConverter *Int16_To_Int8_Dither; */
    Int16_To_UInt8,                /* PaUtilConverter *Int16_To_UInt8; */
    Int16_To_UInt8_Dither,         /* PaUtilConverter *Int16_To_UInt8_Dither; */

    Int8_To_Float32,               /* PaUtilConverter *Int8_To_Float32; */
    Int8_To_Int32,                 /* PaUtilConverter *Int8_To_Int32; */
    Int8_To_Int24,                 /* PaUtilConverter *Int8_To_Int24 */
    Int8_To_Int16,                 /* PaUtilConverter *Int8_To_Int16; */
    Int8_To_UInt8,                 /* PaUtilConverter *Int8_To_UInt8; */

    UInt8_To_Float32,              /* PaUtilConverter *UInt8_To_Float32; */
    UInt8_To_Int32,                /* PaUtilConverter *UInt8_To_Int32; */
    UInt8_To_Int24,                /* PaUtilConverter *UInt8_To_Int24; */
    UInt8_To_Int16,                /* PaUtilConverter *UInt8_To_Int16; */
    UInt8_To_Int8,                 /* PaUtilConverter *UInt8_To_Int8; */

    Copy_8_To_8,                   /* PaUtilConverter *Copy_8_To_8; */
    Copy_16_To_16,                 /* PaUtilConverter *Copy_16_To_16; */
    Copy_24_To_24,                 /* PaUtilConverter *Copy_24_To_24; */
    Copy_32_To_32                  /* PaUtilConverter *Copy_32_To_32; */
};
