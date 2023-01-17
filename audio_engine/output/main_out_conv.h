#ifndef MAIN_OUT_CONV_H_INCLUDED
#define MAIN_OUT_CONV_H_INCLUDED

#include "main_out.h"

struct MRUtilTriangularDitherGenerator;

typedef void MRUtilConverter(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count, struct MRUtilTriangularDitherGenerator *ditherGenerator);

typedef void MRUtilZeroer(
    void *destinationBuffer, signed int destinationStride, unsigned int count);


MRUtilConverter* MRUtil_SelectConverter(MRSampleFormat sourceFormat,
                                        MRSampleFormat destinationFormat, unsigned long flags);

MRUtilZeroer* MRUtil_SelectZeroer(MRSampleFormat destinationFormat);

typedef struct
{
    MRUtilConverter *Float32_To_Int32;
    MRUtilConverter *Float32_To_Int32_Dither;
    MRUtilConverter *Float32_To_Int32_Clip;
    MRUtilConverter *Float32_To_Int32_DitherClip;

    MRUtilConverter *Float32_To_Int24;
    MRUtilConverter *Float32_To_Int24_Dither;
    MRUtilConverter *Float32_To_Int24_Clip;
    MRUtilConverter *Float32_To_Int24_DitherClip;

    MRUtilConverter *Float32_To_Int16;
    MRUtilConverter *Float32_To_Int16_Dither;
    MRUtilConverter *Float32_To_Int16_Clip;
    MRUtilConverter *Float32_To_Int16_DitherClip;

    MRUtilConverter *Float32_To_Int8;
    MRUtilConverter *Float32_To_Int8_Dither;
    MRUtilConverter *Float32_To_Int8_Clip;
    MRUtilConverter *Float32_To_Int8_DitherClip;

    MRUtilConverter *Float32_To_UInt8;
    MRUtilConverter *Float32_To_UInt8_Dither;
    MRUtilConverter *Float32_To_UInt8_Clip;
    MRUtilConverter *Float32_To_UInt8_DitherClip;

    MRUtilConverter *Int32_To_Float32;
    MRUtilConverter *Int32_To_Int24;
    MRUtilConverter *Int32_To_Int24_Dither;
    MRUtilConverter *Int32_To_Int16;
    MRUtilConverter *Int32_To_Int16_Dither;
    MRUtilConverter *Int32_To_Int8;
    MRUtilConverter *Int32_To_Int8_Dither;
    MRUtilConverter *Int32_To_UInt8;
    MRUtilConverter *Int32_To_UInt8_Dither;

    MRUtilConverter *Int24_To_Float32;
    MRUtilConverter *Int24_To_Int32;
    MRUtilConverter *Int24_To_Int16;
    MRUtilConverter *Int24_To_Int16_Dither;
    MRUtilConverter *Int24_To_Int8;
    MRUtilConverter *Int24_To_Int8_Dither;
    MRUtilConverter *Int24_To_UInt8;
    MRUtilConverter *Int24_To_UInt8_Dither;

    MRUtilConverter *Int16_To_Float32;
    MRUtilConverter *Int16_To_Int32;
    MRUtilConverter *Int16_To_Int24;
    MRUtilConverter *Int16_To_Int8;
    MRUtilConverter *Int16_To_Int8_Dither;
    MRUtilConverter *Int16_To_UInt8;
    MRUtilConverter *Int16_To_UInt8_Dither;

    MRUtilConverter *Int8_To_Float32;
    MRUtilConverter *Int8_To_Int32;
    MRUtilConverter *Int8_To_Int24;
    MRUtilConverter *Int8_To_Int16;
    MRUtilConverter *Int8_To_UInt8;

    MRUtilConverter *UInt8_To_Float32;
    MRUtilConverter *UInt8_To_Int32;
    MRUtilConverter *UInt8_To_Int24;
    MRUtilConverter *UInt8_To_Int16;
    MRUtilConverter *UInt8_To_Int8;

    MRUtilConverter *Copy_8_To_8;       /* copy without any conversion */
    MRUtilConverter *Copy_16_To_16;     /* copy without any conversion */
    MRUtilConverter *Copy_24_To_24;     /* copy without any conversion */
    MRUtilConverter *Copy_32_To_32;     /* copy without any conversion */
} MRUtilConverterTable;

extern MRUtilConverterTable mrConverters;

typedef struct
{
    MRUtilZeroer *ZeroU8; /* unsigned 8 bit, zero == 128 */
    MRUtilZeroer *Zero8;
    MRUtilZeroer *Zero16;
    MRUtilZeroer *Zero24;
    MRUtilZeroer *Zero32;
} MRUtilZeroerTable;

#endif // MAIN_OUT_CONV_H_INCLUDED
