#ifndef MYRADIO_STRING_H_INCLUDED
#define MYRADIO_STRING_H_INCLUDED

/* myradio_string.cpp */
//typedef unsigned long id3_ucs4_t;
//typedef unsigned char id3_latin1_t;
typedef unsigned short id3_utf16_t;
typedef signed char id3_utf8_t;

class MyString
{
public:
    MyString();
    MyString(const char *text);
    MyString(const id3_utf8_t *text);
    MyString(const wchar_t *text);
    MyString(const std::string& text);
    MyString(const wstring& text);
    MyString(const MyString& text);
    //MyString(const id3_latin1_t *text);
    //MyString(const id3_ucs4_t *text);

    ~MyString();

    MyString& operator =(const MyString& rhs);
    MyString& operator +=(const MyString& rhs);
    MyString& operator +(const MyString& rhs);
    const bool operator ==(const MyString& rhs);
    const bool operator !=(const MyString& rhs);
    wchar_t& operator [](unsigned i);

    const char* c_str();
    const wchar_t* w_str();
    const id3_utf8_t* utf8_str();
    const wstring& wstring_str() const;

    const char* c_str_v2();

    int Get_utf8_str_size();
    int Get_c_str_size();

    void SetANSI_Text(char *val);

    size_t length();
    size_t size();
    bool str_empty();
    void wstring_clear();
    wchar_t& my_front();
    //void Clean();

    static bool wcharToUTF8(char *dest, const wchar_t *src, size_t max);
    static bool UTF8Towchar(wchar_t *dest, const char *src, size_t max);
    static bool wcharToANSI(char *dest, const wchar_t *src, size_t max);
    static bool ANSITowchar(wchar_t *dest, const char *src, size_t max);

protected:

    wstring str;
    id3_utf8_t *utf8;
    char *cstr;
    char *cstr_v2;

    int utf8_str_size;
    int c_str_size;

    void init();
    void clean();
};

#endif // MYRADIO_STRING_H_INCLUDED
