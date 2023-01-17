#include "main_out_dither.h"

#define MR_DITHER_BITS_   (15)

void MRUtil_InitializeTriangularDitherState(MRUtilTriangularDitherGenerator *ditherState)
{
    ditherState->previous = 0;
    ditherState->randSeed1 = 22222;
    ditherState->randSeed2 = 5555555;
}

signed int MRUtil_Generate16BitTriangularDither(MRUtilTriangularDitherGenerator *ditherState)
{
    signed int current, highPass;

    ditherState->randSeed1 = (ditherState->randSeed1 * 196314165) + 907633515;
    ditherState->randSeed2 = (ditherState->randSeed2 * 196314165) + 907633515;

#define DITHER_SHIFT_  ((sizeof(signed int)*8 - MR_DITHER_BITS_) + 1)

    current = (((signed int)ditherState->randSeed1)>>DITHER_SHIFT_) +
              (((signed int)ditherState->randSeed2)>>DITHER_SHIFT_);

    highPass = current - ditherState->previous;
    ditherState->previous = current;
    return highPass;
}

#define MR_FLOAT_DITHER_SCALE_  (1.0f / ((1<<MR_DITHER_BITS_)-1))
static const float const_float_dither_scale_ = MR_FLOAT_DITHER_SCALE_;

float MRUtil_GenerateFloatTriangularDither(MRUtilTriangularDitherGenerator *ditherState)
{
    signed int current, highPass;

    ditherState->randSeed1 = (ditherState->randSeed1 * 196314165) + 907633515;
    ditherState->randSeed2 = (ditherState->randSeed2 * 196314165) + 907633515;

    current = (((signed int)ditherState->randSeed1)>>DITHER_SHIFT_) +
              (((signed int)ditherState->randSeed2)>>DITHER_SHIFT_);

    highPass = current - ditherState->previous;
    ditherState->previous = current;
    return ((float)highPass) * const_float_dither_scale_;
}
