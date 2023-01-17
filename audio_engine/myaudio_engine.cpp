#include "common.h"
#include "../my_global_utils.h"
#include "protocol.h"
#include "decoders/decoder.h"
#include "output/main_out.h"
#include "output/output_wmme.h"
#include "myaudio_engine.h"


wstring dwFormats2wstring(DWORD val)
{
    wstring str(_T(""));
    if (val & WAVE_FORMAT_1M08) str=str+_T("11.025 kHz, mono, 8-bit\n\t\t\t");
    if (val & WAVE_FORMAT_1S08) str=str+_T("11.025 kHz, stereo, 8-bit\n\t\t\t");
    if (val & WAVE_FORMAT_1M16) str=str+_T("11.025 kHz, mono, 16-bit\n\t\t\t");
    if (val & WAVE_FORMAT_1S16) str=str+_T("11.025 kHz, stereo, 16-bit\n\t\t\t");
    if (val & WAVE_FORMAT_2M08) str=str+_T("22.05 kHz, mono, 8-bit\n\t\t\t");
    if (val & WAVE_FORMAT_2S08) str=str+_T("22.05 kHz, stereo, 8-bit\n\t\t\t");
    if (val & WAVE_FORMAT_2M16) str=str+_T("22.05 kHz, mono, 16-bit\n\t\t\t");
    if (val & WAVE_FORMAT_2S16) str=str+_T("22.05 kHz, stereo, 16-bit\n\t\t\t");
    if (val & WAVE_FORMAT_4M08) str=str+_T("44.1 kHz, mono, 8-bit\n\t\t\t");
    if (val & WAVE_FORMAT_4S08) str=str+_T("44.1 kHz, stereo, 8-bit\n\t\t\t");
    if (val & WAVE_FORMAT_4M16) str=str+_T("44.1 kHz, mono, 16-bit\n\t\t\t");
    if (val & WAVE_FORMAT_4S16) str=str+_T("44.1 kHz, stereo, 16-bit\n\t\t\t");
    if (val & WAVE_FORMAT_96M08) str=str+_T("96 kHz, mono, 8-bit\n\t\t\t");
    if (val & WAVE_FORMAT_96M16) str=str+_T("96 kHz, mono, 16-bit\n\t\t\t");
    if (val & WAVE_FORMAT_96S08) str=str+_T("96 kHz, stereo, 8-bit\n\t\t\t");
    if (val & WAVE_FORMAT_96S16) str=str+_T("96 kHz, stereo, 16-bit\n\t\t\t");
    if (val & WAVE_FORMAT_PCM) str=str+_T("PCM\n\t\t\t");
    if (val & WAVE_FORMAT_EXTENSIBLE) str=str+_T("EXTENSIBLE");

    return str;
}

class station
{
public:
    station(main_output *out)
    {
        mod_decoder = NULL;
        mod_output  = out;

        //dump_stream = false;
        do_record = false;
        trigger_record = false;

        running = false;

        meta_ptr = (void *)&meta;

    }
    ~station()
    {
        //LOG_DEBUG("~station");
    }

    /* The idea here is to do everything dangerous in the loop (loop is
    called from the helper thread, everything else is called from the main
    thread). Because there are very few actions that can interrupt the loop
    we don't have to implement a more advanced event queue. We use threads
    to avoid blocking the main thread, not to handle advanced
    asynchronous events. */
    void record()
    {
        trigger_record = true;
    }

    bool is_recording()
    {
        return false; //return (mod_recorder != NULL);
    }

    /*void (*error_callback)(int);
    void set_error_callback(void (*callback)(int))
    {
        error_callback = callback;
    }*/

    /*void set_dump()
    {
        dump_stream = true;
    }*/
    int open(std::string &url)
    {
        input_info details;
        std::string mprotocol;
        int ret = parse_url(url, mprotocol, details.hostname, details.port, details.filepath);

        if(GlobalConfig->uselog)
        {
            wchar_t *wurl  = (wchar_t *)ansi2wchar((char *)url.c_str());
            wchar_t *wprot = (wchar_t *)ansi2wchar((char *)mprotocol.c_str());
            wchar_t *whost = (wchar_t *)ansi2wchar((char *)details.hostname.c_str());
            wchar_t *wpath = (wchar_t *)ansi2wchar((char *)details.filepath.c_str());
            GlobalConfig->MyAddLogParam(_T("Parse url: url = %s, protocol = %s, host = %s, port = %d, path = %s, error? = %d"),
                                        wurl, wprot, whost, details.port, wpath, int(ret!=1));
            if(wurl) free(wurl);
            if(wprot) free(wprot);
            if(whost) free(whost);
            if(wpath) free(wpath);
        }

        if (ret != 1)
        {
            //LOG_DEBUG("parse url failed (%s)", url.c_str());
            return ret;
        }

        std::string proto = str_lowercase(mprotocol);
        if (proto == "http" || proto == "icy")// || proto == "peercast")
        {
            GlobalConfig->MyAddLog(_T("create: http_input.."));
            mod_protocol = new input_http();
        }
        /*else if (proto == "mms" || proto == "mmst")
            mod_protocol = new input_mmst();
        else if (proto == "rtsp")
            mod_protocol = new input_rtsp();
        else if (proto == "lastfm")
            mod_protocol = new input_lastfm();
        else if (proto == "file")
            mod_protocol = new input_file();
        else*/
        else return ERROR_PARSEURL_INVALIDURL;

        attach_all();

        int open_ret = mod_protocol->open(details);
        if (open_ret < 0)
            return -1;

        running = true;

        return 1;
    }

    int loop()
    {
        while (running)
        {
            if (trigger_record)
            {
                local_record();
                trigger_record = false;
            }

            if (mod_protocol)
            {
                mod_protocol->tick();
                if (mod_protocol->eof())//(mod_protocol->failed())
                {

                }
                //{
                /* flush protocol output to playlist */
                //     if (!mod_protocol->stream_out->empty())
                //     {
                //         while (!mod_protocol->stream_out->empty())
                //             playlist += (char)mod_protocol->stream_out->getbyte();
                //     }

                //     return -1;
                //}
                else if (mod_protocol->failed())
                {
                    return -1;
                }

                if (mod_protocol->reset())
                {
                    //LOG_DEBUG("resetting");
                    if (mod_decoder)
                    {
                        delete mod_decoder;
                        mod_decoder = NULL;
                    }

                    s2.clear();
                    attach_all();

                    mod_protocol->reset(false);
                }
            }

            if (mod_protocol && mod_protocol->get_format() != FUNKNOWN)
            {
                mod_decoder = decoder_create(mod_protocol->get_format());
                mod_decoder->set_flags(0);
                attach_all();
            }

            if(!mod_decoder)
                continue;

            /*
             * decode and output
             */
            if (mod_decoder && mod_decoder->stream_in)
            {
                /* mod_decoder wants to know the waveout buffer so it knows when to stop
                decoding, to save RAM. */
                //!!!!!mod_decoder->set_output_buffer_size(mod_output->buffer_size());

                int decode_ret = mod_decoder->decode();
                if (decode_ret == DECODER_HEADERMISMATCH || decode_ret == DECODER_INTERNALERROR)
                {
                    /* danger! */
                    //LOG_ERROR("Decoder returned %d", decode_ret);
                    running = false;
                    return -1;
                }

                /* re-buffer */
                if (mod_output->samples_to_seconds() < 0.2f)
                    mod_output->pause();

                //if (mod_waveout->is_paused() || mod_waveout->is_stopped())
                //    LOG_DEBUG("buffering...");
                /* */

                if (!s2.empty())
                {
                    // record
                    //
                    // waveout:
                    mod_output->set_parameters(mod_decoder->get_samplerate(), mod_decoder->get_channels());
                    mod_output->push();

                    if (mod_output->samples_to_seconds() > 5)
                    {
                        mod_output->play();
                    }
                    /*else// if (buffer_samples < 0.2f)
                    {
                        if (buffer_samples < 0.5f)
                        {
                            LOG_DEBUG("buffering...");
                            mod_waveout->stop();
                        }
                    }*/
                }
            }
        }

        return 0;
    }

    int close()
    {
        running = false;

        if (do_record)
            local_record();

        if (mod_output)
            mod_output->stop();

        s1.clear();
        s2.clear();

        if (mod_decoder)
        {
            delete mod_decoder;
            mod_decoder = NULL;
        }

        if (mod_protocol)
        {
////                closing_because_of_failure = mod_protocol->failed();
            mod_protocol->close();
            delete mod_protocol;
        }

        return 0;
    }
    volatile bool running;

    station_meta meta;
private:

    bool do_record;
    volatile bool trigger_record;
    //bool dump_stream;

    /* Must be called from loop(). */
    void local_record()
    {
        /*if (do_record)
        {
            LOG_DEBUG("stopped recording");
            do_record = false;
            if (mod_recorder)
            {
                delete mod_recorder;
                mod_recorder = NULL;
            }
        }
        else
        {
            LOG_DEBUG("started recording");
            do_record = true;
            if (mod_recorder)
            {
                delete mod_recorder;
                mod_recorder = NULL;
            }
            mod_recorder = new encoder_mp3(meta.info.name);
        }*/
    }

    void attach_all()
    {
        mod_protocol->set_output(&s1);
        mod_protocol->set_common(meta_ptr);
        if (mod_decoder)
        {
            mod_decoder->set_input(&s1);
            mod_decoder->set_output(&s2);

            //mod_decoder->codec_info = &(mod_demuxer->info);
            //mod_decoder->set_parameters(mod_demuxer->get_parameters());
            mod_decoder->set_common(meta_ptr);
        }
        mod_output->set_input(&s2);
    }

    decoder     *mod_decoder;
    main_output *mod_output;
    input       *mod_protocol;
    //demuxer *mod_demuxer;

    //encoder_mp3 *mod_recorder;

    stream s1, s2;

    void *meta_ptr;

    //mutex *mtx;
    //thread *loop_thread;
};


bool MyAudioEngine::set_last_error(bool ret_val, int vlast_error)
{
    last_error = vlast_error;
    return ret_val;
}

MyAudioEngine::MyAudioEngine(sys_info_t *sysi)
{
    memcpy(&sysinfo, sysi, sizeof(sys_info_t));
    last_error = AE_NOERROR;
    last_error_str = _T("");
    busy = false;
}

MyAudioEngine::~MyAudioEngine()
{
    main_output *mo;
    mo = output_create(mrMME);

    unsigned short j;
    for(j = 0; j < mme_devcount; j++)
    {
        mo->freehostapi(&output_device_list[j]);
    }
    delete(mo);

    MROutput_FreeMemory(output_device_list);
    output_device_list = NULL;
}

bool MyAudioEngine::OpenURL(MyString &vurl)
{
    if(busy) return set_last_error(false, AE_BUSY);
    int sockm = SOCKET_ERROR, serr;

    SockProxy *prs = new SockProxy();
    prs->PreFileDownload(vurl);
    prs->SetProxyfromConfig();



    prs->StopFileDownload(sockm);
    delete prs;
    busy = true;
}

bool MyAudioEngine::CloseURL()
{

    busy = false;
    return true;
}

bool MyAudioEngine::InitOutputDevice()
{
    bool ret = true;
    alldevcount=mme_devcount=ds_devcount=0;
    double defaultLowLatency, defaultHighLatency;

    main_output *mo;
    mo = output_create(mrMME);
    mme_devcount = mo->getdevcount();
    GlobalConfig->MyAddLogParam(_T("AE.Output: mme dev count = %i"), mme_devcount);
    delete(mo);

    alldevcount = mme_devcount + ds_devcount;
    GlobalConfig->MyAddLogParam(_T("AE.Output: all dev count = %i"), alldevcount);

    output_device_list = (MRDeviceInfo*)MROutput_AllocateMemory(sizeof(MRDeviceInfo) * alldevcount);

    mo = output_create(mrMME);
    int dev_ok = 0;

    if(mo || mme_devcount > 0)
    {
        mo->getdefaultlatencies(&sysinfo, &defaultLowLatency, &defaultHighLatency);
        unsigned short j;
        for(j = 0; j < mme_devcount; j++)
        {
            MRDeviceInfo *dev_info = &output_device_list[j];
            if(mo->getdevinfo(&sysinfo, j, dev_info, &dev_ok) != outer_NoError)
            {
                ret = false;
                break;
            }
            else
            {
                dev_info->devidx = j;
                dev_info->is_ok = (bool)dev_ok;
                dev_info->defaultLowOutputLatency = defaultLowLatency;
                dev_info->defaultHighOutputLatency = defaultHighLatency;
            }

            if(GlobalConfig->uselog)
            {
                MRWinMmeDeviceInfo *wmdi = (MRWinMmeDeviceInfo*)dev_info->host_devinfo;
                GlobalConfig->MyAddLogParam(_T("AE.Output: mme device (%i) info:"), j);
                GlobalConfig->MyAddLogParamNoTime(_T("\t\t\tname = %s\n\t\t\tok? = %i\n\t\t\tdefaultSampleRate = %i"
                                                     "\n\t\t\tmaxOutputChannels = %i"
                                                     "\n\t\t\tdwFormats:\n\t\t\t%s"),
                                                  dev_info->name->c_str(),
                                                  (int)(dev_info->is_ok),
                                                  (int)dev_info->defaultSampleRate,
                                                  dev_info->maxOutputChannels,
                                                  dwFormats2wstring(wmdi->dwFormats).c_str());
            }

        }

        delete(mo);
    }

    /*unsigned int i;
    for(i = 0; i < alldevcount; i++)
    {
        main_output *mo;
        mo = output_create((i == 0) ?  mrMME : mrDirectSound);
        if(mo)
        {
            int devcount = mo->getdevcount();

            unsigned int j;
            for(j = 0; j < devcount; j++)
            {

                mo->getdevinfo(sysinfo, y, )
            }
            delete(mo);
        }
    }*/

    return ret;
}
