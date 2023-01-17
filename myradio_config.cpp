#include "myhdr.h"
#include "myradio.h"
#include <stdlib.h>
#include <time.h>
#include "all_type.h"

Config *GlobalConfig;

Config::Config(const wstring pcurdir)
{
    inifile.append(pcurdir);
    logfile.append(pcurdir);
    CreateDirectory(inifile.c_str(), 0);
    inifile+=_T("\\myradio.ini");
    logfile+=_T("\\log.txt");
}

Config::~Config()
{
    inifile.clear();
    logfile.clear();
}

void Config::LoadLogSetting()
{
    DWORD tmp;
    LoadDWORD(_T("UseLog"), &tmp, 0);
    uselog = bool(tmp);
    LoadDWORD(_T("UseFullLog"), &tmp, 0);
    usefulllog = bool(tmp);
}

bool Config::LoadSettings()
{
    bool ret = true;

    if(!LoadDATA(_T("Win"), &winpos, sizeof(winpos)))
    {
        int cxScreen = GetSystemMetrics(SM_CXSCREEN);//GetSystemMetrics(SM_CXFULLSCREEN);
        int cyScreen = GetSystemMetrics(SM_CYSCREEN);//GetSystemMetrics(SM_CYFULLSCREEN);

        winpos.w_height    = cyScreen * 0.5;;
        winpos.w_width     = cxScreen * 0.5;
        winpos.wleft       = ((cxScreen - winpos.w_width) / 2);
        winpos.wtop        = ((cyScreen -  winpos.w_height) / 2);
        winpos.wstate      = 0; // normal state
        winpos.stayontop   = 0;
        winpos.wsp_pos     = winpos.w_width * 0.25;

        ret = false;
    }

    if(!LoadDATA(_T("Proxy"), &proxy_settings, sizeof(proxy_settings)))
    {
        proxy_settings.proxy_on = false;
        proxy_settings.proxy_type = LL_PROXY_NONE;
        proxy_settings.proxy_ip[0] = 0;
        proxy_settings.proxy_port = 0;
        proxy_settings.proxy_athoriz_on = false;
        proxy_settings.proxy_username[0] = 0;
        proxy_settings.proxy_password[0] = 0;

        ret = false;
    }

    return ret;
}

void Config::SaveOutputSettings()
{
//
}

void Config::SaveConnectSettings()
{
    SaveDATA(_T("Proxy"), (void*)&proxy_settings, sizeof(proxy_settings));
}

void Config::SaveSettings()
{
    //main windows
    SaveDATA(_T("Win"), (void*)&winpos, sizeof(winpos));
    SaveDWORD(_T("UseLog"), DWORD(uselog));
    SaveConnectSettings();
}

bool Config::LoadDWORD(const wstring name, DWORD *value, DWORD def)
{
    wchar_t lpReturnedString[100];
    if(!GetPrivateProfileString(INI_SECTION_NAME, name.c_str(), _T("\0"), lpReturnedString, 100, inifile.c_str()))
    {
        *value = def;
        return false;
    }

    *value = (DWORD)_wtoi(lpReturnedString);
    return true;
}

bool Config::LoadSZ(const wstring name, wstring *value, const wstring def)
{
    if(name.empty())
    {
        value->clear();
        return false;
    }

    wchar_t tmp[4096];
    if(!GetPrivateProfileString(INI_SECTION_NAME, name.c_str(), _T("\0"), tmp, 4096, inifile.c_str()))
    {
        value->append(def);
        return false;
    }
    value->append(tmp);
    return true;
}

bool Config::LoadDATA(const wstring name, void* value, size_t size)
{
    return GetPrivateProfileStruct(INI_SECTION_NAME, name.c_str(), value, size, inifile.c_str());
}

bool Config::SaveDWORD(const wstring name, DWORD value)
{
    if(name.empty())
    {
        return false;
    }

    wchar_t lpszString[100];
    swprintf(lpszString, _T("%i"),(int)value);
    return WritePrivateProfileString(INI_SECTION_NAME, name.c_str(), lpszString, inifile.c_str());
}

bool Config::SaveSZ(const wstring name, wstring value)
{
    if(name.empty() || value.empty())
    {
        return false;
    }

    return WritePrivateProfileString(INI_SECTION_NAME, name.c_str(), value.c_str(), inifile.c_str());
}

bool Config::SaveDATA(const wstring name, void* value, size_t size)
{
    return WritePrivateProfileStruct(INI_SECTION_NAME, name.c_str(), value, size, inifile.c_str());
}

void Config::MyAddLogHeader(MyString value, bool req)
{
    if(!uselog) return;
    GlobalConfig->MyAddLog(req?_T("SEND REQUEST >>\r\n"):_T("RECV HEADER >>\r\n"));
    GlobalConfig->MyAddLog(value, false);
    GlobalConfig->MyAddLog(req?_T("SEND REQUEST END"):_T("RECV HEADER END"), false);
}

void Config::MyAddLog(MyString value, bool addtime)
{
    if(!uselog) return;
    if(value.str_empty()) return;

    FILE *f = _wfopen(logfile.c_str(), _T("a"));

    if(addtime)
    {
        MyString str(_T("[")+MyDateAndTime2WStrShort()+_T("]: "));

        str+=value;
        str.utf8_str();
        fwrite(str.utf8_str(), 1, str.Get_utf8_str_size(), f);  // write UTF-8 text
        fputs("\r\n", f);
    }
    else
    {
        MyString str(value);
        str.utf8_str();
        fwrite(str.utf8_str(), 1, str.Get_utf8_str_size(), f); // write UTF-8 text
        fputs("\r\n", f);
    }
    fclose(f);
}

void Config::MyAddLogParam(const wchar_t* fmt, ...)
{
    if(!uselog) return;

    wchar_t* m_buf = NULL;

    va_list va;
    va_start(va, fmt);

    int len = _vscwprintf(fmt, va) + 1;
    m_buf = (wchar_t*)malloc(len * sizeof(wchar_t));
    _vsnwprintf(m_buf, len, fmt, va);

    va_end(va);

    MyAddLog(MyString(m_buf));

    if(m_buf) free(m_buf);
}

void Config::MyAddLogParamNoTime(const wchar_t* fmt, ...)
{
    if(!uselog) return;

    wchar_t* m_buf = NULL;

    va_list va;
    va_start(va, fmt);

    int len = _vscwprintf(fmt, va) + 1;
    m_buf = (wchar_t*)malloc(len * sizeof(wchar_t));
    _vsnwprintf(m_buf, len, fmt, va);

    va_end(va);

    MyAddLog(MyString(m_buf), false);

    if(m_buf) free(m_buf);
}

void Config::MyAddLogInt(int value, bool addtime)
{
    if(!uselog) return;
    wchar_t lpszString[100];
    swprintf(lpszString, _T("%i"),(int)value);
    MyAddLog(lpszString, addtime);
}
