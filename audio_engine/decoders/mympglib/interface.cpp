#include <windows.h>
#include <float.h>

#include "mympg123.h"
#include "mympglib.h"
#include "MinMax.h"

mpg123::mpg123()
{
    static bool	InitTable = false;

    if(InitTable == false)
    {
        make_decode_tables(1);
        init_layer2();
        init_layer3(SBLIMIT);
        InitTable = true;
    }

    init(
        NULL,
        NULL,
        0);
}

mpg123::~mpg123(void)
{
    release();

    delete SideInfoBuff;
    delete ReservoirBuff;
}

void __fastcall mpg123::init(RingBuff_8bit* _SideInfoBuff, RingBuff_16bit* _ReservoirBuff, const int _currentBitrate)
{
    memset(this, 0, sizeof *this);

    if(_SideInfoBuff)
    {
        SideInfoBuff = _SideInfoBuff;
    }
    else
    {
        SideInfoBuff = new RingBuff_8bit(/*32*/);
    }

    if(_ReservoirBuff)
    {
        ReservoirBuff = _ReservoirBuff;
        ReservoirBuff->Init();
    }
    else
    {
        ReservoirBuff = new RingBuff_16bit(/*MAXFRAMESIZE * 10*/);
    }

    currentBitrate = _currentBitrate;

    synth_bo = 1;

    First = true;
#ifdef MPG123_USE_ONLY_FLOAT64
    Max_osize = 1152 * (64 >> 3) * 2;
#else
    Max_osize = 1152 * (_OutputBps >> 3) * 2;
#endif
}

void __fastcall mpg123::release(void)
{
    framebuf*	b = tail;
    framebuf*	bn;

    while(b)
    {
        delete[] b->pnt;
        bn = b->next;
        delete b;
        b = bn;
    }
}

int __fastcall mpg123::get_frequency(void)
{
    return freqs[fr.frequency];
}

int __fastcall mpg123::get_channel(void)
{
    return fr.stereo;
}

void __fastcall mpg123::flush(void)
{
    release();
    init(SideInfoBuff, ReservoirBuff, currentBitrate);
}

int __fastcall mpg123::decode(const unsigned char* in, const int isize, unsigned char* out, const int osize, int* done)
{
//	errorstring = "no error or unknown error";

    if(osize < Max_osize)
    {
//		errorstring = "output buffer is too small";
        return MP3_ERR;
    }

    if(in && (addbuf(in, isize) == NULL)) return MP3_ERR;

    /* First decode header */
    if(FrameSize == 0)
    {
        if(bsize < 4) return MP3_NEED_MORE;

        unsigned int header = read_head();

        if(decode_header(&fr, header) == false) return MP3_ERR;

        if(First)
        {
            switch(fr.lay)
            {
            case 3:
                do_layer123_func = &mpg123::do_layer3;
                break;
            case 2:
                do_layer123_func = &mpg123::do_layer2;
                break;
            case 1:
                do_layer123_func = &mpg123::do_layer1;
                break;
            }

            First_fr = fr;
            First = false;
        }
        else
        {
            if(		(fr.mpeg25 != First_fr.mpeg25) ||
                    (fr.lsf != First_fr.lsf) ||
                    (fr.lay != First_fr.lay) ||
                    (fr.frequency != First_fr.frequency) ||
                    (fr.stereo != First_fr.stereo))
            {
                return MP3_ERR;
            }
        }

        FrameSize = fr.framesize;
        currentBitrate = tabsel_123[fr.lsf][fr.lay - 1][fr.bitrate];
    }

    if(bsize < FrameSize) return MP3_NEED_MORE;

    if(fr.error_protection) skip_buf_2byte();

    AddRingBuff(SideInfoBuff, fr.sideInfoSize);
    AddRingBuff(ReservoirBuff, FrameSize - fr.error_protection * 2 - fr.sideInfoSize);

    FrameSize = 0;

    *done = 0;

    return (this->*do_layer123_func)(out, done);
}

inline void __fastcall mpg123::AddRingBuff(RingBuff_8bit* Buff, const int Size)
{
    Buff->SetPointer();

    int		AddSize = 0;

    while(AddSize < Size)
    {
        const int	nSize = Min(Size - AddSize, tail->size - tail->pos);

        Buff->Add(tail->pnt + tail->pos, nSize);

        AddSize += nSize;
        tail->pos += nSize;
        bsize -= nSize;

        if(tail->pos == tail->size) remove_buf();
    }
}

inline void __fastcall mpg123::AddRingBuff(RingBuff_16bit* Buff, const int Size)
{
    Buff->SetPointer();

    int		AddSize = 0;

    while(AddSize < Size)
    {
        const int	nSize = Min(Size - AddSize, tail->size - tail->pos);

        Buff->Add(tail->pnt + tail->pos, nSize);

        AddSize += nSize;
        tail->pos += nSize;
        bsize -= nSize;

        if(tail->pos == tail->size) remove_buf();
    }
}

inline framebuf* __fastcall mpg123::addbuf(const unsigned char* buf, const int size)
{
    framebuf*	nbuf = new framebuf;

    if(nbuf == NULL)
    {
//		errorstring = "Out of memory";
//		fprintf(stderr,"Out of memory!\n");
        return NULL;
    }

    nbuf->pnt = new unsigned char[size];

    if(nbuf->pnt == NULL)
    {
        delete nbuf;
        return NULL;
    }

    nbuf->size = size;
    memcpy(nbuf->pnt, buf, size);
    nbuf->next = NULL;
    nbuf->prev = head;
    nbuf->pos = 0;

    if(tail)
    {
        head->next = nbuf;
    }
    else
    {
        tail = nbuf;
    }

    head = nbuf;
    bsize += size;

    return nbuf;
}

inline void __fastcall mpg123::remove_buf(void)
{
    framebuf*	buf = tail;

    tail = buf->next;

    if(tail)
    {
        tail->prev = NULL;
    }
    else
    {
        head = NULL;
        tail = NULL;
    }

    delete[] buf->pnt;
    delete buf;
}

inline unsigned int __fastcall mpg123::read_head(void)
{
    unsigned int	header;

    header = read_buf_byte();
    header <<= 8;
    header |= read_buf_byte();
    header <<= 8;
    header |= read_buf_byte();
    header <<= 8;
    header |= read_buf_byte();

    return header;
}

inline int __fastcall mpg123::read_buf_byte(void)
{
    int		pos = tail->pos;

    while(pos >= tail->size)
    {
        remove_buf();

        if(tail == NULL)
        {
//			errorstring = "read_buf_byte error";
//			fprintf(stderr,"Fatal error!\n");
            return 0;
        }

        pos = tail->pos;
    }

    bsize--;
    tail->pos++;

    return tail->pnt[pos];
}

inline void __fastcall mpg123::skip_buf_2byte(void)
{
    int		SkipSize = 0;

    while(SkipSize < 2)
    {
        const int	nSize = Min(2 - SkipSize, tail->size - tail->pos);

        SkipSize += nSize;
        tail->pos += nSize;
        bsize -= nSize;

        if(tail->pos == tail->size) remove_buf();
    }
}
