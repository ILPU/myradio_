#include "decoder.h"

void decoder_init()
{
	/* libavcodec */
	//avcodec_init();
	//avcodec_register_all();
}

decoder *decoder_create(format type)
{
    if (type == FMP3)
        return new decoder_mp3();

    /*if (type == FMP3ADU)
        return new decoder_mp3adu();

    if (type == FVORBIS)
        return new decoder_vorbis();

    if (type == FAAC)
        return new decoder_aac();

    if (type == FRA || type == FRACOOK)
        return new decoder_ra();

    if (type == FRASIPR || type == FRADNET || type == FRAATRC)
        return new decoder_rabin();

    if (type == FWMA || type == FWMAV1 || type == FWMAV2)
        return new decoder_wma();*/

    return NULL;
}

void decoder_destroy(decoder *ptr)
{
    delete ptr;
}
