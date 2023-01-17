#include "myhdr.h"
#include "myradio.h"
#include <stdarg.h>
#include <algorithm>

/*  */
MYQueryStr::MYQueryStr()
    : m_buf(NULL)
{
}

const wchar_t* MYQueryStr::Format(const wchar_t* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    free(m_buf);

    int len = _vscwprintf(fmt, va) + 1;
    m_buf = (wchar_t*)malloc(len * sizeof(wchar_t));
    //vswprintf(m_buf, len, fmt, va);
    _vsnwprintf(m_buf, len, fmt, va);

    va_end(va);
    return m_buf;
}

const wchar_t* MYQueryStr::Get() const
{
    return m_buf;
}

MYQueryStr::~MYQueryStr()
{
    free(m_buf);
}
/*  */

// wstring tools

template <typename Func>
void inplaceW(wstring &string, Func func)
{
    if(string.length() > MAXDWORD)
    {
        return;//throw uniconv_error("String too long");
    }
    func(&string[0], string.length());
}


void toUpper(wstring &string)
{
    return inplaceW(string, CharUpperBuffW);
}

void toLower(wstring &string)
{
    return inplaceW(string, CharLowerBuffW);
}

struct lowercasew_func_lat
{
    void operator()(wstring::value_type &v)
    {
        v = towlower(v);
    }
};

void make_lowercaseW_LATIN(wstring &s)
{
    for_each(s.begin(), s.end(), lowercasew_func_lat());
}

void wStringSplit(wstring str, wstring delim, std::vector<wstring>* results)
{
    unsigned int cutAt;
    while( (cutAt = str.find_first_of(delim)) != str.npos )
    {
        if(cutAt > 0)
        {
            results->push_back(str.substr(0,cutAt));
        }
        str = str.substr(cutAt+1);
    }
    if(str.length() > 0)
    {
        results->push_back(str);
    }
}

int VectorFindNameIdx(std::vector<wstring>* vect, wstring find_me)
{
    int idx = -1;
    for(unsigned int i = 0; i < vect->size(); i++)
    {
        if(vect->at(i).compare(find_me) == 0)
        {
            idx = i;
            break;
        }
    }
    if(idx != -1)
        return idx;

    vect->push_back(find_me);
    return vect->size()-1;
}

//

void ReplaceStringInPlace(wstring& subject, const wchar_t* search,
                          const wchar_t* replace, int search_len, int replace_len)
{
    size_t pos = 0;
    while((pos = subject.find(search, pos)) != std::wstring::npos)
    {
        subject.replace(pos, search_len, replace);
        pos += replace_len;
    }
}

wstring myhtml_decode(wchar_t* src, bool rep_space)
{
    const wchar_t* NAMED_ENTITIES[][2] =
    {
        { _T("&#039;"), _T("`") },
        { _T("&quot;"), _T("\"") },
        { _T("&amp;"), _T("&") },
    };

    int NAMED_ENTITIES_LEN[][2] =
    {
        {6, 1},
        {6, 1},
        {5, 1},
    };

    wstring tmp(src);
    if(!tmp.length()) return _T("");
    int lenne = sizeof(NAMED_ENTITIES) / sizeof(*NAMED_ENTITIES);
    for(int i=0; i != lenne; i++)
    {
        ReplaceStringInPlace(tmp, NAMED_ENTITIES[i][0], NAMED_ENTITIES[i][1], NAMED_ENTITIES_LEN[i][0], NAMED_ENTITIES_LEN[i][1]);
    }
    if(rep_space)
        ReplaceStringInPlace(tmp, _T(" "), _T(","), 1, 1);
    return tmp;
}

long long SysTime264Time(const SYSTEMTIME *time)
{
    long long _64Time = 0;

    _64Time = time->wMilliseconds;
    _64Time += time->wSecond * 1000;
    _64Time += time->wMinute * 100000;
    _64Time += time->wHour * 10000000;
    _64Time += (long long)time->wDay * 1000000000;
    _64Time += (long long)time->wMonth * 100000000000;
    _64Time += (long long)time->wYear * 10000000000000;

    return _64Time;
}

void Time642SysTime(long long in, SYSTEMTIME* out)
{
    long long nTemp = 0;

    out->wYear = (WORD)(in / 10000000000000);
    nTemp = in % 10000000000000;
    out->wMonth = (WORD)(nTemp / 100000000000);
    nTemp = in % 100000000000;
    out->wDay = (WORD)(nTemp / 1000000000);

    nTemp = in % 1000000000; // 20160822113409983
    out->wHour = (WORD)(nTemp / 10000000);
    nTemp = nTemp % 10000000;
    out->wMinute = (WORD)(nTemp / 100000);
    nTemp = (WORD)(nTemp % 100000);
    out->wSecond = (WORD)(nTemp / 1000);
    out->wMilliseconds =  (WORD)(nTemp % 1000);
}

wstring mySHGetFolderPath(int dir_type)
{
    DWORD code = 1;
    switch(dir_type)
    {
    case 1:
        code = CSIDL_APPDATA;
        break;
    case 2:
        code = CSIDL_DESKTOP;
        break;
    }
    wchar_t szPath[MAX_PATH];
    if(SUCCEEDED(SHGetFolderPath(NULL, code, NULL, 0, szPath)))
    {
        return wstring(szPath);
    }
    return _T("");
}

wstring myGetStartDir(HINSTANCE phInst)
{
    wchar_t temp[MAX_PATH];
    GetModuleFileName(phInst, temp, MAX_PATH);
    return wstring(temp);
}

/*
wstring utf8to16(const char* src)
{
    std::vector<wchar_t> buffer;
    buffer.resize(MultiByteToWideChar(CP_UTF8, 0, src, -1, 0, 0));
    MultiByteToWideChar(CP_UTF8, 0, src, -1, &buffer[0], buffer.size());
    return &buffer[0];
}

std::string to_utf8(const wchar_t* buffer, int len)
{
    int nChars=WideCharToMultiByte(CP_UTF8, 0, buffer, len, NULL, 0, NULL, NULL);
    if (nChars == 0) return "";
    std::string newbuffer;
    newbuffer.resize(nChars) ;
    WideCharToMultiByte(CP_UTF8, 0, buffer, len, const_cast<char*>(newbuffer.c_str()), nChars, NULL, NULL);
    return newbuffer;
}

std::string wstring2string(const wstring& str)
{
    return to_utf8(str.c_str(), (int)str.size());
}
*/

const char* wchar2ansi(wchar_t* src)
{
    int len = wcslen(src) + 1;
    if(len == 1)
    {
        return NULL;
    }
    else
    {
        int nChars = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, src, len*sizeof(wchar_t), NULL, 0, NULL, NULL);
        if(nChars == 0) return NULL;
        char* cstr = (char *)malloc(nChars);
        strcpy(cstr, "");
        WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, src, len*sizeof(wchar_t), cstr, nChars, NULL, NULL);
        return (const char*)cstr;
    }
}

const wchar_t* ansi2wchar(char* src)
{
    int len = strlen(src) + 1;
    if(len == 1)
    {
        return NULL;
    }
    else
    {
        int nChars = MultiByteToWideChar(CP_ACP, 0, src, len*sizeof(char), NULL, 0); //MB_PRECOMPOSED
        if(nChars == 0) return NULL;
        wchar_t* cstr = (wchar_t *)malloc(sizeof(wchar_t) * nChars);
        wcscpy(cstr, _T(""));
        MultiByteToWideChar(CP_ACP, 0, src, len*sizeof(char), cstr, nChars);
        //cstr[nChars] = 0;
        //if(cstr[0] == 0xFEFF) // skip Oxfeff
        //    for(int i = 0; i < nChars; i ++)
        //        cstr[i] = cstr[i+1];
        return cstr;
    }
}

wchar_t* UTF8ToUTF16(const char* str)
{
    int textlen = 0;
    wchar_t * result = NULL;
    textlen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);

    result = new wchar_t[textlen + 1];
    memset(result, 0, (textlen + 1) * sizeof(wchar_t));

    MultiByteToWideChar(CP_UTF8, 0, str, -1, (LPWSTR)result, textlen);
    return  result;
}

wchar_t* ANSItoUTF16(const char* str)
{
    int textlen = 0;
    wchar_t * result = NULL;
    textlen = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);

    result = new wchar_t[textlen + 1];
    MultiByteToWideChar(CP_ACP, 0, str, -1, (LPWSTR)result, textlen);

    return result;
}

wstring MySystemTime2WStr(LCID loc, DWORD flags, const SYSTEMTIME *time, LPCWSTR format)
{
    std::vector<wchar_t> buffer;
    int numChars = GetTimeFormat(loc, flags, time, format, NULL, 0);
    if(numChars != 0)
    {
        buffer.resize(numChars);
        GetTimeFormat(loc, flags, time, format, &buffer[0], buffer.size()); // buffer.size()+1
        return &buffer[0];
    }
    return wstring(_T(""));
}

wstring MySystemDate2WStr(LCID loc, DWORD flags, const SYSTEMTIME *time, LPCWSTR format)
{
    std::vector<wchar_t> buffer;
    int numChars = GetDateFormat(loc, flags, time, format, NULL, 0);
    if(numChars != 0)
    {
        buffer.resize(numChars);
        GetDateFormat(loc, flags, time, format, &buffer[0], buffer.size()); // buffer.size()+1
        return &buffer[0];
    }
    return wstring(_T(""));
}

wstring MyDateAndTime2WStrShort()
{
    SYSTEMTIME lt;
    GetLocalTime(&lt);
    return MySystemDate2WStr(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &lt, NULL)+_T(" ")+
           MySystemTime2WStr(LOCALE_USER_DEFAULT, 0, &lt, NULL);
}

wstring MyGetFormatDateAndTimeSystemFormat(const SYSTEMTIME *date_time)
{
    return MySystemDate2WStr(LOCALE_USER_DEFAULT, DATE_SHORTDATE, date_time, NULL)+_T(" ")+
           MySystemTime2WStr(LOCALE_USER_DEFAULT, 0, date_time, NULL);
}

unsigned char PrTypSet2CBCurSel(unsigned short type)
{
    switch(type)
    {
    case LL_PROXY_HTTP:
        return 0;
    case LL_PROXY_HTTPCONNECT:
        return 1;
    case LL_PROXY_SOCKS4:
        return 2;
    case LL_PROXY_SOCKS5:
        return 3;
    case LL_PROXY_NONE:
        return 0;
    default:
        return 0;
    }
}

unsigned short CBCurSelPrTypSet(unsigned char cursel)
{
    switch(cursel)
    {
    case 0:
        return LL_PROXY_HTTP;
    case 1:
        return LL_PROXY_HTTPCONNECT;
    case 2:
        return LL_PROXY_SOCKS4;
    case 3:
        return LL_PROXY_SOCKS5;
    default:
        return LL_PROXY_NONE;
    }
}

void WinVer2WString(version_e v, wstring *vstr)
{
    switch(v)
    {
    case V_WINDOWS_95:
        vstr->append(_T("Windows 95"));
        break;
    case V_WINDOWS_98:
        vstr->append(_T("Windows 98"));
        break;
    case V_WINDOWS_ME:
        vstr->append(_T("Windows ME"));
        break;
    case V_WINDOWS_NT:
        vstr->append(_T("Windows NT 4.0"));
        break;
    case V_WINDOWS_2000:
        vstr->append(_T("Windows 2000"));
        break;
    case V_WINDOWS_XP:
        vstr->append(_T("Windows XP"));
        break;
    case V_WINDOWS_VISTA:
        vstr->append(_T("Windows Vista"));
        break;
    case V_WINDOWS_7:
        vstr->append(_T("Windows 7"));
        break;
    case V_WINDOWS_8:
        vstr->append(_T("Windows 8"));
        break;
    case V_WINDOWS_8_1:
        vstr->append(_T("Windows 8.1"));
        break;
    case V_WINDOWS_10:
        vstr->append(_T("Windows 10"));
        break;
    default:
        vstr->append(_T("Unknown"));
        break;
    }
}

void Arch2WString(architecture_e a, wstring *vstr)
{
    switch(a)
    {
    case A_WINDOWS_X86:
        vstr->append(_T("32-bit"));
        break;
    case A_WINDOWS_X86_64:
        vstr->append(_T("64-bit"));
        break;
    default:
        vstr->append(_T("Unknown"));
        break;
    }
}

void GetMySystemInfo(sys_info_t *si)
{
    si->version = V_UNKNOWN;
    si->arch    = A_UNKNOWN;

    HMODULE k = LoadLibrary(_T("Kernel32.dll"));
    //
    typedef void (WINAPI *SysInfo)(LPSYSTEM_INFO lpSystemInfo);
    SysInfo fn1 = (SysInfo)GetProcAddress(k, "GetNativeSystemInfo");
    if(!fn1) si->arch = A_WINDOWS_X86;
    if(fn1)
    {
        SYSTEM_INFO sys_info;
        fn1(&sys_info);
        switch(sys_info.wProcessorArchitecture)
        {
        case PROCESSOR_ARCHITECTURE_INTEL:
            si->arch = A_WINDOWS_X86;
            break;
        case PROCESSOR_ARCHITECTURE_AMD64:
            si->arch = A_WINDOWS_X86_64;
            break;
        default:
            si->arch = A_UNKNOWN;
        }
    }
    //
    bool isunicode = false;
    typedef int (WINAPI *getVersion)(OSVERSIONINFOEXA*  lpVersionInfo);
    getVersion fn = (getVersion)GetProcAddress(k, "GetVersionEx");
    if(!fn)
    {
        fn = (getVersion)GetProcAddress(k, "GetVersionExA");
        //if(fn) GlobalConfig->MyAddLog(_T("GetVersionExA..."));
    }
    if(!fn)
    {
        fn = (getVersion)GetProcAddress(k, "GetVersionExW");
        if(fn)
        {
            isunicode = true;
            //GlobalConfig->MyAddLog(_T("GetVersionExW..."));
        }
    }
    if(!fn)
    {
        si->version = V_UNKNOWN;
        FreeLibrary(k);
        return;
    }

    int M, m, p;
    M=m=p=0;

    if(!isunicode)
    {
        OSVERSIONINFOEXA verInfo;
        memset(&verInfo, 0, sizeof(OSVERSIONINFOEXA));
        verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
        fn(&verInfo);
        M = verInfo.dwMajorVersion;
        m = verInfo.dwMinorVersion;
        p = verInfo.dwPlatformId;
        //GlobalConfig->MyAddLog(_T("GetVersionEx..."));
    }

    si->dwMajorVersion = M;
    si->dwMinorVersion = m;
    si->dwPlatformId   = p;

    if(M == 4 && m == 0 && p != 2)
        si->version = V_WINDOWS_95;
    else if(M == 4 && m == 10)
        si->version = V_WINDOWS_98;
    else if(M == 4 && m == 90)
        si->version = V_WINDOWS_ME;
    else if(M == 4 && p == 2)
        si->version = V_WINDOWS_NT;
    else if(M == 5 && m == 0)
        si->version = V_WINDOWS_2000;
    else if(M == 5 && (m == 1 || m == 2))
        si->version = V_WINDOWS_XP;
    else if(M == 6 && m == 0)
        si->version = V_WINDOWS_VISTA;
    else if(M == 6 && m == 1)
        si->version = V_WINDOWS_7;
    else if(M == 6 && m == 2)
        si->version = V_WINDOWS_8;
    else if(M == 6 && m == 3)
        si->version = V_WINDOWS_8_1;
    else if(M == 10 && m == 0)
        si->version = V_WINDOWS_10;
    else
        si->version = V_UNKNOWN;
    //
    FreeLibrary(k);
}

// DIALOG UTILS
HWND Dialog_GetItemHWND(HWND dhwnd, UINT id)
{
    return GetDlgItem(dhwnd, id);
}

int Dialog_GetItemTextLen(HWND dhwnd, UINT id)
{
    return GetWindowTextLength(GetDlgItem(dhwnd, id));
}

wstring Dialog_GetItemText(HWND dhwnd, UINT id)
{
    wchar_t *text = new wchar_t[Dialog_GetItemTextLen(dhwnd, id) + 1];
    GetDlgItemText(dhwnd, id, text, Dialog_GetItemTextLen(dhwnd, id) + 1);
    wstring str = text;
    delete[] text;
    return str;
}

void Dialog_GetItemText2(HWND dhwnd, UINT id, wchar_t* text, int len)
{
    GetDlgItemText(dhwnd, id, text, len);
}

void Dialog_GetItemText2ANSI(HWND dhwnd, UINT id, char* text, int len)
{
    if(!len) return;
    wchar_t *src = new wchar_t[len];
    GetDlgItemText(dhwnd, id, src, len);
    //strcpy(text, wchar2ansi(src));
    delete[] src;
}

bool DSetItemText(HWND dhwnd, LPCWSTR text, UINT id)
{
    if(!SetDlgItemText(dhwnd, id, text)) return false;
    return true;
}

int Dialog_GetItemCheck(HWND dhwnd, UINT id)
{
    return IsDlgButtonChecked(dhwnd, id);
}

bool Dialog_SetItemCheck(HWND dhwnd, UINT id, int check)
{
    if(!CheckDlgButton(dhwnd, id, check)) return false;
    return true;
}

int Dialog_GetItemInt(HWND dhwnd, UINT id)
{
    return GetDlgItemInt(dhwnd, id, 0, 0);
}

bool DSetItemInt(HWND dhwnd, int value, UINT id)
{
    if(!SetDlgItemInt(dhwnd, id, value, 0)) return false;
    return true;
}

bool Dialog_EnableHWND(HWND dhwnd, UINT id, bool en)
{
    return EnableWindow(Dialog_GetItemHWND(dhwnd, id), en);
}
//

wstring Bytes2WString(DWORD inBytes)
{
    double size = (double)inBytes;
    wchar_t const* unitStr = inBytes == 1 ? _T("байт") : _T("байтов");
    wchar_t buf[128];

    if(inBytes > 1000 * 1024*1024)
    {
        size   /= 1024*1024*1024;
        unitStr = _T("Гб");
    }
    else if(inBytes > 1000 * 1024)
    {
        size   /= 1024*1024;
        unitStr = _T("Мб");
    }
    else if(inBytes > 1000)
    {
        size   /= 1024;
        unitStr = _T("КБ");
    }
    else
    {
        swprintf(buf, _T("%zu %s"), inBytes, unitStr);
        return wstring(buf);
    }

    swprintf(buf, _T("%.1f %s"), size, unitStr);
    return wstring(buf);
}

//

void str_stripws(std::string &str)
{
    while(str.size())
    {
        if (str[0] == ' ' || str[0] == '\t' || str[0] == '\r' ||
                str[0] == '\n')
            str.erase(str.begin());
        else
            break;
    }

    for(;;)
    {
        int last = str.size();
        if (!last)
            break;
        if (str[last-1] == ' ' || str[last-1] == '\t' ||
                str[last-1] == '\r' || str[last-1] == '\n')
            str.erase(str.end()-1);
        else
            break;
    }
}

int default_port(std::string &protocol)
{
    if(protocol == "http" || protocol == "icy")
        return 80;

    return 0;
}

int parse_url(std::string stream_url, std::string &protocol, std::string &host, unsigned short &port, std::string &path)
{
    str_stripws(stream_url);
    //if (str_in(stream_url, " "))
    //    return ERROR_INVALIDURL;
    if(stream_url.size() > 255*2)
        return ERROR_PARSEURL_INVALIDURL;

    /* protocol */
    size_t npos = stream_url.find("://");
    if(npos != stream_url.npos)
    {
        protocol = stream_url.substr(0, npos);
        stream_url.erase(stream_url.begin(), stream_url.begin() + npos + 3);
    }
    else
        return ERROR_PARSEURL_INVALIDURL;

    /* host / port / path */
    port = default_port(protocol);

    npos = stream_url.find(":");
    size_t npos2 = stream_url.find("/");
    if(npos2 == stream_url.npos)
        npos2 = 0x7fffff;
    if(npos != stream_url.npos && npos < npos2)
    {
        host = stream_url.substr(0, npos);
        stream_url.erase(stream_url.begin(), stream_url.begin() + npos + 1);

        port = atoi(stream_url.c_str());
        signed char delete_chars = 0;
        if (port <= 9) return ERROR_PARSEURL_INVALIDURL;
        else if (port >= 10 && port <= 99) delete_chars = 2;
        else if (port >= 100 && port <= 999) delete_chars = 3;
        else if (port >= 1000 && port <= 9999) delete_chars = 4;
        else if (port >= 10000) delete_chars = 5;
        stream_url.erase(stream_url.begin(), stream_url.begin() + delete_chars);
    }
    else
    {
        npos = stream_url.find("/");

        if(npos != stream_url.npos)
        {
            host = stream_url.substr(0, npos);
            stream_url.erase(stream_url.begin(), stream_url.begin() + npos);
        }
        else
        {
            host = stream_url;
            stream_url.clear();
        }
    }

    path.append(stream_url);
    if (path.empty())
        path = "/";

    return 1;
}

bool str_in(std::string &haystack, const char *needle)
{
    return haystack.find(needle) != haystack.npos;
}

void str_replace(std::string &s, const std::string &to_find, const std::string& repl_with)
{
    size_t pos;
    while ((pos = s.find(to_find)) != std::string::npos)
        s.replace(pos, to_find.size(), repl_with);
}

std::string str_lowercase(std::string str)
{
    std::string temp;
    for (size_t s = 0; s < str.size(); s++)
        temp += tolower(str[s]);
    return temp;
}

static int separator(TCHAR x)
{
    if (!x || x==_T(' ')) return 1;
    if (x==_T('\'') || x==_T('_')) return 0;
    return !iswalnum(x);
}

void my_unicode_caps(wstring &str)
{
    int len = str.length() + 1;
    if(len > 1)
    {
        uint16_t *z1;

        len = sizeof(wchar_t) * len;
        z1 = (uint16_t*)malloc(len);
        memcpy(z1, (uint16_t*)str.c_str(), len);
        int sep = 1;
        str.clear();
        while(1)
        {
            TCHAR c=*(z1++);
            if(c == _T('\0')) break;
            int s = separator(c);
            if(!s && sep)
                c=unicode_upper(c);
            else if(!sep) c=unicode_lower(c);
            sep=s;
            str.push_back(c);
        }
        free(z1);
    }
}

//
int MyMessageBox(HWND hwnd, wstring text, unsigned int type)
{
    UINT flag = MB_APPLMODAL | MB_SETFOREGROUND | MB_OK;
    wstring capt;
    switch(type)
    {
    case MY_MES_ERROR:
        flag = flag | MB_ICONERROR;
        capt = _T("Ошибка");
        break;
    case MY_MES_QUESTION:
        flag = flag | MB_ICONQUESTION;
        capt = _T("Вопрос");
        break;
    case MY_MES_WARNING:
        flag = flag | MB_ICONWARNING;
        capt = _T("Предупреждение");
        break;
    case MY_MES_INFORMATION:
        flag = flag | MB_ICONINFORMATION;
        capt = _T("Информация");
        break;
    case MY_MES_STOP:
        flag = flag | MB_ICONSTOP;
        capt = _T("Стоп");
        break;
    }
    return MessageBox(hwnd, text.c_str(), capt.c_str(), flag);
}

bool CenterWindow(HWND hwnd)
{
    HWND hwndParent;
    RECT rect, rectP;
    int width, height;
    int screenwidth, screenheight;
    int x, y;

    hwndParent = GetParent(hwnd);

    GetWindowRect(hwnd, &rect);
    GetWindowRect(hwndParent, &rectP);

    width  = rect.right  - rect.left;
    height = rect.bottom - rect.top;

    x = ((rectP.right-rectP.left) -  width) / 2 + rectP.left;
    y = ((rectP.bottom-rectP.top) - height) / 2 + rectP.top;

    screenwidth  = GetSystemMetrics(SM_CXSCREEN);
    screenheight = GetSystemMetrics(SM_CYSCREEN);

    if(x < 0) x = 0;
    if(y < 0) y = 0;
    if(x + width  > screenwidth)  x = screenwidth  - width;
    if(y + height > screenheight) y = screenheight - height;

    MoveWindow(hwnd, x, y, width, height, FALSE);
    return true;
}

typedef HRESULT (WINAPI * ENABLETHEMEDIALOGTEXTURE)(HWND, DWORD);

HRESULT WINAPI MyEnableThemeDialogTexture(HWND hWnd, DWORD dwFlags)
{
    ENABLETHEMEDIALOGTEXTURE pfnETDT;
    HINSTANCE                hDll;
    HRESULT                  hr;

    hr = HRESULT_FROM_WIN32(ERROR_CALL_NOT_IMPLEMENTED);

    if(NULL != (hDll = LoadLibrary(_T("uxtheme.dll"))))
    {
        if(NULL != (pfnETDT = (ENABLETHEMEDIALOGTEXTURE)GetProcAddress(hDll, "EnableThemeDialogTexture")))
        {
            hr = pfnETDT(hWnd, dwFlags);
        }
        FreeLibrary(hDll);
    }

    return(hr);
}
