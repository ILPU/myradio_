#include "../myhdr.h"
#include "../myradio.h"
#include <winsock2.h>
#include <cmath>
#include <stdlib.h>
#include <time.h>
#include "mysock_proxy.h"

#define SETAR(x, type) (*((type *)(&(x))))
#define HTTP_STATUS_CODE(header) ((header[9]-'0')*100 + (header[10]-'0')*10 + (header[11]-'0'))


InitWinSock::InitWinSock()
{
    WSADATA wsadata;
    WSAStartup(0x101, &wsadata);
    srand(time(NULL));
}

InitWinSock::~InitWinSock()
{
    WSACleanup();
}


SockProxy::SockProxy()
{
    proxy_type = LL_PROXY_NONE;
}

SockProxy::~SockProxy()
{
//
}

/*
 * simple_base64
 *
 * Compact base64 encoding implementation for short data.
 */

void SockProxy::simple_base64(unsigned char *data, int len, char *out)
{
    char encmap[] = "ABCDEFGHIJKLMNOPQRSTUV"
                    "WXYZabcdefghijklmnopqr"
                    "stuvwxyz0123456789+/=";
    int oc = 0;
    for (int d = 0; d < len; d += 3)
    {
        int bits = data[d+0] << 16;
        if ((d + 1) < len)
            bits |= (data[d+1] << 8);
        if ((d + 2) < len)
            bits |= (data[d+2]);

        out[oc++] = encmap[(bits >> 18) & 63];
        out[oc++] = encmap[(bits >> 12) & 63];
        out[oc++] = encmap[(bits >> 6)  & 63];
        out[oc++] = encmap[(bits)       & 63];
    }
    if (len % 3 == 1)
        out[oc-2] = out[oc-1] = '=';
    if (len % 3 == 2)
        out[oc-1] = '=';
    out[oc] = 0;
}

/*
 * simple_md5
 *
 * Compact MD5 implementation for short data.
 */

void SockProxy::simple_md5(unsigned char *data, int len, unsigned char *digest)
{
#define ROTLEFT(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))
    int i, origlen = len;
    unsigned k[64], h[] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476};
    unsigned r[] = {7, 12, 17, 22, 5, 9, 14, 20, 4, 11, 16, 23, 6, 10, 15, 21};
    for (i = 0; i < 64; i++)
    {
        //k[i] = floor(abs(sin((double)(i + 1))) * (double)0x100000000LL);
        double ds = sin((double)(i + 1));
        if (ds < 0) ds = -ds;
        k[i] = floor(ds * (double)0x100000000LL);
    }

    unsigned char *msg = (unsigned char *)malloc(len + 64);
    memcpy(msg, data, len);

    /* padding */
    msg[len++] = 0x80;
    while (len % 64 != 56)
        msg[len++] = 0;
    *((unsigned long long*)(msg+len)) = origlen * 8; // XXX
    len += 8;

    /* transform */
    for (int part = 0; part < len; part += 64)
    {
        unsigned int *w = (unsigned int *)(msg+part);
        unsigned int a=h[0], b=h[1], c=h[2], d=h[3], f, g, temp;
        for (i = 0; i < 64; i++)
        {
            if (i < 16)
            {
                f = (b & c) | ((~b) & d);
                g = i;
            }
            else if (i < 32)
            {
                f = (d & b) | ((~d) & c);
                g = 5 * i + 1;
            }
            else if (i < 48)
            {
                f = b ^ c ^ d;
                g = 3 * i + 5;
            }
            else if (i < 64)
            {
                f = c ^ (b | (~d));
                g = 7 * i;
            }

            temp = d;
            d = c;
            c = b;
            b = ROTLEFT(a+f+k[i]+w[g%16], r[(i/16)*4+(i%16)%4]) + b;
            a = temp;
        }
        h[0]+=a;
        h[1]+=b;
        h[2]+=c;
        h[3]+=d;
    }
    //printf("%08x %08x %08x %08x\n", h[0], h[1], h[2], h[3]);
    for (i = 0; i < 4; i++)
    {
        digest[i*4+0] = h[i] & 0xff;
        digest[i*4+1] = (h[i]>>8) & 0xff;
        digest[i*4+2] = (h[i]>>16) & 0xff;
        digest[i*4+3] = (h[i]>>24) & 0xff;
    }

    free(msg);
}

void SockProxy::simple_md5_hexdigest(unsigned char *data, int len, char *hex_digest)
{
    unsigned char digest[32];
    hex_digest[0] = 0;
    simple_md5(data, len, digest);
    for (int d = 0; d < 16; d++)
    {
        char temp[16];
        sprintf(temp, "%02x", digest[d]);
        strcat(hex_digest, temp);
    }
}

void SockProxy::_llint_init()
{
    //
}

long SockProxy::_llint_resolve(char *hostname)
{
    hostent *he = gethostbyname(hostname);
    if (he == NULL)
        return -1;
    in_addr *addr = (in_addr *)he->h_addr;
    return addr->s_addr;
}

sockaddr_in SockProxy::_llint_sockaddr_in(char *ip_or_host, unsigned short port)
{
    sockaddr_in temp;
    temp.sin_family = AF_INET;
    temp.sin_port = htons(port);
    //temp.sin_addr.s_addr = inet_addr(ip);
    temp.sin_addr.s_addr = _llint_resolve(ip_or_host);
    memset(&(temp.sin_zero), 0, 8);
    return temp;
}

void SockProxy::SetProxy(short type, char *ip, unsigned short port, char *username, char *password)
{
    proxy_type = type;
    proxy_port = port;
    proxy_username[0] = 0;
    proxy_password[0] = 0;
    if(ip)
        strcpy(proxy_ip, ip);
    if(username)
        strcpy(proxy_username, username);
    if(password)
        strcpy(proxy_password, password);

    if(GlobalConfig->uselog)
    {
        wchar_t *wip = NULL;
        wchar_t *wuser = NULL;
        wchar_t *wpass = NULL;
        if(ip)
            wip = (wchar_t *)ansi2wchar(ip);
        if(username)
            wuser = (wchar_t *)ansi2wchar(username);
        if(password)
            wpass = (wchar_t *)ansi2wchar(password);

        GlobalConfig->MyAddLogParam(_T("Socket: proxy type = %d, url = %s, port = %d, auth on = %d, user = %s, pass = %s"),
                                    (int)proxy_type, (wip)?wip:_T(""), (int)port,
                                    (int)GlobalConfig->proxy_settings.proxy_athoriz_on,
                                    (wuser)?wuser:_T(""), (wpass)?wpass:_T(""));

        if(wip) free(wip);
        if(wuser) free(wuser);
        if(wpass) free(wpass);
    }

    proxy_state.host[0] = 0;
    proxy_state.port = 0;
    proxy_state.counter = 1;
    proxy_state.A1_saved[0] = 0;
    proxy_state.recv_buffer_len = 0;
    proxy_state.recv_buffer[0] = 0;
}

int SockProxy::ll_tcp_socket()
{
    _llint_init();
    return socket(PF_INET, SOCK_STREAM, 0);
}

int SockProxy::ll_udp_socket()
{
    _llint_init();
    return socket(PF_INET, SOCK_DGRAM, 0);
}

int SockProxy::SockClose(int sock, bool reconnect)
{
    if(GlobalConfig->uselog)
    {
        MyString tmp(_T("Socket: closesocket"));
        if(reconnect)
            tmp+=_T(" (407 error, reconnect to proxy)");
        GlobalConfig->MyAddLog(tmp);
    }
    if(sock > 0)
    {
        shutdown(sock, SD_BOTH);
        return closesocket(sock);
    }
    else return SOCKET_ERROR;
}

int SockProxy::ll_send2(int sock, const char *buf, int len)
{
    int ret, sent = 0;
    char *buf_ptr = (char*)buf; // XXX
    do
    {
        ret = send(sock, buf_ptr, len - sent, 0);
        if (ret < 0)
            return ret;
        sent += ret;
        buf_ptr += ret;
    }
    while(sent != len);
    return sent;
}

int SockProxy::ll_recv(int sock, char *buf, int len)
{
    int ret = 0, read = 0;
    while(len)
    {
        ret = recv(sock, &buf[read], len, 0);

        GlobalConfig->MyAddLogParam(_T("RET = %i; LEN = %i"), ret, len);

        if(ret == SOCKET_ERROR)
        {
            return 0;
        }
        if (ret == 0)
        {
            break;
        }
        read += ret;
        len -= ret;
    }
    return read;
}

int SockProxy::SockRecv(int sock, char *buf, int len)
{
    if(proxy_type == LL_PROXY_HTTP && proxy_state.recv_buffer_len != 0)
    {
        if(len >= proxy_state.recv_buffer_len)
        {
            char *tmpb = new char[len];
            memcpy(&tmpb[0], &proxy_state.recv_buffer[512-proxy_state.recv_buffer_len], proxy_state.recv_buffer_len);
            int ret = ll_recv(sock, &tmpb[proxy_state.recv_buffer_len], (len - proxy_state.recv_buffer_len));
            memcpy(buf, tmpb, proxy_state.recv_buffer_len + ret);
            delete[] tmpb;
            ret = ret + proxy_state.recv_buffer_len;
            proxy_state.recv_buffer_len = 0;
            proxy_state.recv_buffer[0] = 0;
            return ret;
        }
        else
        {
            memcpy(buf, &proxy_state.recv_buffer[512-proxy_state.recv_buffer_len], len);
            proxy_state.recv_buffer_len = proxy_state.recv_buffer_len - len;
            return len;
        }
    }

    return ll_recv(sock, buf, len);
}

int SockProxy::SockRecvLine(int sock, char *buf, int len)
{
    char *ptr = buf;
    DWORD cnt = 0;
    char buff[2];

    while(cnt < (DWORD)len-1)
    {
        if(SockRecv(sock, buff, 1) < 0) break;
        *ptr++ = buff[0];
        cnt++;
        if (buff[0] == 0 || buff[0] == '\n') break;
    }
    *ptr++ = 0;

    return cnt;
}

int SockProxy::_llint_httpauth(char *authenticate, const char *method, char *path, char *request)
{
    if(strstr(authenticate, "Basic"))
    {
        char basic_userpass[512];
        sprintf(basic_userpass, "%s:%s", proxy_username, proxy_password);

        char temp[512];
        //int templen = 512-1;
        //base64_encode((unsigned char *)temp, &templen,
        //    (unsigned char *)basic_userpass, strlen(basic_userpass));

        simple_base64((unsigned char *)basic_userpass, strlen(basic_userpass), temp);

        sprintf(request, "Basic %s", temp);

        GlobalConfig->MyAddLog(_T("Socket: HTTP proxy authenticate = Basic"));
        if(GlobalConfig->usefulllog)
        {
            MyString tmp((char *)request);
            GlobalConfig->MyAddLogParam(_T("Socket: Basic request = %s"), tmp.w_str());
        }
        return 0;
    }

    //char path[512];
    //sprintf(path, "%s:%d", host, port);

    /* parse authenticate string */
    char realm[128], qop[128], nonce[128], opaque[128], algorithm[128];
    realm[0] = qop[0] = nonce[0] = opaque[0] = algorithm[0] = 0;

    char *p;
    int i;

#define PARSE(str, field) {    \
        i = 0;                     \
        if((p = strstr(authenticate, str)) != NULL) \
        {                          \
            p += strlen(str) + 2;  \
            while (*p != '"')      \
                field[i++] = *p++; \
        }                          \
        field[i] = 0;              \
    }

    PARSE("realm", realm);
    PARSE("qop", qop);
    PARSE("nonce", nonce);
    PARSE("opaque", opaque);
    PARSE("algorithm", algorithm);

#undef PARSE

    /* create response */
    char cnonce[32];
    sprintf(cnonce, "%08x", 0xF6E3F947*rand()); // hex(int((sqrt(2)-1)*10000000000))
    char ncstring[16];
    sprintf(ncstring, "%08x", (unsigned int)proxy_state.counter);

    /*
    3.2.2.2 A1

    If the "algorithm" directive's value is "MD5" or is unspecified, then
    A1 is:
        A1       = unq(username-value) ":" unq(realm-value) ":" passwd
    where
        passwd   = < user's password >
    If the "algorithm" directive's value is "MD5-sess", then A1 is
    calculated only once - on the first request by the client following
    receipt of a WWW-Authenticate challenge from the server.  It uses the
    server nonce from that challenge, and the first client nonce value to
    construct A1 as follows:
        A1       = H( unq(username-value) ":" unq(realm-value)
                        ":" passwd )
                        ":" unq(nonce-value) ":" unq(cnonce-value)
    */

    char A1[512];
    sprintf(A1, "%s:%s:%s", proxy_username, realm, proxy_password);

    if(strstr(algorithm, "MD5-sess"))
    {
        if(strlen(proxy_state.A1_saved) == 0)
        {
            char A1_digest[64];
            simple_md5_hexdigest((unsigned char *)A1, strlen(A1), A1_digest);
            sprintf(A1, "%s:%s:%s", A1_digest, nonce, cnonce);

            sprintf(proxy_state.A1_saved, "%s", A1);
        }
        else
        {
            sprintf(A1, "%s", proxy_state.A1_saved);
        }
        GlobalConfig->MyAddLog(_T("Socket: HTTP proxy MD5-sess"));
    }

    /*
    3.2.2.3 A2

    If the "qop" directive's value is "auth" or is unspecified, then A2
    is:
        A2       = Method ":" digest-uri-value
    If the "qop" value is "auth-int", then A2 is:
        A2       = Method ":" digest-uri-value ":" H(entity-body)
    */

    char A2[512];

    sprintf(A2, "%s:%s", method, path);
    if(strstr(qop, "auth-int"))
    {
        // XXX: what is entity-body?
        //printf("auth-int not implemented");
        return -1;
    }

    GlobalConfig->MyAddLog(_T("Socket: HTTP proxy authenticate = Digest"));
    /*
    3.2.2.1 Request-Digest

    If the "qop" value is "auth" or "auth-int":

        request-digest  = <"> < KD ( H(A1),     unq(nonce-value)
                                            ":" nc-value
                                            ":" unq(cnonce-value)
                                            ":" unq(qop-value)
                                            ":" H(A2)
                                    ) <">

    If the "qop" directive is not present (this construction is for
    compatibility with RFC 2069):

        request-digest  =
                    <"> < KD ( H(A1), unq(nonce-value) ":" H(A2) ) >
    <">
    */


    /*
    KD(secret, data) = H(concat(secret, ":", data))
    */
    char kd_secret[512] = {0};
    char kd_data[512] = {0};
    simple_md5_hexdigest((unsigned char *)A1, strlen(A1), kd_secret);

    char A2_digest[64];
    simple_md5_hexdigest((unsigned char *)A2, strlen(A2), A2_digest);

    if (strlen(qop) == 0)
        sprintf(kd_data, "%s:%s", nonce, A2_digest);
    else
        sprintf(kd_data, "%s:%s:%s:%s:%s", nonce, ncstring, cnonce, qop, A2_digest);

    char resp_digest[1024];
    sprintf(resp_digest, "%s:%s", kd_secret, kd_data);

    char request_digest[64];
    simple_md5_hexdigest((unsigned char *)resp_digest, strlen(resp_digest), request_digest);

    /*
    ret.append("Digest ");
    RET_ADD("username", global_settings->httpproxy_username);
    RET_ADD("realm", realm);
    RET_ADD("nonce", nonce);
    RET_ADD("uri", path);
    RET_ADD_UNQ("algorithm", algorithm);
    RET_ADD("response", resp_digest);
    RET_ADD("opaque", opaque);
    RET_ADD_UNQ("qop", qop);
    if (!qop.empty())
    {
    RET_ADD_UNQ("nc", ncstring);
    RET_ADD("cnonce", cnonce);
    }
    ret.erase(ret.end()-1);
    ret.erase(ret.end()-1);
    */

    sprintf(request, "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", "
            "uri=\"%s\", algorithm=%s, response=\"%s\", opaque=\"%s\", qop=%s",
            proxy_username, realm, nonce, path, algorithm, request_digest, opaque, qop);
    if(strlen(qop) != 0)
    {
        /*
        cnonce
        This MUST be specified if a qop directive is sent (see above), and
        MUST NOT be specified if the server did not send a qop directive in
        the WWW-Authenticate header field.

        nonce-count
        This MUST be specified if a qop directive is sent (see above), and
        MUST NOT be specified if the server did not send a qop directive in
        the WWW-Authenticate header field.
        */
        char dump[512];
        sprintf(dump, ", nc=%s, cnonce=\"%s\"", ncstring, cnonce);
        strcat(request, dump);
    }

    //printf("\n\n[%s]\n\n", request);

    proxy_state.counter++;
    return 0;
}

int SockProxy::SockSend(int &sock, const char *buf, int len)
{
    if(proxy_type != LL_PROXY_HTTP)
        return ll_send2(sock, buf, len);

    // XXX: there are more methods than GET...
    //assert(len > 16 && buf[0] == 'G' && buf[1] == 'E' && buf[2] == 'T');

    char *body = NULL;
    //int body_len = len;
    char *buf_ptr = (char *)buf; // XXX

    /* decapitate */
    char header[1024];
    int hc = 0;
    for(int i = 0; i < len && i < 1024; i++)
    {
        if (buf[i  ] == '\r' && buf[i+1] == '\n' &&
                buf[i+2] == '\r' && buf[i+3] == '\n')
            break;

        if (buf[i] == '\n')
            header[hc++] = 0;
        else if (buf[i] != '\r')
            header[hc++] = buf[i];

        body = buf_ptr + i;
    }
    header[hc++] = 0;
    header[hc++] = 0;
    body += 5;

    //body_len = (buf + len) - body;

    //assert(body_len == 0); // XXX: fixme

    /* modify header */
    char header_updated[1024];
    header_updated[0] = 0;
    char *p = header;
    char file_path[256];
    file_path[0] = 0;
    for(;;)
    {
        if (*p == 0)
            break;

        if (p[0] == 'G' && p[1] == 'E' && p[2] == 'T')
        {
            char srow[256];
            sprintf(srow, "GET http://%s:%d", proxy_state.host, proxy_state.port);
            strcat(header_updated, srow);
            strcat(header_updated, p+4);

            sprintf(file_path, "%s", p+4);
            file_path[ strlen(file_path) - strlen(" HTTP/1.x") ] = 0;
        }
        else
            strcat(header_updated, p);

        strcat(header_updated, "\r\n");
        p += strlen(p) + 1;
    }
    strcat(header_updated, "\r\n");

    /* init */
    if(GlobalConfig->uselog)
    {
        GlobalConfig->MyAddLog(_T("Socket: HTTP proxy send header:"));
        GlobalConfig->MyAddLogHeader(header_updated, true);
    }
    ll_send2(sock, header_updated, strlen(header_updated));

    int ret;
    char text[512+1];

    ret = SockRecv(sock, text, 512);
    if(GlobalConfig->uselog)
    {
        GlobalConfig->MyAddLog(_T("Socket: HTTP proxy recv header:"));
        GlobalConfig->MyAddLogHeader(text, false);
    }
    if(ret < 0)
        return SocketReturnError(ERROR_SOCK_PROXY_BROKEN);
    text[ret] = 0;

    if(HTTP_STATUS_CODE(text) != 407)
    {
        /* cache data do be written back to the client on the next ll_recv().
        see ll_recv() for more info. */
        proxy_state.recv_buffer_len = ret;
        memcpy(proxy_state.recv_buffer, text, ret);
    }
    else if(HTTP_STATUS_CODE(text) == 407)
    {
        /* reconnect to proxy */
        sockaddr_in addr_proxy = _llint_sockaddr_in(proxy_ip, proxy_port);
        SockClose(sock, true);
        sock = ll_tcp_socket();
        if (connect(sock, (sockaddr *)&addr_proxy, sizeof(addr_proxy)) < 0)
            return SocketReturnError(ERROR_SOCK_PROXY_CONNFAILED);

        /* parse Proxy-Authenticate: string */
        char *authenticate = strstr(text, "uthenticate");
        if (authenticate == NULL)
            return SocketReturnError(ERROR_SOCK_PROXY_BROKEN);
        authenticate += 11;
        while (*authenticate == ' ' || *authenticate == ':')
            authenticate++;
        char *x = strchr(authenticate, '\r');
        if (x != NULL)
            *x = 0;

        /* */
        char path[512];
        sprintf(path, "http://%s:%d%s", proxy_state.host, proxy_state.port, file_path);
        char digest[1024] = {0};
        _llint_httpauth(authenticate, "GET", path, digest);
        header_updated[ strlen(header_updated) - 2] = 0; // XXX
        strcat(header_updated, "Proxy-Authorization: ");
        strcat(header_updated, digest);
        strcat(header_updated, "\r\n\r\n");

        //printf("header_updated:\n\n[%s]\n\n\n", header_updated);

        /* */
        if(ll_send2(sock, header_updated, strlen(header_updated)) < 0)
            return SocketReturnError(ERROR_SOCK_PROXY_BROKEN);
        GlobalConfig->MyAddLog(_T("Socket: HTTP (AUTH) proxy send header"));

        ret = SockRecv(sock, text, 512);
        if(GlobalConfig->uselog)
        {
            GlobalConfig->MyAddLog(_T("Socket: HTTP (AUTH) proxy recv header:"));
            GlobalConfig->MyAddLogHeader(text, false);
        }
        if(ret < 0)
        {
            SockClose(sock);
            return SocketReturnError(ERROR_SOCK_PROXY_BROKEN);
        }
        text[ret] = 0;
        if(HTTP_STATUS_CODE(text) == 407)
        {
            SockClose(sock);
            return SocketReturnError(ERROR_SOCK_PROXY_AUTHFAILED);
        }
        else
        {
            proxy_state.recv_buffer_len = ret;
            memcpy(proxy_state.recv_buffer, text, ret);
        }
    }

    return len;
}

void SockProxy::SetProxyfromConfig()
{
    GlobalConfig->MyAddLog(_T("SetProxyfromConfig >>"));
    if(!GlobalConfig->proxy_settings.proxy_on)
        SetProxy(LL_PROXY_NONE, NULL, 0, NULL, NULL);
    else
    {
        char* _ip = NULL;
        char* _user = NULL;
        char* _pass = NULL;
        _ip = NULL;
        _user = NULL;
        _pass = NULL;
        if(GlobalConfig->proxy_settings.proxy_athoriz_on)
        {
            _ip   = (char*)wchar2ansi(GlobalConfig->proxy_settings.proxy_ip);
            _user = (char*)wchar2ansi(GlobalConfig->proxy_settings.proxy_username);
            _pass = (char*)wchar2ansi(GlobalConfig->proxy_settings.proxy_password);
            SetProxy(GlobalConfig->proxy_settings.proxy_type, _ip, GlobalConfig->proxy_settings.proxy_port, _user, _pass);
        }
        else
        {
            _ip   = (char*)wchar2ansi(GlobalConfig->proxy_settings.proxy_ip);
            SetProxy(GlobalConfig->proxy_settings.proxy_type, _ip, GlobalConfig->proxy_settings.proxy_port, NULL, NULL);
        }
        if(_ip) free(_ip);
        if(_user) free(_user);
        if(_pass) free(_pass);
    }
    GlobalConfig->MyAddLog(_T("<< SetProxyfromConfig"));
}

int SockProxy::GetFileDownloadSize()
{
    if(!http_header["content-length"].empty())
        return atoi(http_header["content-length"].c_str());
    else return 0;
}

bool SockProxy::GetDatafromHttpHeader(MyString &data, char *val)
{
    if(!http_header[val].empty())
    {
        data.SetANSI_Text((char*)http_header[val].c_str());
        return true;
    }
    else
    {
        return false;
    }
}

int SockProxy::PreFileDownload(MyString &url)
{
    GlobalConfig->MyAddLog(_T("PreFileDownload >>"));
    stream_url.assign(url.c_str_v2());
    int ret = parse_url(stream_url, stream_protocol, stream_hostname, stream_port, stream_filepath);
    if(GlobalConfig->uselog)
    {
        wchar_t *wurl  = (wchar_t *)ansi2wchar((char *)stream_url.c_str());
        wchar_t *wprot = (wchar_t *)ansi2wchar((char *)stream_protocol.c_str());
        wchar_t *whost = (wchar_t *)ansi2wchar((char *)stream_hostname.c_str());
        wchar_t *wpath = (wchar_t *)ansi2wchar((char *)stream_filepath.c_str());
        GlobalConfig->MyAddLogParam(_T("Parse url: url = %s, protocol = %s, host = %s, port = %d, path = %s, error? = %d"),
                                    wurl, wprot, whost, stream_port, wpath, int(ret!=1));
        if(wurl) free(wurl);
        if(wprot) free(wprot);
        if(whost) free(whost);
        if(wpath) free(wpath);
    }
    GlobalConfig->MyAddLog(_T("<< PreFileDownload"));
    return ret;
}

bool SockProxy::StartFileDownload(int &vsock, int &error)
{
    GlobalConfig->MyAddLog(_T("StartFileDownload >>"));
    int ret = SockOpen((char*)stream_hostname.c_str(), stream_port);
    if(ret < 0)
    {
        GlobalConfig->MyAddLog(_T("SockOpen if(ret < 0)"));
        error = ret;
        vsock = SOCKET_ERROR;
        return false;
    }
    else
    {
        GlobalConfig->MyAddLog(_T("SockOpen if(ret > 0)"));
        char temp[1024];
        sprintf(temp,
                "GET %s HTTP/1.0\r\n"
                "Host: %s\r\n"
                "User-Agent: %s\r\n"
                "Accept: */*\r\n"
                "Connection: close\r\n\r\n",
                stream_filepath.c_str(), stream_hostname.c_str(),
                "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:40.0) Gecko/20100101 Firefox/40.1");
        // sprintf(temp,
        //         "GET / HTTP/1.0"
        //         "\r\n\r\n");
        if(GlobalConfig->uselog)
            GlobalConfig->MyAddLogHeader(temp, true);

        int send_byte = SockSend(ret, temp, strlen(temp));
        if(send_byte < 0)
        {
            error = send_byte;
            vsock = ret;
            return false;
        }

        /*char text[512];
        int ret2 = SockRecv(ret, text, 512-1);
        text[ret2] = 0;
        if(GlobalConfig->uselog)
        {
            GlobalConfig->MyAddLog(_T("TEST RECV HEADER:"));
            GlobalConfig->MyAddLogHeader(text, false);
            GlobalConfig->MyAddLogInt(ret2);
        }*/

        std::string header;
        char buffer[4080];
        int ret_byte = SockRecvLine(ret, buffer, 4080);
        GlobalConfig->MyAddLog(_T("SockOpen, SockRecvLine"));
        if(ret_byte > 0)
        {
            GlobalConfig->MyAddLogHeader(buffer, false);
            error = HTTP_STATUS_CODE(buffer);
            if(error == 200)
            {
                GlobalConfig->MyAddLog(_T("SockOpen (OK, code = 200)"));
                while(strcmp((const char *)buffer, "\r\n") != 0)
                {
                    header += buffer;
                    ret_byte = SockRecvLine(ret, buffer, 4080);
                    if(ret_byte < 1) break;
                }
                if(GlobalConfig->uselog)
                    GlobalConfig->MyAddLogHeader(header, false);

                http_header.clear();
                int header_len = 2;
                for(;;)
                {
                    size_t rpos = header.find("\r\n");
                    if(rpos != header.npos)
                    {
                        std::string row = header.substr(0, rpos);
                        if (row.empty())
                            break;
                        header_len += row.size() + 2;
                        size_t cpos = row.find(":");
                        if(cpos != header.npos)
                        {
                            if (row[cpos+1] == ' ')
                                http_header[str_lowercase(row.substr(0,cpos))] =
                                    row.substr(cpos+2, row.size());
                            else
                                http_header[str_lowercase(row.substr(0,cpos))] =
                                    row.substr(cpos+1, row.size());
                        }
                        header.erase(header.begin(), header.begin()+rpos+2);
                    }
                    else break;
                }
                header.clear();

                vsock = ret;
                error = 0;
                return true;

            }
            else
            {
                GlobalConfig->MyAddLog(_T("SockOpen (error != 200)"));
                if(GlobalConfig->uselog)
                    GlobalConfig->MyAddLogHeader(buffer, false);
                vsock = ret;
                return false;
            }
        }
        else
        {
            error = ret_byte;
            vsock = ret;
            return false;
        }
    }
    error = SOCKET_ERROR;
    vsock = SOCKET_ERROR;
    return false;
}

void SockProxy::StopFileDownload(int &vsock)
{
    SockClose(vsock);
    http_header.clear();
}

int SockProxy::SocketReturnError(int val)
{
    if(val < 0)
    {
        if(GlobalConfig->uselog)
        {
            switch(val)
            {
            case ERROR_SOCK_CONNFAILED:
                GlobalConfig->MyAddLog(_T("Socket: error = connect failed"));
                break;
            case ERROR_SOCK_PROXY_AUTHFAILED:
                GlobalConfig->MyAddLog(_T("Socket: error = authenticate failed"));
                break;
            case ERROR_SOCK_PROXY_CONNFAILED:
                GlobalConfig->MyAddLog(_T("Socket: error = proxy connect failed"));
                break;
            case ERROR_SOCK_PROXY_BROKEN:
                GlobalConfig->MyAddLog(_T("Socket: error = proxy broken"));
                break;
            default:
                GlobalConfig->MyAddLog(_T("Socket: error = unknown error"));
                break;
            }
        }
        return val;
    }
    return val;
}

int SockProxy::SockOpen(char *host, unsigned short port)
{
    GlobalConfig->MyAddLog(_T("SockOpen >>"));
    _llint_init();
    int sock, ret;

    sockaddr_in addr_host;
    if(proxy_type == LL_PROXY_NONE || proxy_type == LL_PROXY_SOCKS4 || proxy_type == LL_PROXY_SOCKS5)
        addr_host = _llint_sockaddr_in(host, port);

    sockaddr_in addr_proxy;
    if(proxy_type != LL_PROXY_NONE)
        addr_proxy = _llint_sockaddr_in(proxy_ip, proxy_port);

    if(proxy_type == LL_PROXY_NONE)
    {
        GlobalConfig->MyAddLog(_T("SockOpen proxy_type == LL_PROXY_NONE"));
        sock = ll_tcp_socket();
        if(connect(sock, (sockaddr *)&addr_host, sizeof(addr_host)) < 0)
        {
            SockClose(sock);
            GlobalConfig->MyAddLog(_T("SockOpen connect < 0"));
            return SocketReturnError(ERROR_SOCK_CONNFAILED);
        }
        return sock;
    }

    proxy_state.port = port;
    sprintf(proxy_state.host, "%s", host);
    GlobalConfig->MyAddLog(_T("SockOpen connect to proxy"));
    /* connect to proxy */
    sock = ll_tcp_socket();
    if(connect(sock, (sockaddr *)&addr_proxy, sizeof(addr_proxy)) < 0)
    {
        SockClose(sock);
        return SocketReturnError(ERROR_SOCK_PROXY_CONNFAILED);
    }

    if(proxy_type == LL_PROXY_HTTPCONNECT)
    {
        char text[512];

        /* */
        sprintf(text,
                "CONNECT %s:%d HTTP/1.1\r\n"
                "User-Agent: %s\r\n"
                "\r\n\r\n",
                host, port,
                "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:40.0) Gecko/20100101 Firefox/40.1");

        if(GlobalConfig->uselog)
        {
            GlobalConfig->MyAddLog(_T("Socket: HTTP CONNECT proxy send:"));
            GlobalConfig->MyAddLogHeader(text, true);
        }

        if(SockSend(sock, text, strlen(text)) < 0)
        {
            SockClose(sock);
            return SocketReturnError(ERROR_SOCK_PROXY_BROKEN);
        }
        /* */
        ret = SockRecv(sock, text, 512-1);
        if(ret < 0)
        {
            SockClose(sock);
            return SocketReturnError(ERROR_SOCK_PROXY_BROKEN);
        }
        text[ret] = 0;

        if(GlobalConfig->uselog)
        {
            GlobalConfig->MyAddLog(_T("Socket: HTTP CONNECT proxy recv header:"));
            GlobalConfig->MyAddLogHeader(text, false);
        }

        /* auth required */
        if(HTTP_STATUS_CODE(text) == 407)
        {
            /* reconnect to proxy */
            SockClose(sock, true);
            sock = ll_tcp_socket();
            if (connect(sock, (sockaddr *)&addr_proxy, sizeof(addr_proxy)) < 0)
            {
                SockClose(sock);
                return SocketReturnError(ERROR_SOCK_PROXY_CONNFAILED);
            }

            /* parse Proxy-Authenticate: string */
            char *authenticate = strstr(text, "uthenticate");
            if(authenticate == NULL)
            {
                SockClose(sock);
                return SocketReturnError(ERROR_SOCK_PROXY_BROKEN);
            }
            authenticate += 11;
            while(*authenticate == ' ' || *authenticate == ':')
                authenticate++;
            char *x = strchr(authenticate, '\r');
            if(x != NULL)
                *x = 0;

            /* */
            char path[512];
            sprintf(path, "%s:%d", host, port);

            char request[1024] = {0};
            //_llint_httpauth(authenticate, "CONNECT", host, port, NULL, request);
            _llint_httpauth(authenticate, "CONNECT", path, request);
            sprintf(text,
                    "CONNECT %s:%d HTTP/1.1\r\n"
                    "User-Agent: %s\r\n"
                    "Proxy-Authorization: %s\r\n"
                    "\r\n", host, port,
                    "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:40.0) Gecko/20100101 Firefox/40.1",
                    request);

            if(GlobalConfig->uselog)
            {
                GlobalConfig->MyAddLog(_T("Socket: HTTP CONNECT (AUTH) proxy send:"));
                GlobalConfig->MyAddLogHeader(text, true);
            }

            if(SockSend(sock, text, strlen(text)) < 0)
            {
                SockClose(sock);
                return SocketReturnError(ERROR_SOCK_PROXY_BROKEN);
            }

            ret = SockRecv(sock, text, 512-1);
            if(ret < 0)
            {
                SockClose(sock);
                return SocketReturnError(ERROR_SOCK_PROXY_BROKEN);
            }
            text[ret] = 0;

            if(GlobalConfig->uselog)
            {
                GlobalConfig->MyAddLog(_T("Socket: HTTP CONNECT (AUTH) proxy recv header:"));
                GlobalConfig->MyAddLogHeader(text, false);
            }

            if(HTTP_STATUS_CODE(text) == 407)
            {
                SockClose(sock);
                return SocketReturnError(ERROR_SOCK_PROXY_AUTHFAILED);
            }

            return sock;
        }
    }
    else if(proxy_type == LL_PROXY_SOCKS4)
    {
        unsigned char packet[256] = {0};
        size_t packet_len = 0;

        packet[0] = 4; // ver
        packet[1] = 1; // 1 = connect
        SETAR(packet[2], unsigned short) = addr_host.sin_port;
        SETAR(packet[4], long) = addr_host.sin_addr.s_addr;
        const char *temp_user = "DEFAULT";
        if(proxy_username[0])
            temp_user = proxy_username;
        for(unsigned int p = 0; p < strlen(temp_user); p++)
            packet[p+8] = temp_user[p];
        packet_len = 8 + strlen(temp_user) + 1;

        //ret = send(sock, (const char *)packet, packet_len, 0);
        if(SockSend(sock, (const char *)packet, packet_len) < 0)
        {
            SockClose(sock);
            return SocketReturnError(ERROR_SOCK_PROXY_BROKEN);
        }
        ret = SockRecv(sock, (char *)packet, 256-1);
        if(ret < 0)
        {
            SockClose(sock);
            return SocketReturnError(ERROR_SOCK_PROXY_BROKEN);
        }
        if(ret != 8 || packet[0] != 0 || packet[1] != 90)
        {
            SockClose(sock);
            return SocketReturnError(ERROR_SOCK_PROXY_CONNFAILED);
        }
    }
    else if(proxy_type == LL_PROXY_SOCKS5)
    {
        unsigned char packet[256] = {0};
        size_t packet_len = 0;

        // init
        packet[0] = 5; // ver
        packet[1] = 2; // num methods
        packet[2] = 0; // no auth
        packet[3] = 2; // username/pass
        //ret = send(sock, (const char *)packet, 4, 0);
        if(SockSend(sock, (const char *)packet, 4) < 0)
        {
            SockClose(sock);
            return SocketReturnError(ERROR_SOCK_PROXY_BROKEN);
        }
        ret = SockRecv(sock, (char *)packet, 256-1);
        if(ret != 2 || packet[0] != 5 || packet[1] == 0xff)
        {
            SockClose(sock);
            return SocketReturnError(-1);
        }
        int auth_method = packet[1];

        if(auth_method == 2) // username/pass
        {
            packet[0] = 1;
            packet[1] = strlen(proxy_username);
            packet_len = 2;
            for(unsigned int t = 0; t < strlen(proxy_username); t++)
                packet[packet_len++] = proxy_username[t];
            packet[packet_len++] = strlen(proxy_password);
            for(unsigned int t = 0; t < strlen(proxy_password); t++)
                packet[packet_len++] = proxy_password[t];

            //ret = send(sock, (const char *)packet, packet_len, 0);
            //if (ret != packet_len)
            if(SockSend(sock, (const char *)packet, packet_len) < 0)
            {
                SockClose(sock);
                return SocketReturnError(ERROR_SOCK_PROXY_BROKEN);
            }
            ret = SockRecv(sock, (char *)packet, 256-1);
            if(ret != 2 || packet[1])
            {
                SockClose(sock);
                return SocketReturnError(ERROR_SOCK_PROXY_AUTHFAILED);
            }
        }

        // conn
        packet[0] = 5;
        packet[1] = 1; // 1 = connect
        packet[2] = 0; // reserved
        packet[3] = 1; // 1 = ipv4
        SETAR(packet[4], long) = addr_host.sin_addr.s_addr;
        SETAR(packet[8], unsigned short) = addr_host.sin_port;

        //ret = send(sock, (const char *)packet, 10, 0);
        if(SockSend(sock, (const char *)packet, 10) < 0)
        {
            //Sock_Close(sock); ???
            return SocketReturnError(ret); // ret ???
        }
        ret = SockRecv(sock, (char *)packet, 256-1);
        if(ret < 10)
        {
            SockClose(sock);
            return SocketReturnError(ERROR_SOCK_PROXY_BROKEN);
        }
        if(packet[1] != 0) // 0 = success
        {
            SockClose(sock);
            return SocketReturnError(ERROR_SOCK_CONNFAILED);
        }
    }

    return sock;
}
