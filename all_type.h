#ifndef ALL_TYPE_H_INCLUDED
#define ALL_TYPE_H_INCLUDED

#include "myradio_string.h"

typedef unsigned long MRSampleFormat;

#define mrFloat32        ((MRSampleFormat) 0x00000001)
#define mrInt32          ((MRSampleFormat) 0x00000002)
#define mrInt24          ((MRSampleFormat) 0x00000004) /**< Packed 24 bit format. */
#define mrInt16          ((MRSampleFormat) 0x00000008)
#define mrInt8           ((MRSampleFormat) 0x00000010)
#define mrUInt8          ((MRSampleFormat) 0x00000020)
#define mrCustomFormat   ((MRSampleFormat) 0x00010000)

#define mrNonInterleaved ((MRSampleFormat) 0x80000000)

enum protocol_type
{
    PSHOUTCAST,
    PICECAST,
    PMSHTTP,
    PMMS,
    PRTSP,
    PTCP,
    PUDP,
};

enum decoder_type
{
    DECODER_UNKNOWN = 0,

    DECODER_MP3,
    DECODER_VORBIS,
    DECODER_AAC,

    DECODER_WMA,
    DECODER_WMA_V1,
    DECODER_WMA_V2,
    DECODER_WMA_SPEECH,

    DECODER_RA,
    DECODER_RA_SIPR,
    DECODER_RA_DNET,
    DECODER_RA_COOK,
    DECODER_RA_ATRC
};

enum format
{
    FUNKNOWN,
    FDUMP,

    /* decoders */
    FMP3,
    FVORBIS,
    FWMA,
    FRA,
    FAAC,
    FRACOOK,
    FRASIPR,
    FRADNET,
    FRAATRC,
    FWMAV1,
    FWMAV2,
    FWMASPEECH,

    /* demuxers */
    FASF,
    FRMFF,

    FMP3ADU,
};

typedef enum MRHostApiTypeId
{
    mrInDevelopment=0,
    mrMME=1,
    mrDirectSound=2,
    mrASIO=3,
    mrWASAPI=4,
} MRHostApiTypeId;

typedef struct MRDeviceInfo
{
    wstring *name;
    bool is_ok;
    MRHostApiTypeId devtype;
    int devidx;

    //int maxInputChannels;
    int maxOutputChannels;

    double defaultLowOutputLatency;
    double defaultHighOutputLatency;
    double defaultSampleRate;

    void *host_devinfo;
} MRDeviceInfo;

typedef struct MRStreamParameters
{
    int device;
    unsigned short channelCount;
    double sampleRate;
    MRSampleFormat sampleFormat;
    double suggestedLatency;
    //int suggestedLatency;
    void *host_streaminfo;

} MRStreamParameters;

/* ////////////////////////// */

typedef enum
{
    V_UNKNOWN,
    V_WINDOWS_95,
    V_WINDOWS_98,
    V_WINDOWS_ME,
    V_WINDOWS_NT,
    V_WINDOWS_2000,
    V_WINDOWS_XP,
    V_WINDOWS_VISTA,
    V_WINDOWS_7,
    V_WINDOWS_8,
    V_WINDOWS_8_1,
    V_WINDOWS_10,
} version_e;

typedef enum
{
    A_UNKNOWN,
    A_WINDOWS_X86,
    A_WINDOWS_X86_64,
} architecture_e;

typedef struct
{
    version_e version;
    architecture_e arch;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwPlatformId;
} sys_info_t;


/* ////////////////////////// */


struct winpos_t
{
    DWORD w_width,w_height;
    DWORD wleft,wtop;
    DWORD wstate;
    DWORD wsp_pos;
    bool stayontop;
};

struct colors_t
{
    COLORREF textcolor;
    COLORREF backcolor;
    COLORREF ramkacolor;
};

/* myradio_config.cpp */
struct proxy_t
{
    bool proxy_on;
    unsigned short proxy_type; //0 - not use, 1 - sock ver 4, 2 - sock ver 5, 3 - http connect proxy, 4 - http proxy,
    wchar_t proxy_ip[32];
    unsigned short proxy_port;
    bool proxy_athoriz_on;
    wchar_t proxy_username[128];
    wchar_t proxy_password[128];
};


/* MYQueryStr */
class MYQueryStr
{
public:
    MYQueryStr();
    ~MYQueryStr();
    const wchar_t* Format(const wchar_t* fmt, ...);
    const wchar_t* Get() const;

private:
    wchar_t* m_buf;
};
/* MYQueryStr */

class Config
{
public:
    Config(const wstring pcurdir);
    ~Config();

    winpos_t winpos;
    colors_t colors;

    bool uselog;
    bool usefulllog;

    DWORD cur_radio;
    DWORD volume;

    DWORD priority;		    /* decoder thread priority -2..+2 */
    DWORD move_snap;
    DWORD winamp_e;
    DWORD auto_play_last_radio;
    DWORD icon_in_tray;
    DWORD autoruns;
    //DWORD poslock;

    //DWORD channel;		    /* channel selection */
    //DWORD resolution;		/* bits per output sample */

    DWORD sbuffer;        /* stream input buffer length */
    DWORD sprebuffer;       /* stream output buffer length */

    //DWORD multimedia_global_hotkey;
    DWORD inet_timeout;

    proxy_t proxy_settings;

    wchar_t user_agent[64];

    //output
    DWORD block_count_c;
    DWORD block_size_c;
    DWORD cur_device;
    DWORD disable_hard_mix;
    //

    void LoadLogSetting();
    //
    bool LoadSettings();
    void SaveSettings();
    //void SaveOtherSettings();
    void SaveOutputSettings();
    void SaveConnectSettings();
    //
    void MyAddLogHeader(MyString value, bool req);
    void MyAddLog(MyString value, bool addtime = TRUE);
    void MyAddLogParam(const wchar_t* fmt, ...);
    void MyAddLogParamNoTime(const wchar_t* fmt, ...);
    void MyAddLogInt(int value, bool addtime = TRUE);

    bool LoadDWORD(const wstring name, DWORD *value, DWORD def);
    bool LoadSZ(const wstring name, wstring *value, const wstring def);
    bool LoadDATA(const wstring name, void* value, size_t size);
    bool SaveDWORD(const wstring name, DWORD value);
    bool SaveSZ(const wstring name, const wstring value);
    bool SaveDATA(const wstring name, void* value, size_t size);

private:
    wstring inifile;
    wstring logfile;
};

extern Config *GlobalConfig;

#endif // ALL_TYPE_H_INCLUDED
