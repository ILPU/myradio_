#ifndef MYSOCK_PROXY_H_INCLUDED
#define MYSOCK_PROXY_H_INCLUDED

#define LL_PROXY_NONE        10000
#define LL_PROXY_SOCKS4      10001
#define LL_PROXY_SOCKS5      10002
#define LL_PROXY_HTTPCONNECT 10003
#define LL_PROXY_HTTP        10004 /* will only work for HTTP */

#include "../myradio_string.h"

/* */

class InitWinSock
{
public:
    InitWinSock();
    ~InitWinSock();
};

/* Proxy Sock */
class SockProxy
{
public:
    SockProxy();
    ~SockProxy();

    void SetProxy(short type, char *ip, unsigned short port, char *username, char *password);

    int SockOpen(char *host, unsigned short port);
    int SockClose(int sock, bool reconnect = false);

    int SockRecv(int sock, char *buf, int len);
    int SockRecvLine(int sock, char *buf, int len);
    int SockSend(int &sock, const char *buf, int len);

    int PreFileDownload(MyString &url);
    bool StartFileDownload(int &vsock, int &error);
    void StopFileDownload(int &vsock);

    int GetFileDownloadSize();
    bool GetDatafromHttpHeader(MyString &data, char *val);

    void SetProxyfromConfig();

private:
    std::map<std::string,std::string> http_header;
    std::string stream_url, stream_protocol,
        stream_hostname, stream_filepath;
    unsigned short stream_port;

    char proxy_ip[32];
    unsigned short proxy_port;
    char proxy_username[128];
    char proxy_password[128];
    short proxy_type;

    struct proxy_state_t
    {
        long counter;
        char A1_saved[512];
        char host[1024+512];
        unsigned int port;

        char recv_buffer[512+1];
        int recv_buffer_len;
    };
    proxy_state_t proxy_state;

    int SocketReturnError(int val);

    struct ll_socket_t
    {
        int fd;
        unsigned char type;
    };

    void simple_base64(unsigned char *data, int len, char *out);
    void simple_md5(unsigned char *data, int len, unsigned char *digest);
    void simple_md5_hexdigest(unsigned char *data, int len, char *hex_digest);

    void _llint_init();
    long _llint_resolve(char *hostname);
    sockaddr_in _llint_sockaddr_in(char *ip_or_host, unsigned short port);
    int ll_tcp_socket();
    int ll_udp_socket();
    int ll_send2(int sock, const char *buf, int len);
    int ll_recv(int sock, char *buf, int len);
    int _llint_httpauth(char *authenticate, const char *method, char *path, char *request);

};

#endif // MYSOCK_PROXY_H_INCLUDED
