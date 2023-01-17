#ifndef CIRCLEBUFFER_H_INCLUDED
#define CIRCLEBUFFER_H_INCLUDED

#include "common.h"

typedef int (__stdcall *TRecordCallbackFunc)(void* puser_data, void* pdata, unsigned int pbyte_data, BOOL pnotstop);

class CircleBuffer
{
public:
    CircleBuffer(const unsigned int iBufferSize);
    ~CircleBuffer();
    void BufferWrite(const void* _pSourceBuffer, const unsigned int _iNumBytes);
    BOOL BufferRead(void* pDestBuffer, const unsigned int _iBytesToRead, unsigned int* pbBytesRead);
    void Flush();
    unsigned int GetBufferFreeSpace();
    unsigned int GetBufferUsedSpace();
    void SetComplete();
    BOOL IsComplete();

    void SetRecordCallbackFunct(TRecordCallbackFunc rval, void *ruser_data);

protected:

    BYTE* m_pBuffer;
    unsigned int m_iBufferSize;
    unsigned int m_iReadCursor;
    unsigned int m_iWriteCursor;
    HANDLE m_evtDataAvailable;
    CRITICAL_SECTION m_csCircleBuffer;
    BOOL m_bComplete;

    TRecordCallbackFunc breccal;
    void *buser_data;

};

#endif // CIRCLEBUFFER_H_INCLUDED
