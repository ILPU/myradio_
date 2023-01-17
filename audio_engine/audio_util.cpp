#include "common.h"

/* Generally useful canary with int3 guards and some bad magic. */
void mem_guard(byte *ptr)
{
    unsigned int  *c = (unsigned int  *)ptr;
    c[0] = 0xcccccccc;
    c[1] = 1650802786;
    c[2] = 1785884206;
    c[3] = 1936011318;
}

void mem_assert(byte *ptr)
{
    unsigned int  *c = (unsigned int  *)ptr;
    if (c[0] != 0xcccccccc || c[1] != 1650802786 ||
            c[2] != 1785884206 || c[3] != 1936011318)
    {
        //LOG_ERROR("grave danger!");
        exit(0);
    }
}

void *MROutput_AllocateMemory(long size)
{
    void *result = GlobalAlloc(GPTR, size);
}

void MROutput_FreeMemory(void *block)
{
    if(block != NULL)
    {
        GlobalFree(block);
    }
}
