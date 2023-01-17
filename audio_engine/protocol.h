#ifndef PROTOCOL_H_INCLUDED
#define PROTOCOL_H_INCLUDED

class protocol : public chain
{
public:
    protocol()
    {
        failure = false;
        end_reached = false;
        reset_everything = false;
    }

    virtual ~protocol()
    {
    }

    /*
     *
     */
    virtual void init() = 0;

    /*
     *
     */
    virtual void handle() = 0;

    /*
     *
     */
    bool failed()
    {
        return failure;
    }

    bool eof()
    {
        return end_reached;
    }

    bool reset()
    {
        return reset_everything;
    }
protected:
    /*
     * NOTE: define in input, not in a derived protocol! example:
     * input_http derives from http.
     * http derives from protocol.
     * input_http defines send_command
     */
    virtual void send_command(std::string cmd)
    {
    }

    bool failure;
    bool end_reached;
    bool reset_everything;
};

#include "protocol_http.h"
#include "../net/mysock_proxy.h"

enum input_type
{
    INPUT_NETWORK,
    INPUT_LOCAL
};

struct input_info
{
    input_type type;
    std::string hostname;
    std::string filepath;
    unsigned short port;
    input_info()
    {
        type = INPUT_NETWORK;
    }
};

class input : public chain
{
public:
    input()
    {
        connection_failed = false;
        end_reached = false;
        reset_everything = false;

        SockProxy *prs = new SockProxy();
        prs->SetProxyfromConfig();
    }

    virtual ~input()
    {
        delete prs;
    }

    /*
     * read and write.
     */
    virtual void tick()
    {
        //handler.select();
    }

    /*
     * open
     */
    virtual int open(input_info &details) = 0;

    /*
     * close
     */
    virtual int close() = 0;

    format get_format()
    {
        return format_type;
    }

    bool failed()
    {
        return connection_failed;
    }

    bool eof()
    {
        return end_reached;
    }

    bool reset(bool r=true)
    {
        if(!r)
            reset_everything = !reset_everything;
        return reset_everything;
    }

protected:

    SockProxy *prs;
    format format_type;
    input_info input_details;
    protocol_type proto_type;
    bool connection_failed;
    bool end_reached;
    bool reset_everything;

private:
};

class input_http : public http, public input
{
public:
    int sock;
    stream recieved;

    input_http()
    {
        //http::http();
        input::connection_failed = false;
        //main_sock = NULL;
        //old_sock = NULL;
    }
    ~input_http()
    {
        //if (main_sock)
        //{
        //    delete main_sock;
        //    main_sock = NULL;
        //}
        //if (old_sock)
        //{
        //    delete old_sock;
        //    old_sock = NULL;
        //}
    }
    input_info details_backup;
    int open(input_info &details)
    {
        //main_sock->recv()->clear();
        /*if (main_sock)
        {
            old_sock = main_sock;
            main_sock = new_tcp_socket();
        }
        else
        {
            main_sock = new_tcp_socket();
        }*/

        details_backup = details;
        input::connection_failed = false;

        /* */
        if (str_in(details.filepath, " "))
        {
            str_replace(details.filepath, " ", "%20");
        }
        /* */

        memset(temp_hostname, 0, 1023);
        memset(temp_filepath, 0, 1023);
        sprintf(temp_hostname, "%s", details.hostname.c_str());
        sprintf(temp_filepath, "%s", details.filepath.c_str());
        http::hostname = temp_hostname;
        http::filepath = temp_filepath;
        http::port = details.port;

        sock = prs->SockOpen(temp_hostname, details.port);

        if(sock < 0)
        {
            close();
            return sock;
        }

        http::set_input(&recieved);
        http::set_output(input::stream_out);
        http::set_common(input::common_data);
        http::init();

        return 1;
    }
    void tick()
    {
        input::tick();
        //http::set_recv_len(main_sock->recv_len);
        //input::connection_failed = http::connection_failed;

        /**/
        char buf[1024+1];

        int recvd = prs->SockRecv(sock, buf, 1024);
        if (recvd > 0)
            recieved.push((byte *)buf, recvd);
        /**/

        if (http::failed())
            connection_failed = true;
        input::format_type = http::format_type;

        if (recvd < 0)
        {
            input::connection_failed = true;
        }

        if (recvd == 0)
        {
            /* flush.
            TODO: This is pretty ugly. */
            unsigned int recv_size = recieved.size();
            byte *dead_data = (byte *)malloc(recv_size + 1);
            recieved.pop(dead_data, recv_size);
            dead_data[recv_size] = 0;
            //LOG_DEBUG("from dead: %s", (char *)dead_data);
            input::stream_out->push(dead_data, recv_size);
            free(dead_data);

            //input::connection_failed = true;
            input::end_reached = true;
        }

        http::handle();
    }
    int close()
    {
        //main_sock->disconnect();
        //if (old_sock)
        //    old_sock->disconnect();
        prs->StopFileDownload(sock);
        return 0;
    }
private:
    void send_command(std::string cmd)
    {
        //main_sock->send(cmd);
        prs->SockSend(sock, cmd.c_str(), cmd.size());
    }

    char temp_hostname[1024];
    char temp_filepath[1024];

    bool connection_failed;

    //tcp_socket *main_sock;
    //tcp_socket *old_sock;
};

#endif // PROTOCOL_H_INCLUDED
