#ifndef MYMPGLIB_H_INCLUDED
#define MYMPGLIB_H_INCLUDED

#if ( defined(_MSC_VER) || defined(__BORLANDC__) )
typedef int BOOL; /* windef.h contains the same definition */
#else
#define BOOL int
#endif

//#define	MPG123_USE_ONLY_FLOAT64
//#define	MPG123_USE_GAIN

#define	MP3_OK			0
#define	MP3_ERR			-1
#define	MP3_WAR			-2
#define	MP3_NEED_MORE	1

struct framebuf
{
    unsigned char*  pnt;
    int size;
    int pos;
    framebuf*   next;
    framebuf*   prev;
};

class mpg123
{
public:
    enum
    {
        DATA_FORMAT_LINEAR_PCM,
        DATA_FORMAT_IEEE_FLOAT,
    };
    int		currentBitrate;

    mpg123();
    ~mpg123(void);

    int __fastcall	get_frequency(void);
    int __fastcall	get_channel(void);
    void __fastcall	flush(void);
    int __fastcall	decode(
        const unsigned char* in,
        const int isize,
        unsigned char* out,
        const int osize,
        int* done);

private:

    bool	First;
    int		Max_osize;
    framebuf*	head;
    framebuf*	tail;
    int		bsize;
    int		FrameSize;
    frame	First_fr;
    frame	fr;
    RingBuff_8bit*	SideInfoBuff;
    RingBuff_16bit*	ReservoirBuff;
    int		hybrid_blc[2];
    real	hybrid_block[2][2][SBLIMIT * SSLIMIT];
    int		synth_bo;
    real	synth_buffs[2][2][0x110];

    DO_LAYER123_FUNC	do_layer123_func;

    void __fastcall	init(
        RingBuff_8bit* _SideInfoBuff,
        RingBuff_16bit* _ReservoirBuff,
        const int _currentBitrate);
    void __fastcall	release(void);
    inline void __fastcall	AddRingBuff(RingBuff_8bit* Buff, const int Size);
    inline void __fastcall	AddRingBuff(RingBuff_16bit* Buff, const int Size);
    inline framebuf* __fastcall	addbuf(const unsigned char* buf, const int size);
    inline void __fastcall	remove_buf(void);
    inline unsigned int __fastcall 	read_head(void);
    inline int __fastcall	read_buf_byte(void);
    inline void __fastcall	skip_buf_2byte(void);

    void __fastcall	I_step_one(unsigned int balloc[], unsigned int scale_index[2][SBLIMIT]);
    void __fastcall	I_step_two(
        real fraction[2][SBLIMIT],
        unsigned int balloc[2 * SBLIMIT],
        unsigned int scale_index[2][SBLIMIT]);
    int __fastcall	do_layer1(unsigned char* pcm_sample, int* pcm_point);

    void __fastcall	II_step_one(unsigned int* bit_alloc, int* scale);
    void __fastcall	II_step_two(
        unsigned int* bit_alloc,
        real fraction[2][4][SBLIMIT],
        int* scale,
        int x1);
    void __fastcall	II_select_table(void);
    int __fastcall	do_layer2(unsigned char* pcm_sample, int* pcm_point);

    int __fastcall	III_get_side_info(III_sideinfo* si, int ms_stereo);
    int __fastcall	III_get_scale_factors_1(int* scf, gr_info_s* gr_info);
    int __fastcall	III_get_scale_factors_2(int* scf, gr_info_s* gr_info, int i_stereo);
    int __fastcall	III_dequantize_sample(
        real xr[SBLIMIT][SSLIMIT],
        int* scf,
        gr_info_s* gr_info,
        int part2bits);
    void __fastcall	III_i_stereo(
        real xr_buf[2][SBLIMIT][SSLIMIT],
        int* scalefac,
        gr_info_s* gr_info,
        int ms_stereo);
    void __fastcall	III_antialias(real xr[SBLIMIT][SSLIMIT], gr_info_s* gr_info);
    void __fastcall	III_hybrid(
        real fsIn[SBLIMIT][SSLIMIT],
        real tsOut[SSLIMIT][SBLIMIT],
        int ch,
        gr_info_s* gr_info);
    int __fastcall	do_layer3(unsigned char* pcm_sample, int* pcm_point);


    void __fastcall	synth_1to1_float64(real*, int, unsigned char*, int*);
    void __fastcall	synth_1to1_mono_float64(real*, unsigned char*, int*);
};

#endif // MYMPGLIB_H_INCLUDED
