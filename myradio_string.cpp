#include "myhdr.h"
#include "all_type.h"

MyString::MyString()
{
    str = _T("");
    init();
}

MyString::MyString(const char *text)
{
    int len = strlen(text) + 1;
    if (len == 1)
    {
        str = _T("");
    }
    else
    {
        wchar_t *buffer = (wchar_t *)malloc(sizeof(wchar_t) * len);
        wcscpy(buffer, _T(""));
        ANSITowchar(buffer, text, len);
        str = wstring(buffer);
        free(buffer);
    }
    init();
}

MyString::MyString(const id3_utf8_t *text)
{
    int len = strlen((const char *)text) + 1;
    if (len == 1)
    {
        str = _T("");
    }
    else
    {
        wchar_t *buffer = (wchar_t *)malloc(sizeof(wchar_t) * len);
        wcscpy(buffer, _T(""));
        UTF8Towchar(buffer, (const char *)text, len);
        str = wstring(buffer);
        free(buffer);
    }
    init();
}

MyString::MyString(const wchar_t *text)
{
    str = wstring(text);
    init();
}

MyString::MyString(const std::string& text)
{
    int len = text.length() + 1;
    if (len == 1)
    {
        str = _T("");
    }
    else
    {
        wchar_t *buffer = (wchar_t *)malloc(sizeof(wchar_t) * len);
        wcscpy(buffer, _T(""));
        ANSITowchar(buffer, text.c_str(), len);
        str = wstring(buffer);
        free(buffer);
    }
    init();
}

MyString::MyString(const wstring& text)
{
    str = text;
    init();
}

MyString::MyString(const MyString& text)
{
    str = text.wstring_str();
    init();
}

MyString::~MyString()
{
    clean();
}

void MyString::SetANSI_Text(char *val)
{
    clean();
    int len = strlen(val) + 1;
    if (len == 1)
    {
        str = _T("");
    }
    else
    {
        wchar_t *buffer = (wchar_t *)malloc(sizeof(wchar_t) * len);
        wcscpy(buffer, _T(""));
        ANSITowchar(buffer, val, len);
        str = wstring(buffer);
        free(buffer);
    }
    init();
}

bool MyString::wcharToUTF8(char *dest, const wchar_t *src, size_t max)
{
    if (WideCharToMultiByte(CP_UTF8, 0, src, -1, (LPSTR)dest, max, NULL, NULL) == 0)
    {
        return false;
    }

    return true;
}

bool MyString::UTF8Towchar(wchar_t *dest, const char *src, size_t max)
{
    if (MultiByteToWideChar(CP_UTF8, 0, (LPSTR)src, -1, dest, max) == 0)
    {
        return false;
    }

    return true;
}

bool MyString::wcharToANSI(char *dest, const wchar_t *src, size_t max)
{
    if(WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, src, -1, (LPSTR)dest, max, NULL, NULL) == 0)
    {
        return FALSE;
    }

    return TRUE;
}

bool MyString::ANSITowchar(wchar_t *dest, const char *src, size_t max)
{
    if(MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPSTR)src, -1, dest, max) == 0)
    {
        return false;
    }

    return true;
}

void MyString::init()
{
    cstr = NULL;
    c_str_size = 0;

    cstr_v2 = NULL;

    utf8 = NULL;
    utf8_str_size = 0;
}

void MyString::clean()
{
    if(cstr != NULL)
    {
        free(cstr);
        cstr = NULL;
        c_str_size = 0;
    }

    if(cstr_v2 != NULL)
    {
        free(cstr_v2);
        cstr_v2 = NULL;
    }

    if(utf8 != NULL)
    {
        free(utf8);
        utf8 = NULL;
        utf8_str_size = 0;
    }
}

MyString& MyString::operator =(const MyString& rhs)
{
    if (this == &rhs) //self-assignment
    {
        return *this;
    }

    str = rhs.str;
    clean();
    return *this;
}

MyString& MyString::operator +=(const MyString& rhs)
{
    str += rhs.str;
    clean();
    return *this;
}

MyString& MyString::operator +(const MyString& rhs)
{
    str += rhs.str;
    clean();
    return *this;
}

const bool MyString::operator ==(const MyString& rhs)
{
    if (str == rhs.str)
        return true;

    return false;
}

const bool MyString::operator !=(const MyString& rhs)
{
    return !(*this == rhs);
}

wchar_t& MyString::operator [](unsigned int i)
{
    return str[i];
}

const char* MyString::c_str()
{
    if(cstr == NULL)
    {
        int nChars = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, str.c_str(), str.size(), NULL, 0, NULL, NULL);
        if(nChars == 0) return NULL;
        cstr = (char *)malloc(nChars);
        strcpy(cstr, "");
        WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, str.c_str(), str.size(), cstr, nChars, NULL, NULL);
        c_str_size = nChars;
    }
    return (const char*)cstr;
}

const char* MyString::c_str_v2()
{
    if(cstr_v2 == NULL)
    {
        int len = str.length() + 1;
        cstr_v2 = (char *)malloc(len);
        strcpy(cstr_v2, "");
        wcharToANSI(cstr_v2, str.c_str(), len);
    }
    return (const char*)cstr_v2;
}

const wchar_t* MyString::w_str()
{
    return str.c_str();
}

//void MyString::Clean()
//{
//    clean();
//}

const id3_utf8_t* MyString::utf8_str()
{
    if(utf8 == NULL)
    {
        int nChars = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.size(), NULL, 0, NULL, NULL);
        if(nChars == 0) return NULL;
        utf8 = (id3_utf8_t *)malloc(nChars);
        strcpy((char *)utf8, "");
        WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.size(), (char *)(utf8), nChars, NULL, NULL);
        utf8_str_size = nChars;
    }
    return (const id3_utf8_t*)utf8;
}

const wstring& MyString::wstring_str() const
{
    return str;
}

size_t MyString::length()
{
    return str.length();
}

size_t MyString::size()
{
    return str.size();
}

bool MyString::str_empty()
{
    return str.empty();
}

wchar_t& MyString::my_front()
{
    return str.front();
}

void MyString::wstring_clear()
{
    str.clear();
}

int MyString::Get_utf8_str_size()
{
    return utf8_str_size;
}

int MyString::Get_c_str_size()
{
    return c_str_size;
}
