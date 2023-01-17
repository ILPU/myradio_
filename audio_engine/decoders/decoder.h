#ifndef DECODER_H
#define DECODER_H

#include "../common.h"
#include "decoder_internal.h"

/*
 * init decoding libraries.
 */
void decoder_init();

/*
 * create a decoder.
 */
decoder *decoder_create(format type);

/*
 * errors.
 */
#define DECODER_ERRORBASE      -0xd3c0d3
#define DECODER_NEEDMOREDATA   (DECODER_ERRORBASE + 1)
#define DECODER_INTERNALERROR  (DECODER_ERRORBASE + 2)
#define DECODER_HEADERMISMATCH (DECODER_ERRORBASE + 3)

/*
 * decoders.
 */

class decoder_mp3 : public decoder
{
public:
    decoder_mp3();
    ~decoder_mp3();
    int decode();
private:
    void *impl;
};


//IMPLEMENT_DECODER(libav);
//IMPLEMENT_DECODER(wma);
//IMPLEMENT_DECODER(ra);
//IMPLEMENT_DECODER(rabin);
//IMPLEMENT_DECODER(mp3);
//IMPLEMENT_DECODER(mp3adu);
//IMPLEMENT_DECODER(vorbis);
//IMPLEMENT_DECODER(aac);
#endif
