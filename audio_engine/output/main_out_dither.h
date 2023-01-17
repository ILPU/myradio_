#ifndef MAIN_OUT_DITHER_H_INCLUDED
#define MAIN_OUT_DITHER_H_INCLUDED

/**
typedef signed int PaInt32;
typedef unsigned int PaUint32;
*/

typedef struct MRUtilTriangularDitherGenerator
{
    unsigned int previous; // PaUint32
    unsigned int randSeed1;
    unsigned int randSeed2;
} MRUtilTriangularDitherGenerator;


void MRUtil_InitializeTriangularDitherState(MRUtilTriangularDitherGenerator *ditherState);
/**
return signed 32-bit integer with a range of +32767 to -32768
*/
signed int MRUtil_Generate16BitTriangularDither(MRUtilTriangularDitherGenerator *ditherState);
/**
return float with a range of -2.0 to +1.99999.
*/
float MRUtil_GenerateFloatTriangularDither(MRUtilTriangularDitherGenerator *ditherState);

#endif // MAIN_OUT_DITHER_H_INCLUDED
