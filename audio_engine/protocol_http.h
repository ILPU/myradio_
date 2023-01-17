#ifndef PROTOCOL_HTTP_H_INCLUDED
#define PROTOCOL_HTTP_H_INCLUDED

class http : public protocol
{
public:
    http()
    {
        reset();
    }

    void reset()
    {
        meta_int          = 0;
        meta_int_chunk    = 1;
        has_header        = false;
        meta_byte_counter = 0;
        format_type       = FUNKNOWN;
        //authhttp          = NULL;
        //if (global_settings->httpproxy_state)
        //    authhttp = (auth_http *)(global_settings->httpproxy_state);
        http_header.clear();

        microsoft         = false;
        microsoft_packet_size = 0;
    }

    ~http()
    {
    }


    void init()
    {
        if (strstr(filepath, "MSWMExt="))
        {
            microsoft = true;
            format_type = FASF;
        }
        send_command(cmd_connect());
    }

    void handle()
    {
        if(has_header == false)
        {
            unsigned int temp_size = stream_in->size();
            if (temp_size < 10)
                return;
            byte *temp_data = (byte *)malloc(temp_size + 1);
            stream_in->read(temp_data, temp_size);

            bool do_parse_header = false;
            temp_size -= 3;
            while (temp_size--)
            {
                //if (temp_data[temp_size+0] == '\r' &&
                //    temp_data[temp_size+1] == '\n' &&
                //    temp_data[temp_size+2] == '\r' &&
                //    temp_data[temp_size+3] == '\n')
                if(BE_32(temp_data + temp_size) == BE_32("\r\n\r\n"))
                    do_parse_header = true;
                // if (BE_32(temp_data) == 0x13101310)
            }
            free(temp_data);

            if (do_parse_header)
            {
                parse_header();
            }
        }
        else
        {
            /* handle shoutcast metadata */
            if(meta_int /*&& proto_type == PSHOUTCAST*/)
            {
                read_shoutcast();
            }
            /* not shoutcast, just dump */
            else
            {
                if (microsoft)
                {
                    /* header 1 (type header) */
                    if (stream_in->size() < 8)
                        return;
                    byte ms_head[16];
                    stream_in->read(ms_head, 8);

                    unsigned int len = LE_16_BUF(ms_head + 2);
                    if (stream_in->size() < len + 2)
                        return;

                    // LOG_DEBUG("MS TYPE: %c %c %02x %02x", ms_head[0], ms_head[1], ms_head[2], ms_head[3]);

                    stream_in->erase(4);

                    /* header 2 (pre header) */
                    stream_in->erase(8);
                    len -= 8;

                    /* get min packet len */
                    if(ms_head[0] == '$' && ms_head[1] == 'H')
                    {
                        byte asf_header[16 * 1024];
                        stream_in->read(asf_header, (len < 16000) ? len : 16000);

                        //
                        static byte guid_file[] = {0xA1, 0xDC, 0xAB, 0x8C, 0x47, 0xA9, 0xCF, 0x11,
                                                   0x8E, 0xE4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65
                                                  };

                        for (unsigned int h = 0; h < 16000; h++)
                        {
                            bool file_header = true;
                            for (int g = 0; g < 16; g++)
                            {
                                if (guid_file[g] != asf_header[h + g])
                                    file_header = false;
                            }
                            if (file_header)
                            {
                                microsoft_packet_size = (asf_header[h+92]) | (asf_header[h+93]<<8) |
                                                        (asf_header[h+94]<<16) | (asf_header[h+95]<<24);
                                //LOG_DEBUG("packet_size: %d", microsoft_packet_size);
                                break;
                            }
                        }
                        //
                    }

                    /* opaque */
                    if (ms_head[1] == 'D' || ms_head[1] == 'H')
                    {
                        stream_out->push(*stream_in, len);
                        if (ms_head[1] == 'D')
                            stream_out->putpadding(microsoft_packet_size - len);
                    }
                    else
                    {
                        stream_out->erase(len);
                    }
                }
                else
                {

                    unsigned int data_in_size = stream_in->size();
                    //stream_in->pop(scrap_buffer, data_in_size);
                    //stream_out->push(scrap_buffer, data_in_size);
                    stream_out->push(*stream_in, data_in_size);

                }
            }

        }

    }

protected:

    char* filepath;
    char* hostname;
    unsigned short port;
    format        format_type;
    protocol_type proto_type;

private:

    //auth_http *authhttp;


    unsigned int calc_shoutcast_chunk()
    {
        if (meta_int <= 10000)
            return meta_int;

        static unsigned int divs[] = {10000, 8192, 8000, 5000, 4000, 4096, 2048,
                                2000, 1024, 1000, 512, 500
                               };

        for (int d_i = 0; d_i < int(sizeof(divs) / sizeof(int)); d_i++)
        {
            if (meta_int % divs[d_i] == 0)
                return divs[d_i];
        }

        // LOG_WARNING("calc_shoutcast_chunk() = 1 for meta_int %d", meta_int);

        return 1;
    }

    unsigned int meta_int_chunk;

    void read_shoutcast()
    {
        /* shoutcast metadata is interleaved in the mp3 frames. it works like
        this: [aaaa][Lm][aaaa][Lm][aaaa]...
        where a is audio data / mp3 frames of length meta_int, where L is the
        meta data length (can be 0 - often is), where m is the actual meta
        data of length L*16. */

        //unsigned int meta_int_chunk = calc_shoutcast_chunk();

        //if (meta_int % meta_int_chunk == 0)
        while (stream_in->size() >= meta_int_chunk)
        {
            if (meta_byte_counter == meta_int)
            {
                unsigned int meta_size = (unsigned int)stream_in->peek() * 16;

                if (meta_size)
                {
                    if (stream_in->size() < meta_size + 1)
                        return;

                    stream_in->getbyte();
                    char meta_data[256 * 16];
                    stream_in->pop((byte *)meta_data, meta_size);
                    meta_data[meta_size] = 0;

                    //
                    station_meta *temp_meta = (station_meta *)common_data;
                    std::string tempsong = std::string(meta_data);
                    size_t rpos = tempsong.find("StreamTitle='");
                    if (rpos != tempsong.npos)
                    {
                        tempsong.erase(0, rpos + 13);
                        std::string song_title = tempsong.substr(0, tempsong.find("';"));
                        //temp_meta->info.song =
                        //    tempsong.substr(0, tempsong.find("';"));

                        if (song_title.size() > 3)
                            temp_meta->info.song = song_title;

                        //LOG_DEBUG("[%s]", temp_meta->info.song.c_str());
                    }
                    //
                }
                else
                {
                    stream_in->getbyte();
                }

                meta_byte_counter = 0;
            }
            else
            {
                if(meta_int_chunk == 1)
                    stream_out->putbyte(stream_in->getbyte());
                else
                    stream_out->push(*stream_in, meta_int_chunk);
                meta_byte_counter += meta_int_chunk;
            }
        }

    }

    std::string cmd_connect()
    {
        /* there should not be a space in "Icy-MetaData:1". */
        char temp[1024*2];

        //bool proxy = (global_settings->httpproxy_port != 0) &&
        //    (global_settings->httpproxy_connect == false);

        /***/
        std::string full_path = filepath;
        //if (proxy)
        //    full_path = ssprintf("http://%s:%d%s", hostname, port, filepath);
        //string proxy_row;
        /*if (authhttp)
        {
            string proxy_auth = authhttp->create("GET", filepath);
            proxy_row = ssprintf("Proxy-Authorization: %s\r\n", proxy_auth.c_str());
        }*/

        if(!microsoft)
        {
            sprintf(temp,
                    "GET %s HTTP/1.0\r\n"
                    "Host: %s\r\n"
                    "User-Agent: %s\r\n"
                    "Accept: */*\r\n"
                    "Icy-Metadata:1\r\n"
                    "Connection: close\r\n\r\n",
                    full_path.c_str(), hostname, "WinampMPEG/5.19");

            if(GlobalConfig->uselog)
                GlobalConfig->MyAddLogHeader(temp, true);

        }
        else
        {
            //sprintf(temp,
            //    "GET %s HTTP/1.0\r\n"
            //    "Host: %s\r\n"
            //    "User-Agent: %s\r\n"
            //    "Accept: */*\r\n"
            //    "Pragma: no-cache,rate=1.000000,stream-time=0,stream-offset=0:0,request-context=1,max-duration=0\r\n"
            //    "Pragma: xClientGUID={0D030675-3D55-4C6E-834C-ECF19497EA56}\r\n"
            //    "%s"
            //    "Connection: close\r\n\r\n",
            //    full_path.c_str(), hostname, "NSPlayer/4.1.0.3928", proxy_row.c_str());

            sprintf(temp,
                    "GET %s HTTP/1.0\r\n"
                    "Host: %s\r\n"
                    "User-Agent: %s\r\n"
                    "Accept: */*\r\n"
                    "Pragma: no-cache,rate=1.000000,stream-time=0,stream-offset=4294967295:4294967295,request-context=2,max-duration=0\r\n"
                    "Pragma: xClientGUID={0D030675-3D55-4C6E-834C-ECF19497EA56}\r\n"
                    "Pragma: xPlayStrm=1\r\n"
                    "Pragma: stream-switch-count=1\r\n"
                    "Pragma: stream-switch-entry=ffff:1:0 \r\n"
                    "\r\n",
                    full_path.c_str(), hostname, "NSPlayer/4.1.0.3928");
            if(GlobalConfig->uselog)
                GlobalConfig->MyAddLogHeader(temp, true);
        }
        return std::string(temp);
    }

    void parse_header()
    {
        char cheader[2048 + 1];
        stream_in->read((byte *)cheader, 2048);
        std::string header(cheader);

        // log ...

        //if (strstr(cheader, "407 Proxy"))
        //{

        //}
        //else

        /* all kinds of http based protocols give 200 OK upon OK. we don't
        have to do anything though, a failed connection means the server will
        disconnect us. */
        if(!strstr(cheader, "200 OK"))
        {
            //LOG_DEBUG("No 200 OK");
            failure = true;
        }

        /* parse http response header */
        int header_len = 2;
        for (;;)
        {
            size_t rpos = header.find("\r\n");
            if (rpos != header.npos)
            {
                std::string row = header.substr(0, rpos);
                if (row.empty())
                    break;
                header_len += row.size() + 2;
                size_t cpos = row.find(":");
                if (cpos != header.npos)
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
        }

        /* TODO: we should be able to reuse this class upon 302, but since we
        already have some means to handle indirictions we can use it.
        construct something our playlist parser will understand, pass it out,
        and since connection_failed is set stream_out will be treated as
        playlist. from the protocol stand point this is not very nice, but
        it's robust though. */
        if(strstr(cheader, "302 Found") || strstr(cheader, "302 found"))
        {
            std::string location = http_header["location"];
            std::string location2;
            if (!str_in(location, "://"))
                location2 = "http://";
            location2.append(location);
            location2.append("\n");

            stream_out->push(location2);
            //for (size_t location_i = 0; location_i<location2.size(); location_i++)
            //    stream_out->putbyte((byte)location2[location_i]);
            //stream_out->putbyte('\n');
        }

        /* guess http protocol type and audio format.*/
        if(strstr(cheader, "icy"))
        {
            /* both shoutcast and icecast contain "icy". shoutcast has a
            special status response wheras icecast has a regular http
            response. */

            proto_type = PICECAST;

            /*if (strstr(cheader, "ICY 200 OK"))
            {
                proto_type = PSHOUTCAST;
                meta_int = 0;
                if (!http_header["icy-metaint"].empty())
                    meta_int = atoi(http_header["icy-metaint"].c_str());
            }*/
            if(strstr(cheader, "ICY 200 OK"))
                proto_type = PSHOUTCAST;

            if(!http_header["icy-metaint"].empty())
            {
                meta_int = atoi(http_header["icy-metaint"].c_str());
                meta_int_chunk = calc_shoutcast_chunk();
                //printf("meta int: %d", meta_int);
                //LOG_DEBUG("meta int: %d", meta_int);
                //LOG_DEBUG("meta int chunk: %d", meta_int_chunk);
            }

            /*******************/
            station_meta *temp_meta = (station_meta *)common_data;
            temp_meta->info.name         = http_header["icy-name"];
            temp_meta->extra.url         = http_header["icy-url"];
            temp_meta->extra.description = http_header["icy-description"];
            temp_meta->extra.genre       = http_header["icy-genre"];
            /*******************/
        }
        else
        {
            proto_type = PMSHTTP;
        }

        /* guess http protocol type and audio format.*/
        if(!http_header["content-type"].empty())
        {
            /* audio/x-mpegurl = PeerCast playlist. */
            if (str_in(http_header["content-type"], "mpeg") && !str_in(http_header["content-type"], "audio/x-mpegurl"))
                format_type = FMP3;
            else if (str_in(http_header["content-type"], "ogg"))
                format_type = FVORBIS;
            else if (str_in(http_header["content-type"], "aac"))
                format_type = FAAC;

            /* TODO: implement ms http.
            if we find ms http, use mms instead. safer. */
            else if (str_in(http_header["content-type"], "asf") ||
                     str_in(http_header["content-type"], "wma") ||
                     str_in(http_header["content-type"], "o/x-ms-"))
            {
                if (microsoft)
                    format_type = FASF;
            }

            /* unknown. */
            else
            {
                // LOG_WARNING("unknown content-type: %s\n",
                //             http_header["content-type"].c_str());
            }
        }
        else
        {
            if (microsoft)
                format_type = FASF;
            else
                format_type = FMP3;
        }

        /* remove header from stream */
        stream_in->erase(header_len);

        /*if (strstr(cheader, "PeerCast"))
        {
            size_t p_len = stream_in->size();
            if (p_len > 1023)
                p_len = 1023;
            LOG_DEBUG("PeerCast1 p_len=%d: %s", p_len, cheader);
            cheader[0] = 0;
            stream_in->pop((byte *)cheader, p_len);
            LOG_DEBUG("PeerCast2: %s", cheader);
            stream_out->push((byte *)cheader, p_len);
        }*/

        /* additional checking (just in case) */
        if(strstr(cheader, "\r\n\r\nOggS"))
            format_type = FVORBIS;

        has_header = true;
    }

    std::map<std::string, std::string> http_header;

    //byte* scrap_buffer;
    int   meta_byte_counter;
    int   meta_int;
    bool  has_header;
    bool  microsoft;
    int   microsoft_packet_size;
};

#endif // PROTOCOL_HTTP_H_INCLUDED
