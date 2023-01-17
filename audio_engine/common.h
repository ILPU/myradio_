#ifndef ACOMMON_H
#define ACOMMON_H

#include "../myhdr.h"
#include "types.h"
#include "../all_type.h"

#define GLUE(a, b) a ## b

#include "stream.h"

struct station_info {
    std::string name;
    std::string song;
};
struct station_info_extra {
    int bitrate;
    std::string url;
    std::string genre;
    std::string description;
    std::string author;
    std::string copyright;
};
struct station_meta {
    station_info info;
    station_info_extra extra;
};

/*
protocols, decoders and all sorts of outputs work with streams, a structure
with constant insertion and removal of items in the front and back. A chain is
the base class to pretty much everything of interest. A chain is used to
connect several components. For example, if you connect a protocol to a
decoder and then connect the decoder to wave_out whenever the network
pushes data to a protocol sound will be played.
*/

class chain
{
public:
    chain()
    {
        stream_out = NULL;
        stream_in = NULL;
        common_data = NULL;
    }
    void set_input(stream *s)
    {
        stream_in = s;
    }
    void set_output(stream *s)
    {
        stream_out = s;
    }
    void set_common(void *data)
    {
        common_data = data;
    }
//protected:
    stream *stream_out;
    stream *stream_in;
    void *common_data;
};

struct decoder_info
{
    format codec_type;

    /* bytes to decode per 'tick' */
    unsigned int frame_size;

    unsigned char channels;
    unsigned int sample_rate;
    unsigned int bit_rate;

    unsigned int block_align;
    unsigned char bits_per_sample;

    /* cook, wmav1, wmav2 needs this. */
    byte extra_data[64];
    unsigned char extra_size;

    /* specific options */
    unsigned int flags;

    unsigned int buffer_size;
};

void mem_guard(byte *ptr);
void mem_assert(byte *ptr);

void *MROutput_AllocateMemory(long size);
void MROutput_FreeMemory(void *block);


/*#define INT3
#ifdef __GNUC__
#  undef INT3
#  define INT3 __asm__("int3")
#endif
#ifdef _MSC_VER
#  undef INT3
#  define INT3 __asm { int 3 }
#endif
#ifdef _DEBUG
#define BREAKPOINT INT3
#else
#define BREAKPOINT
#endif*/

#endif
