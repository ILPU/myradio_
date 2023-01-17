#ifndef MY_GLOBAL_UTILS_H_INCLUDED
#define MY_GLOBAL_UTILS_H_INCLUDED

#define TESTMESS(v) MessageBox(0, v, NULL, 0);
#define TESTMESS1() TESTMESS(_T("TEST"));

#define ERROR_PARSEURL_INVALIDURL       100001

const char* wchar2ansi(wchar_t* src);
const wchar_t* ansi2wchar(char* src);
wchar_t* UTF8ToUTF16(const char* str);
wchar_t* ANSItoUTF16(const char* str);

int parse_url(std::string stream_url, std::string &protocol, std::string &host, unsigned short &port, std::string &path);
bool str_in(std::string &haystack, const char *needle);
std::string str_lowercase(std::string str);
void str_replace(std::string &s, const std::string &to_find, const std::string& repl_with);

#endif // MY_GLOBAL_UTILS_H_INCLUDED
