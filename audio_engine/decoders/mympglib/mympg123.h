#ifndef MYMPG123_H_INCLUDED
#define MYMPG123_H_INCLUDED

#include    <stdio.h>
#include    <string.h>
#include    <signal.h>

#define _USE_MATH_DEFINES
#include    <math.h>

#define M_PI        3.14159265358979323846
#define M_SQRT2	    1.41421356237309504880
#define random rand
#define srandom srand

//#define NEW_DCT9

#define real double

#ifdef __GNUC__
#define INLINE inline
#else
#define INLINE
#endif

#define AUDIOBUFSIZE		16384

#ifndef FALSE
#define FALSE                   0
#endif
#ifndef FALSE
#define TRUE                    1
#endif

#define SBLIMIT                 32
#define SSLIMIT                 18

#define SCALE_BLOCK             12


#define MPG_MD_STEREO           0
#define MPG_MD_JOINT_STEREO     1
#define MPG_MD_DUAL_CHANNEL     2
#define MPG_MD_MONO             3

//#define MAXFRAMESIZE 1792
#define MAXFRAMESIZE 4096

/* Pre Shift fo 16 to 8 bit converter table */
#define AUSHIFT (3)

class   mpg123;

struct frame
{
    bool mpeg25;
    int lsf;
    int lay;
    int error_protection;
    int bitrate;
    int frequency;
    int padding;
    int extension;
    int mode;
    int mode_ext;
    int copyright;
    int original;
    int emphasis;
    int stereo;
    int jsbound;
    int framesize; /* computed framesize */

    int sideInfoSize; /* Layer3 sideInfo Header size */

    /* layer2 stuff */
    int II_sblimit;
    void *alloc;
};

struct parameter
{
    int quiet;	/* shut up! */
    int tryresync;  /* resync stream after error */
    int verbose;    /* verbose level */
    int checkrange;
};

extern const int tabsel_123[2][3][16];

//extern void make_decode_tables(long scaleval);
//extern int do_layer3(struct mpstr *mp,struct frame *fr,unsigned char *,int *);
//extern int do_layer2(struct frame *fr,unsigned char *,int *);
//extern int do_layer1(struct frame *fr,unsigned char *,int *);
extern bool __fastcall decode_header(frame* fr, unsigned long newhead);

struct gr_info_s
{
    int scfsi;
    unsigned part2_3_length;
    unsigned big_values;
    unsigned scalefac_compress;
    unsigned block_type;
    unsigned mixed_block_flag;
    unsigned table_select[3];
    unsigned subblock_gain[3];
    unsigned maxband[3];
    unsigned maxbandl;
    unsigned maxb;
    unsigned region1start;
    unsigned region2start;
    unsigned preflag;
    unsigned scalefac_scale;
    unsigned count1table_select;
    real *full_gain[3];
    real *pow2gain;
};

struct III_sideinfo
{
    unsigned main_data_begin;
    unsigned private_bits;
    struct
    {
        gr_info_s	gr[2];
    } ch[2];
};

class RingBuff_8bit
{
public:
    RingBuff_8bit(void);
    ~RingBuff_8bit(void);

    void __fastcall	Init(void);
    inline void __fastcall
    SetPointer(void)
    {
        ReadPnt = BuffEnd;
        BitIndex = 0;
    }
    inline bool __fastcall
    SetPointer(const unsigned int BackStep)
    {
        if((BuffWrite < (1 << 8)) && (BackStep > ReadPnt)) return false;

        ReadPnt -= BackStep;
        BitIndex = 0;

        return true;
    }
    void __fastcall	Add(unsigned char* AddBuff, const unsigned int Size);
    inline unsigned char __fastcall
    get1bit(void)
    {
        const unsigned char	Value = *(Buff + ReadPnt) << BitIndex;

        ReadPnt += ++BitIndex >> 3;
        BitIndex &= 7;

        return Value >> 7;
    }
    inline unsigned int __fastcall
    getbits_fast(const int NumberOfBits)
    {
        if(NumberOfBits <= 0) return 0;

        unsigned int	Value = (*(Buff + ReadPnt) << 8) |
                                *(Buff + static_cast<unsigned char>(ReadPnt + 1));

        Value = static_cast<unsigned short>(Value << BitIndex) >> (16 - NumberOfBits);

        ReadPnt += (BitIndex += NumberOfBits) >> 3;
        BitIndex &= 7;

        return Value;
    }
    inline unsigned int __fastcall
    getbits(const int NumberOfBits)
    {
        if(NumberOfBits <= 0) return 0;

        unsigned int	Value = (*(Buff + ReadPnt) << 16) |
                                (*(Buff + static_cast<unsigned char>(ReadPnt + 1)) << 8) |
                                *(Buff + static_cast<unsigned char>(ReadPnt + 2));

        Value = ((Value << BitIndex) & 0xffffff) >> (24 - NumberOfBits);

        ReadPnt += (BitIndex += NumberOfBits) >> 3;
        BitIndex &= 7;

        return Value;
    }

private:
    unsigned int	BuffWrite;
    unsigned char	BuffEnd;
    unsigned char	ReadPnt;
    unsigned int	BitIndex;
    unsigned char*	Buff;
};

class RingBuff_16bit
{
public:
    RingBuff_16bit(void);
    ~RingBuff_16bit(void);

    void __fastcall	Init(void);
    inline void __fastcall
    SetPointer(void)
    {
        ReadPnt = BuffEnd;
        BitIndex = 0;
    }
    inline bool __fastcall
    SetPointer(const unsigned int BackStep)
    {
        if((BuffWrite < (1 << 16)) && (BackStep > ReadPnt)) return false;

        ReadPnt -= BackStep;
        BitIndex = 0;

        return true;
    }
    void __fastcall	Add(unsigned char* AddBuff, const unsigned int Size);
    inline unsigned char __fastcall
    get1bit(void)
    {
        const unsigned char	Value = *(Buff + ReadPnt) << BitIndex;

        ReadPnt += ++BitIndex >> 3;
        BitIndex &= 7;

        return Value >> 7;
    }
    inline unsigned int __fastcall
    getbits_fast(const int NumberOfBits)
    {
        if(NumberOfBits <= 0) return 0;

        unsigned int	Value = (*(Buff + ReadPnt) << 8) |
                                *(Buff + static_cast<unsigned short>(ReadPnt + 1));

        Value = static_cast<unsigned short>(Value << BitIndex) >> (16 - NumberOfBits);

        ReadPnt += (BitIndex += NumberOfBits) >> 3;
        BitIndex &= 7;

        return Value;
    }
    inline unsigned int __fastcall
    getbits(const int NumberOfBits)
    {
        if(NumberOfBits <= 0) return 0;

        unsigned int	Value = (*(Buff + ReadPnt) << 16) |
                                (*(Buff + static_cast<unsigned short>(ReadPnt + 1)) << 8) |
                                *(Buff + static_cast<unsigned short>(ReadPnt + 2));

        Value = ((Value << BitIndex) & 0xffffff) >> (24 - NumberOfBits);

        ReadPnt += (BitIndex += NumberOfBits) >> 3;
        BitIndex &= 7;

        return Value;
    }
    inline void __fastcall
    Cache_Start(void)
    {
        CacheSize = 8 - BitIndex;
        Cache = getbits_fast(CacheSize) << (sizeof Cache * 8 - CacheSize);
    }
    inline void __fastcall
    Cache_End(void)
    {
        const unsigned short	SubPnt = CacheSize / 8;

        ReadPnt -= SubPnt;
        BitIndex = 8 - (CacheSize - SubPnt * 8);

        if(BitIndex == 8)
        {
            BitIndex = 0;
        }
        else
        {
            ReadPnt--;
        }
    }
    inline void __fastcall
    Cache_Charge(void)
    {
        while(CacheSize < (sizeof Cache * 8 - 8))
        {
            Cache |= *(Buff + ReadPnt++) << (sizeof Cache * 8 - 8 - CacheSize);
            CacheSize += 8;
        }
    }
    inline unsigned int __fastcall
    Cache_get1bit(void)
    {
        const unsigned int	_Cache = Cache;

        CacheSize--;
        Cache <<= 1;

        return _Cache >> (sizeof Cache * 8 - 1);
    }
    inline unsigned int __fastcall
    Cache_getbits_fast(const int NumberOfBits)
    {
        const unsigned int	_Cache = Cache;

        CacheSize -= NumberOfBits;
        Cache <<= NumberOfBits;

        return _Cache >> (sizeof Cache * 8 - NumberOfBits);
    }

private:
    unsigned int	BuffWrite;
    unsigned short	BuffEnd;
    unsigned short	ReadPnt;
    unsigned int	BitIndex;
    unsigned int	CacheSize;
    unsigned int	Cache;
    unsigned char*	Buff;
};

typedef	int	(__fastcall mpg123::*DO_LAYER123_FUNC)(unsigned char* pcm_sample, int* pcm_point);

typedef	void	(__fastcall mpg123::*SYNTH_1TO1_MONO_FUNC)
(real* bandPtr, unsigned char* samples, int* pnt);
typedef	void	(__fastcall mpg123::*SYNTH_1TO1_FUNC)
(real* bandPtr, int channel, unsigned char* out, int* pnt);

extern int synth_2to1 (real *,int,unsigned char *,int *);
extern int synth_2to1_8bit (real *,int,unsigned char *,int *);
extern int synth_2to1_mono (real *,unsigned char *,int *);
extern int synth_2to1_mono2stereo (real *,unsigned char *,int *);
extern int synth_2to1_8bit_mono (real *,unsigned char *,int *);
extern int synth_2to1_8bit_mono2stereo (real *,unsigned char *,int *);

extern int synth_4to1 (real *,int,unsigned char *,int *);
extern int synth_4to1_8bit (real *,int,unsigned char *,int *);
extern int synth_4to1_mono (real *,unsigned char *,int *);
extern int synth_4to1_mono2stereo (real *,unsigned char *,int *);
extern int synth_4to1_8bit_mono (real *,unsigned char *,int *);
extern int synth_4to1_8bit_mono2stereo (real *,unsigned char *,int *);

extern int synth_ntom (real *,int,unsigned char *,int *);
extern int synth_ntom_8bit (real *,int,unsigned char *,int *);
extern int synth_ntom_mono (real *,unsigned char *,int *);
extern int synth_ntom_mono2stereo (real *,unsigned char *,int *);
extern int synth_ntom_8bit_mono (real *,unsigned char *,int *);
extern int synth_ntom_8bit_mono2stereo (real *,unsigned char *,int *);

extern void rewindNbits(int bits);
extern int  hsstell(void);
extern int get_songlen(struct frame *fr,int no);

extern void __fastcall init_layer3(const unsigned int);
extern void __fastcall init_layer2(void);
extern void __fastcall make_decode_tables(long scale);
extern void make_conv16to8_table(int);
extern void __fastcall dct64(real *,real *,real *);

extern void synth_ntom_set_step(long,long);

extern unsigned char *conv16to8;
extern const long freqs[9];
extern real muls[27][64];
extern real decwin[512+32];
extern real *pnts[5];

extern const struct parameter param;

#endif // MYMPG123_H_INCLUDED
