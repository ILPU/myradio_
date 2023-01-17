class decoder : public chain
{
public:
    decoder()
    {
        dec_info_saved.flags = 0;
        codec_info = &dec_info_saved;
        /*if (!loaded)
        {
            printf("loaded");
            loaded = true;
        }*/
        outbuf_size = 0;
        output_bps = -1;
    }
    virtual ~decoder()
    {
    }

    /*
     * decode as much as possible from stream_in and write to stream_out
     */
    virtual int decode() = 0;

    /*
     * samplerate needed to make sense of wave samples
     */
    int get_samplerate()
    {
        return codec_info->sample_rate;
    }

    /*
     * num channels needed to make sense of wave samples
     */
    int get_channels()
    {
        return codec_info->channels;
    }

    /*
     * codec specific configuration options. for example we sometimes want to
     * delete unknown/broken frames before doing the decoding. however sometimes
     * we don't want this to happen. for sample usage see AAC decoder.
     */
    void set_flags(unsigned int  nflags)
    {
        dec_info_saved.flags = nflags;
    }

    /*
     * set up decoder info with knowledge from demuxer.
     */
    void set_parameters(decoder_info *dec_info)
    {

        memcpy(&dec_info_saved,
               dec_info,
               sizeof(decoder_info));

        if (dec_info->extra_size < 64)
        {
            memcpy(dec_info_saved.extra_data,
                   dec_info->extra_data,
                   dec_info->extra_size);
        }
        dec_info_saved.flags = 0;
        codec_info = &dec_info_saved;
        //codec_info = dec_info;
    }

    void set_output_buffer_size(size_t len)
    {
        outbuf_size = len;
    }

    void set_output_bps(int newbps)
    {
        output_bps = newbps;
    }

    //inline double get_output_buffer_length()
    //{
    //    return outbuf_len;
    //}

protected:
    decoder_info *codec_info;

    inline bool continue_decoding()
    {
        //return true;
        return (stream_out->size() < 1048576) && (outbuf_size < 1048576);
        //return (outbuf_size < 1048576);
    }

private:
    int output_bps;
    size_t outbuf_size;
    decoder_info  dec_info_saved;
    //static bool loaded;
};

//bool decoder :: loaded = false;

/* boilerplate
#define IMPLEMENT_DECODER(name)                                              \
class GLUE(decoder_, name) : public decoder {                                \
public:                                                                      \
    GLUE(decoder_, name)();                                                  \
    ~GLUE(decoder_, name)();                                                 \
    int decode();                                                            \
private:                                                                     \
    void *impl;                                                              \
};
*/
