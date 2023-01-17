#ifndef MYAUDIO_ENGINE_H_INCLUDED
#define MYAUDIO_ENGINE_H_INCLUDED

#include "../all_type.h"

#define AE_NOERROR  0
#define AE_BUSY     1

class MyAudioEngine
{
public:
    MyAudioEngine(sys_info_t *sysi);
    ~MyAudioEngine();

    bool InitOutputDevice();

    bool OpenURL(MyString &vurl);
    bool CloseURL();
private:
    int last_error;
    wstring last_error_str;
    sys_info_t sysinfo;

    bool busy;

    MRDeviceInfo *output_device_list;

    unsigned short alldevcount, mme_devcount, ds_devcount;

    bool set_last_error(bool ret_val, int vlast_error);
};

#endif // MYAUDIO_ENGINE_H_INCLUDED
