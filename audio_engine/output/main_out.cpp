#include "output_wmme.h"

main_output *output_create(MRHostApiTypeId outputype)
{
    if (outputype == mrMME)
        return new output_wmme();
    return NULL;
}

MRSampleFormat MRUtil_SelectClosestAvailableFormat(MRSampleFormat availableFormats, MRSampleFormat format)
{
    MRSampleFormat result;

    format &= ~mrNonInterleaved;
    availableFormats &= ~mrNonInterleaved;

    if((format & availableFormats) == 0)
    {
        /* NOTE: this code depends on the sample format constants being in
            descending order of quality - ie best quality is 0
            FIXME: should write an assert which checks that all of the
            known constants conform to that requirement.
        */

        if(format != 0x01)
        {
            /* scan for better formats */
            result = format;
            do
            {
                result >>= 1;
            }
            while((result & availableFormats) == 0 && result != 0 );
        }
        else
        {
            result = 0;
        }

        if( result == 0 )
        {
            /* scan for worse formats */
            result = format;
            do
            {
                result <<= 1;
            }
            while((result & availableFormats) == 0 && result != mrCustomFormat);

            if( (result & availableFormats) == 0 )
                result = outer_SampleFormatNotSupported;
        }

    }
    else
    {
        result = format;
    }

    return result;
}

unsigned short MR_GetSampleSize(MRSampleFormat format)
{
    unsigned short result;
    switch(format & ~mrNonInterleaved)
    {

    case mrUInt8:
    case mrInt8:
        result = 1;
        break;

    case mrInt16:
        result = 2;
        break;

    case mrInt24:
        result = 3;
        break;

    case mrFloat32:
    case mrInt32:
        result = 4;
        break;

    default:
        result = outer_SampleFormatNotSupported;
        break;
    }

    return result;
}

void MRUtil_InitializeStreamRepresentation(MRUtilStreamRepresentation *streamRepresentation,
        MRStreamCallback *streamCallback,
        void *userData)
{
    streamRepresentation->magic = 0x18273645;
    streamRepresentation->nextOpenStream = 0;
    //streamRepresentation->streamInterface = streamInterface;
    streamRepresentation->streamCallback = streamCallback;
    streamRepresentation->streamFinishedCallback = 0;

    streamRepresentation->userData = userData;

    //streamRepresentation->streamInfo.inputLatency = 0.;
    //streamRepresentation->streamInfo.outputLatency = 0.;
    //streamRepresentation->streamInfo.sampleRate = 0.;
}
