//#include "../../myhdr.h"

#include "mympglib/MinMax.h"

#include "decoder.h"

#include "mympglib/mympg123.h"
#include "mympglib/mympglib.h"

//#define	MAX_SEARCH_HEADER   65536

//#define INBUF_SIZE 4092
#define	FBSIZE                  8192
#define	OBSIZE					65536

#define	POST_DELAY				1152
#define	DECODE_DELAY_LAYER1		0
#define	DECODE_DELAY_LAYER2		(480 + 1)
#define	DECODE_DELAY_LAYER3		(528 + 1)

struct mp3info
{
    bool	stream;
    bool	vbr_tag;

    bool	mpeg25;
    int		lsf;
    int		lay;
    int		freq;
    int		mode;
    int		nch;
    int		bitrate;
    int		frame_samples;
    int		frames;
    int		samples;
    int		length;
    int		out_bps;
    int		out_bps_b;
    int		out_bps_b_nch;
    int		vis_bps_b_nch;
    int		bps;
    int		bps_b;
    int		bps_b_nch;
    int		decode_delay;
    int		skip_start;
    double	skip_start_ms;
    bool	replaygain_valid_track_gain;
    bool	replaygain_valid_track_peak;
    bool	replaygain_valid_album_gain;
    bool	replaygain_valid_album_peak;
    double	replaygain_track_gain;
    double	replaygain_track_peak;
    double	replaygain_album_gain;
    double	replaygain_album_peak;
};

struct mp3_decoder_data
{
    mpg123*	mp;
    //mp3data_struct header;
    //byte inbuf[INBUF_SIZE + 100];
    char sample_buffer[576 * (64 >> 3) * 2 * 2];

    unsigned char m_obuff[OBSIZE];
    int m_bend;
    int m_ssize;
    int m_decoder_stat;
    int m_skip_start_remain;
    int m_decode_pos_samples;
    double m_decode_pos_ms;    // current decoding position, in milliseconds

    mp3info info;

    bool first_read;
};

void Out_Float64toFloat32(char* OutBuff, unsigned char* InBuff, const int Size)
{
    float*	OutBuff_ = reinterpret_cast<float*>(OutBuff);

    for(int Idx = 0; Idx < Size; Idx += 8)
    {
        *OutBuff_++ = static_cast<float>(*reinterpret_cast<double*>(InBuff + Idx));
    }
}

inline unsigned int BSwap32(unsigned int Num)
{
    union
    {
        unsigned char	Work[4];
        unsigned int	Work32;
    };

    *(Work + 0) = *(reinterpret_cast<unsigned char*>(&Num) + 3);
    *(Work + 1) = *(reinterpret_cast<unsigned char*>(&Num) + 2);
    *(Work + 2) = *(reinterpret_cast<unsigned char*>(&Num) + 1);
    *(Work + 3) = *(reinterpret_cast<unsigned char*>(&Num) + 0);

    return Work32;
}

inline bool head_check(unsigned int head)
{
    if((head & 0xffe00000) != 0xffe00000) return false;
    if(((head >> 17) & 3) == 0) return false;
    if((4 - ((head >> 17) & 3)) != 3) return false;
    if(((head >> 10) & 0x3) == 0x3) return false;

    const int bitrate = (head >> 12) & 0xf;

    if((bitrate == 0x0) || (bitrate == 0xf)) return false;

    return true;
}

inline bool head_check2(unsigned int head, mp3info* info)
{
    if((head & 0xffe00000) != 0xffe00000) return false;

    const int lay = (head >> 17) & 3;

    if(lay == 0) return false;
    if((4 - ((head >> 17) & 3)) != 3) return false;

    const int freq = (head >> 10) & 0x3;

    if(freq == 0x3) return false;

    const int bitrate = (head >> 12) & 0xf;

    if((bitrate == 0x0) || (bitrate == 0xf)) return false;

    bool mpeg25;
    int lsf;

    if(head & (1 << 20))
    {
        mpeg25 = false;
        lsf = (head & (1 << 19)) ? 0 : 1;
    }
    else
    {
        mpeg25 = true;
        lsf = 1;
    }

    if(mpeg25 != info->mpeg25) return false;
    if(lsf != info->lsf) return false;
    if((4 - lay) != info->lay) return false;
    if(freqs[freq + (mpeg25 ? 6 : lsf * 3)] != info->freq) return false;
    if(((((head >> 6) & 0x3) == MPG_MD_MONO) ? 1 : 2) != info->nch) return false;

    return true;
}

/* public */
decoder_mp3 :: decoder_mp3()
{
    impl = new mp3_decoder_data;

    mp3_decoder_data *data = (mp3_decoder_data *)impl;

    data->mp = new mpg123();
    data->m_bend = 0;
    data->m_decode_pos_ms = 0.;

    data->first_read = 0;
}

decoder_mp3 :: ~decoder_mp3()
{
    mp3_decoder_data *data = (mp3_decoder_data *)impl;
    if(data->mp) delete data->mp;
    delete data;
}

bool SearchHeader_stream(stream* pstream_in, mpg123* mp, mp3_decoder_data *pdata)
{
    bool RetCode = false;
    stream tmp_stream(*pstream_in);
    while(!tmp_stream.empty())
    {
        unsigned int read_head;
        tmp_stream.readprt(&read_head, 4);
        if(head_check2(BSwap32(read_head), &pdata->info))
        {
            pdata->m_skip_start_remain = pdata->info.decode_delay;
            mp->flush();
            pdata->m_bend = 0;
			pdata->m_decoder_stat = MP3_NEED_MORE;
			if(tmp_stream.size() > (FBSIZE*2) )
            {
               pstream_in->erase(pstream_in->size() - tmp_stream.size());
               RetCode = true;
            }
			break;
        }
        tmp_stream.erase(1);
    }
    return RetCode;
}

int get_576_samples(stream *pstream, mp3_decoder_data *pdata, char* out, bool* supZero)
{
    while(pdata->m_bend < pdata->m_ssize)
    {
        int Size;

        if(pdata->m_decoder_stat != MP3_ERR)
        {
            DWORD Len;
            unsigned char*  InBuff;
            unsigned char   Buff[FBSIZE];

            if(pdata->m_decoder_stat == MP3_NEED_MORE)
            {
                Len = pstream->read(Buff, FBSIZE);
                pstream->erase(Len);

                if(Len)
                {
                    InBuff = Buff;
                }
                else
                {
                    break;
                }
            }
            else
            {
                Len = 0;
                InBuff = NULL;
            }

            pdata->m_decoder_stat = pdata->mp->decode(InBuff, Len, pdata->m_obuff + pdata->m_bend, OBSIZE - pdata->m_bend, &Size);
        }

        if(pdata->m_decoder_stat == MP3_OK)
        {
            pdata->m_bend += Size;

            if(pdata->m_skip_start_remain)
            {
                const int bend_samples = pdata->m_bend / pdata->info.bps_b_nch;

                if(pdata->m_skip_start_remain < bend_samples)
                {
                    const int	m_skip_start_remain_size = pdata->m_skip_start_remain * pdata->info.bps_b_nch;

                    pdata->m_bend -= m_skip_start_remain_size;
                    memmove(pdata->m_obuff, pdata->m_obuff + m_skip_start_remain_size, pdata->m_bend);

                    pdata->m_skip_start_remain = 0;
                }
                else
                {
                    pdata->m_bend = 0;
                    pdata->m_skip_start_remain -= bend_samples;
                }
            }


            if(*supZero)
            {
                const int nch = pdata->info.nch;
                const int bps_b = pdata->info.bps_b;
                int p;

                for(p = 0; p < pdata->m_bend; p += pdata->info.bps_b_nch)
                {
                    for(int c = 0; c < nch; c++)
                    {
                        for(int q = 0; q < bps_b; q++)
                        {
                            if(pdata->m_obuff[p + c * bps_b + q] != 0) goto BreakSupLoop;
                        }
                    }
                }

BreakSupLoop:
                if(p < pdata->m_bend)
                {
                    pdata->m_bend -= p;
                    memmove(pdata->m_obuff, pdata->m_obuff + p, pdata->m_bend);

                    *supZero = false;
                }
                else
                {
                    pdata->m_bend = 0;
                }

                const int zero_samples = p / pdata->info.bps_b_nch;

                pdata->m_decode_pos_ms += (zero_samples * 1000. / pdata->info.freq);
                if(pdata->info.vbr_tag) pdata->m_decode_pos_samples += zero_samples;
            }
        }
        else if(pdata->m_decoder_stat == MP3_WAR)
        {
            pdata->m_decode_pos_ms += (pdata->info.frame_samples * 1000. / pdata->info.freq);
            if(pdata->info.vbr_tag) pdata->m_decode_pos_samples += pdata->info.frame_samples;
        }
        else if(pdata->m_decoder_stat == MP3_ERR)
        {
            return -1;
        }

    } // while(m_bend < m_ssize)

    int CopySize = Min(pdata->m_bend, pdata->m_ssize);

    if(CopySize)
    {
        if(pdata->info.vbr_tag)
        {
            int CopySamples = CopySize / pdata->info.bps_b_nch;

            if((pdata->m_decode_pos_samples + CopySamples) >= pdata->info.samples)
            {
                CopySamples = (pdata->info.samples > pdata->m_decode_pos_samples) ?
                              pdata->info.samples - pdata->m_decode_pos_samples :
                              0;
                CopySize = CopySamples * pdata->info.bps_b_nch;
            }

            pdata->m_decode_pos_samples += CopySamples;
        }

        if(CopySize)
        {
            //m_Bitrate->Set(mp->currentBitrate);

            //if(prm->m_bPostProc) PostProc(m_obuff, CopySize);

            Out_Float64toFloat32(out, pdata->m_obuff, CopySize); // mp3 decoder always return 32 float sample!

            pdata->m_bend -= CopySize;
            if(pdata->m_bend) memmove(pdata->m_obuff,  pdata->m_obuff + CopySize,  pdata->m_bend);
        }
    }
    return CopySize / pdata->info.bps_b_nch;
}

int decoder_mp3 :: decode()
{
    mp3_decoder_data *data = (mp3_decoder_data *)impl;

    if(stream_in->size() < FBSIZE * 2)
        return DECODER_NEEDMOREDATA;

    if(data->first_read == 0)
    {
        stream tmp_stream(*stream_in);
        bool FindHeader = false;

        while (!tmp_stream.empty())
        {
            unsigned int read_head;
            tmp_stream.readprt(&read_head, 4);
            unsigned int head;
            memset(&data->info, 0, sizeof(mp3info));
            if(head_check(head = BSwap32(read_head)))
            {
                if(head & (1 << 20))
                {
                    data->info.mpeg25 = false;
                    data->info.lsf = (head & (1 << 19)) ? 0 : 1;
                }
                else
                {
                    data->info.mpeg25 = true;
                    data->info.lsf = 1;
                }

                const int bitrate = (head >> 12) & 0xf;
                const int padding = (head >> 9) & 0x1;

                data->info.lay = 4 - ((head >> 17) & 3);
                data->info.freq = freqs[((head >> 10) & 0x3) + (data->info.mpeg25 ? 6 : data->info.lsf * 3)];
                data->info.mode = (head >> 6) & 0x3;
                data->info.nch = (data->info.mode == MPG_MD_MONO) ? 1 : 2;
                data->info.bitrate = tabsel_123[data->info.lsf][data->info.lay - 1][bitrate] * 1000;

                int framesizeInBytes = 0;
                switch(data->info.lay)
                {
                case 3:
                    framesizeInBytes = data->info.bitrate * 144 / (data->info.freq << data->info.lsf) + padding;
                    break;
                case 2:
                    framesizeInBytes = data->info.bitrate * 144 / data->info.freq + padding;
                    break;
                case 1:
                    framesizeInBytes = (data->info.bitrate * 12 / data->info.freq + padding) * 4;
                    break;
                }

                tmp_stream.erase(framesizeInBytes);
                if(tmp_stream.readprt(&read_head, 4) == 4)
                    if(head_check2(BSwap32(read_head), &data->info))
                    {
                        FindHeader = true;
                        break;
                    }
            }
            if(!FindHeader)
                tmp_stream.erase(1);
        }

        if(FindHeader)
        {
            stream_in->erase(stream_in->size() - tmp_stream.size());
            switch(data->info.lay)
            {
            case 3:
                data->info.frame_samples = 576 * (data->info.lsf ? 1 : 2 );
                data->info.decode_delay = DECODE_DELAY_LAYER3;
                break;
            case 2:
                data->info.frame_samples = 1152;
                data->info.decode_delay = DECODE_DELAY_LAYER2;
                break;
            case 1:
                data->info.frame_samples = 384;
                data->info.decode_delay = DECODE_DELAY_LAYER1;
                break;
            }
        }

        if(!FindHeader)
            return DECODER_INTERNALERROR;
        data->first_read = 1;
    }

    data->info.vbr_tag = false;
    data->info.length = 0;

    int OutputBps = -32;
    data->info.out_bps = (OutputBps >= 0) ? OutputBps : -OutputBps;
    data->info.out_bps_b = data->info.out_bps >> 3;
    data->info.out_bps_b_nch = data->info.out_bps_b * data->info.nch;

    data->info.bps = 64;
    data->info.bps_b = 64 >> 3;
    data->info.bps_b_nch = data->info.bps_b * data->info.nch;
    data->info.length = 0;
    data->info.skip_start = data->info.decode_delay;

    data->m_ssize = 576 * data->info.bps_b_nch;
    data->m_decode_pos_samples = 0;
    data->m_skip_start_remain = data->info.skip_start;

    data->m_decoder_stat = MP3_NEED_MORE;

    bool supZero = false;
    int ret_error = 0;

    while(stream_in->size() >= FBSIZE && continue_decoding())
    {
        int Len = get_576_samples(stream_in, data, data->sample_buffer, &supZero);

        if(Len == -1)
        {
            if(SearchHeader_stream(stream_in, data->mp, data))
            {
                supZero = false;
            }
            else
            {
                ret_error = DECODER_INTERNALERROR;
                break;
            }
        }
        else if(Len == 0)
        {
            ret_error = DECODER_NEEDMOREDATA;
            break;
        }
        else
        {
            if(Len)
            {
                const int SampleBuffLen = Len * data->info.out_bps_b_nch;
                stream_out->push((byte *)(data->sample_buffer), SampleBuffLen);
            }
        }

    }  // while(stream_in->size() >= FBSIZE && continue_decoding())

    return ret_error;
}
