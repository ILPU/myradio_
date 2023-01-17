#ifndef STREAM_H
#define STREAM_H

#include "../myhdr.h"
#include "types.h"

//#include <deque>

//using namespace std;

class stream
{
public:
    stream();
    explicit stream(wstring &s);
    explicit stream(std::string &s);
    explicit stream(stream &s);
    stream(byte *block, unsigned int  len);
    ~stream();

    /*
     *
     */
    unsigned int  push(byte *block, unsigned int  count);
    unsigned int  push(stream &str, unsigned int  count);
    unsigned int  push(wstring &s);
    unsigned int  push(std::string &s);

    unsigned int  read(byte *block, unsigned int  count);
    unsigned int  readprt(void* block, unsigned int  count);
    byte*  pop(byte *buffer, unsigned int  count);

    /*
     *
     */
    bool   empty();
    size_t size();
    void   erase(unsigned int  count);
    void   clear();

    /*
     *
     */
    byte   peek();
    byte   getbyte();
    unsigned short get16le();
    unsigned short get16be();
    unsigned int  get32le();
    unsigned int  get32be();
    unsigned long long int get64le();
    unsigned long long int get64be();

    void   putbyte(byte b);
    void   put16le(unsigned short n);
    void   put16be(unsigned short n);
    void   put32le(unsigned int  n);
    void   put32be(unsigned int  n);
    void   put64le(unsigned long long int n);
    void   put64be(unsigned long long int n);
    void   putpadding(size_t len);

private:
    void   init_vars();
    void   resize(size_t new_size);

    void   GUARD();
    void   ATTEST();

    //deque<byte> data;
    byte *buffer;
    unsigned int  b, e;
    signed int len; /* signed for comparision <= 0 */
    size_t buffer_size;
};

/*
 * rollerblades
 */
/*
inline bool stream :: empty()
{
    return data.empty();
}

inline unsigned int  stream :: size()
{
    return (unsigned int )data.size();
}*/

#define STREAM_UNIT_SIZE 524288
#define STREAM_BUFFER_PADDING 32
#define STREAM_OPTIMIZE_TIME

#ifdef STREAM_OPTIMIZE_SPACE
#define STREAM_WRAPAROUND if (b >= buffer_size) { b = 0; }
#endif
#ifdef STREAM_OPTIMIZE_TIME
/* we can avoid the branch since buffer_size is a power of two. */
#define STREAM_WRAPAROUND b &= ~buffer_size;
#endif

inline void stream :: clear()
{
    b = 0;
    e = 0;
    len = 0;
}

inline byte stream :: getbyte()
{
    byte val = buffer[b];
    ++b;
    --len;
    STREAM_WRAPAROUND;
    return val;
}

inline byte stream :: peek()
{
    return buffer[b];
}

inline size_t stream :: size()
{
    return len;
}

inline bool stream :: empty()
{
    return len == 0;
}

#endif
