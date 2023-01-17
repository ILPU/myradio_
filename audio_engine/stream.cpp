#include "stream.h"
#define STREAM_OPTIMIZE_TIME

/*void mem_guard2(byte *ptr)
{
    uint32 *c = (uint32 *)ptr;
    c[0] = 0xcccccccc;
    c[1] = 1650802786;
    c[2] = 1785884206;
    c[3] = 1936011318;
}

void mem_assert2(byte *ptr)
{
    uint32 *c = (uint32 *)ptr;
    if (c[0] != 0xcccccccc || c[1] != 1650802786 ||
            c[2] != 1785884206 || c[3] != 1936011318)
    {
        //fprintf(stderr, "GRAVE FUCKING DANGER!!!!");
        exit(0);
    }
}*/

void stream :: GUARD()
{
    byte *ptr = buffer + buffer_size + 1;

    uint32 *c = (uint32 *)ptr;
    c[0] = 0xcccccccc;
    c[1] = 0xcccccccc;
    c[2] = 0xcccccccc;
    c[3] = 0xcccccccc;
}

void stream :: ATTEST()
{
    byte *ptr = buffer + buffer_size + 1;

    uint32 *c = (uint32 *)ptr;

    if (c[0] != 0xcccccccc || c[1] != 0xcccccccc ||
            c[2] != 0xcccccccc || c[3] != 0xcccccccc)
    {
        //fprintf(stderr, "Whoops, memory corrupted.");
        exit(0);
    }
}

/****/

stream :: stream()
{
    init_vars();
}

stream :: stream(wstring &s)
{
    init_vars();
    for (uint32 i = 0; i < s.size(); i++)
        putbyte((byte)s[i]);
}

stream :: stream(std::string &s)
{
    init_vars();
    for (uint32 i = 0; i < s.size(); i++)
        putbyte((byte)s[i]);
}

stream :: stream(stream &s)
{
    init_vars();
    uint32 s2size = s.size();
    byte *temp = (byte *)malloc(s2size + 1);
    s.read(temp, s2size);
    push(temp, s2size);
    free(temp);
}

stream :: stream(byte *block, uint32 count)
{
    init_vars();
    push(block, count);
}

stream :: ~stream()
{
    free(buffer);
}

uint32 stream :: readprt(void* block, uint32 count)
{
    //mem_assert2(buffer + buffer_size + 1);
    ATTEST();

    uint32 actual_read = count;
    if ((int)actual_read > len)
        actual_read = len;

    if (actual_read < buffer_size - b)
    {
        memcpy(block, buffer + b, actual_read);
    }
    else
    {
        uint32 middle_to_end = buffer_size - b;
        uint32 begin_to_middle = actual_read - middle_to_end;

        memcpy(block, buffer + b, middle_to_end);
        memcpy(block + middle_to_end, buffer, begin_to_middle);
    }

    return actual_read;
}

uint32 stream :: read(byte *block, uint32 count)
{
    //mem_assert2(buffer + buffer_size + 1);
    ATTEST();

    uint32 actual_read = count;
    if ((int)actual_read > len)
        actual_read = len;

    if (actual_read < buffer_size - b)
    {
        memcpy(block, buffer + b, actual_read);
    }
    else
    {
        uint32 middle_to_end = buffer_size - b;
        uint32 begin_to_middle = actual_read - middle_to_end;

        memcpy(block, buffer + b, middle_to_end);
        memcpy(block + middle_to_end, buffer, begin_to_middle);
    }

    return actual_read;
}

byte* stream :: pop(byte *block, uint32 count)
{
    //mem_assert2(buffer + buffer_size + 1);
    ATTEST();

    uint32 actual_read = read(block, count);
    erase(actual_read);
    return block;
}

uint32 stream :: push(stream &str, uint32 count = -1)
{
    //mem_assert2(buffer + buffer_size + 1);
    ATTEST();

    /* align */
    if (len < 1)
        clear();

    if (count == (uint32)-1)
        count = str.size();

    if (count + len >= buffer_size)
        resize(count + len + 1);

    if (count < buffer_size - e)
    {
        str.pop(buffer + e, count);
        e += count;
    }
    else
    {
        uint32 middle_to_end = buffer_size - e;
        uint32 begin_to_middle = count - middle_to_end;

        str.pop(buffer + e, middle_to_end);
        str.pop(buffer, begin_to_middle);

        e = begin_to_middle;
    }

    len += count;

    return count;
}

uint32 stream :: push(byte *block, uint32 count)
{
    //mem_assert2(buffer + buffer_size + 1);
    ATTEST();

    /* align */
    if (len < 1) /* <= 0 */
        clear();

    if (count + len >= buffer_size)
        resize(count + len + 1);

    if (count < buffer_size - e)
    {
        memcpy(buffer + e, block, count);
        e += count;
    }
    else
    {
        uint32 middle_to_end = buffer_size - e;
        uint32 begin_to_middle = count - middle_to_end;

        memcpy(buffer + e, block, middle_to_end);
        memcpy(buffer, block + middle_to_end, begin_to_middle);

        e = begin_to_middle;
    }

    len += count;

    return count;
}

uint32 stream :: push(wstring &s)
{
    /* align */
    if (len < 1)
        clear();

    uint32 i;
    for (i = 0; i < s.size(); i++)
        putbyte((byte)s[i]);
    return i;
}

uint32 stream :: push(std::string &s)
{
    /* align */
    if (len < 1)
        clear();

    uint32 i;
    for (i = 0; i < s.size(); i++)
        putbyte((byte)s[i]);
    return i;
}

void stream :: putbyte(byte val)
{
    if ((uint32)len == buffer_size - 1) /* len + 1 >= buffer_size */
        resize(len + 1 + 1);

    buffer[e] = val;
    len++;

    e++;
#ifdef STREAM_OPTIMIZE_TIME
    e &= ~buffer_size;
#endif
#ifdef STREAM_OPTIMIZE_SPACE
    if (e == buffer_size)
        e = 0;
#endif
}

void stream :: erase(uint32 count)
{
    if(!count) return;
    uint32 middle_to_end = buffer_size - b;
    if (count < middle_to_end)
        b += count;
    else
        b = count - middle_to_end;
    len -= count;
    if (len < 1)
        clear();
}

/* TODO: There should be a better way to handle
boundary cases for the get accessors. */

uint16 stream :: get16le()
{
    if (b - 2 < buffer_size)
    {
        uint16 val = LE_16_BUF(buffer + b);
        b += 2;
        len -= 2;
        STREAM_WRAPAROUND;
        return val;
    }
    else
    {
        uint16 val = (uint16)getbyte();
        val |= (uint16)getbyte()<<8;
        return val;
    }
}
uint16 stream :: get16be()
{
    if (b - 2 < buffer_size)
    {
        uint16 val = BE_16_BUF(buffer + b);
        b += 2;
        len -= 2;
        STREAM_WRAPAROUND;
        return val;
    }
    else
    {
        uint16 val = (uint16)getbyte()<<8;
        val |= (uint16)getbyte();
        return val;
    }
}

uint32 stream :: get32le()
{
    if (b - 4 < buffer_size)
    {
        uint32 val = LE_32_BUF(buffer + b);
        b += 4;
        len -= 4;
        STREAM_WRAPAROUND;
        return val;
    }
    else
    {
        uint32 val = (uint32)getbyte();
        val |= (uint32)getbyte()<<8;
        val |= (uint32)getbyte()<<16;
        val |= (uint32)getbyte()<<24;
        return val;
    }
}

uint32 stream :: get32be()
{
    if (b - 4 < buffer_size)
    {
        uint32 val = BE_32_BUF(buffer + b);
        b += 4;
        len -= 4;
        STREAM_WRAPAROUND;
        return val;
    }
    else
    {
        uint32 val = (uint32)getbyte()<<24;
        val |= (uint32)getbyte()<<16;
        val |= (uint32)getbyte()<<8;
        val |= (uint32)getbyte();
        return val;
    }
}

uint64 stream :: get64le()
{
    if (b - 8 < buffer_size)
    {
        uint64 val = LE_64_BUF(buffer + b);
        b += 8;
        len -= 8;
        STREAM_WRAPAROUND;
        return val;
    }
    else
    {
        uint64 val = (uint64)get32le();
        val |= (uint64)get32le()<<32;
        return val;
    }
}
uint64 stream :: get64be()
{
    if (b - 8 < buffer_size)
    {
        uint64 val = BE_64_BUF(buffer + b);
        b += 8;
        len -= 8;
        STREAM_WRAPAROUND;
        return val;
    }
    else
    {
        uint64 val = (uint64)getbyte()<<56;
        val |= (uint64)getbyte()<<48;
        val |= (uint64)getbyte()<<40;
        val |= (uint64)getbyte()<<32;
        val |= (uint64)getbyte()<<24;
        val |= (uint64)getbyte()<<16;
        val |= (uint64)getbyte()<<8;
        val |= (uint64)getbyte();
        return val;
    }
}

void stream :: put16le(uint16 n)
{
    putbyte(n & 0xFF);
    putbyte(n >> 8);
}

void stream :: put16be(uint16 n)
{
    putbyte(n >> 8);
    putbyte(n & 0xFF);
}

void stream :: put32le(uint32 n)
{
    putbyte(n & 0xFF);
    putbyte((n >> 8) & 0xFF);
    putbyte((n >> 16) & 0xFF);
    putbyte(n >> 24);
}

void stream :: put32be(uint32 n)
{
    putbyte(n >> 24);
    putbyte((n >> 16) & 0xFF);
    putbyte((n >> 8) & 0xFF);
    putbyte(n & 0xFF);
}

void stream :: putpadding(size_t len)
{
    while (len--)
        putbyte(0);
}

void stream :: resize(size_t new_size)
{
    uint32 old_buffer_size = buffer_size;

    /* malloc likes powers of two. as a bonus the constant in our amortized runtime
    becomes logarithimic for getbyte() etc. since we don't have to resize as buffer
    grows non-linearly. */
#ifdef STREAM_OPTIMIZE_TIME
    //mem_assert2(buffer + buffer_size + 1);
    ATTEST();

    while (buffer_size < new_size)
        buffer_size <<= 1;

    buffer = (byte *)realloc(buffer, buffer_size + STREAM_BUFFER_PADDING);
    //mem_guard2(buffer + buffer_size + 1);
    GUARD();
#endif

#ifdef STREAM_OPTIMIZE_SPACE
    uint32 blocks = new_size / STREAM_UNIT_SIZE + 1;
    buffer_size = blocks * STREAM_UNIT_SIZE;
    buffer = (byte *)realloc(buffer, buffer_size + 1 + STREAM_BUFFER_PADDING);
#endif

    //printf("resize(%d)\n", new_size);
    //printf("old_buffer_size %d\n", old_buffer_size);
    //printf("buffer_size %d\n", buffer_size);

    /* realign */
    if (e < b)
    {
        //
        // e = 7    b = 55   len = 16
        //                      1                   2                   3
        //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |x|x|x|x|x|x|x|e| | | | | | | | | | | | | | | | | | | | | | | | |
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // | | | | | | | | | | | | | | | | | | | | | | | |b|x|x|x|x|x|x|x|x|
        // +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=old_buffer_size
        // +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
        // |X|X|X|X|X|X|X|E| | | | | | | | | | | | | | | | | | | | | | | | |
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-buffer_size
        //
        // e = 55 + 16 = 71
        //
        if (b + len + e < buffer_size)
        {
            //printf("yyy");
            memcpy(buffer + b + len - e, buffer, e);
            e = b + len;
        }
        //
        // e = 34    b = 55   len = 43
        //                      1                   2                   3
        //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |x|x|e| | | | | | | | | | | | | | | | | | | | |b|x|x|x|x|x|x|x|x|
        // +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=old_buffer_size
        // +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
        // |X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|X|
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-buffer_size
        //
        else
        {
            //printf("xxx\n");

            /*uint32 middle_to_end = buffer_size - old_buffer_size;
            memcpy(buffer + old_buffer_size, buffer, middle_to_end);
            uint32 remaining = e - middle_to_end;
            printf("e: %d\n", e);
            printf("middle_to_end: %d\n", middle_to_end);
            printf("remaining: %d\n", remaining);
            memcpy(buffer, buffer + middle_to_end, remaining);
            e -= middle_to_end;*/

            /* TODO: The above commented code doesn't work, but this case happens
            very rarely (typically less than once per session). It's ugly though, so we should
            rewrite ASAP. */
            byte *temp = (byte *)malloc(buffer_size);
            size_t new_buffer_size = buffer_size;

            buffer_size = old_buffer_size;
            size_t all = size();
            pop(temp, all);
            buffer_size = new_buffer_size;

            push(temp, all);
            free(temp);
        }
    }
}

void stream :: init_vars()
{
    buffer_size = STREAM_UNIT_SIZE;
#ifdef STREAM_OPTIMIZE_SPACE
    buffer_size += STREAM_BUFFER_PADDING;
#endif
    buffer = (byte *)malloc(buffer_size + STREAM_BUFFER_PADDING);
    //mem_guard2(buffer + buffer_size + 1);
    GUARD();
    buffer[0] = 0;
    //set_canary();
    b = 0;
    e = 0;
    len = 0;
}
