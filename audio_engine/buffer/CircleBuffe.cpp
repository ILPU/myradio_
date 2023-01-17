#include "CircleBuffer.h"

#define CIC_WAITTIMEOUT  3000

CircleBuffer::CircleBuffer(const unsigned int iBufferSize)
{
    m_iBufferSize = iBufferSize;
    m_pBuffer = (BYTE*)malloc(iBufferSize);
    m_iReadCursor = 0;
    m_iWriteCursor = 0;
    m_bComplete = FALSE;
    m_evtDataAvailable = CreateEvent(NULL, FALSE, FALSE, NULL);
    buser_data = NULL;
    breccal = NULL;
    InitializeCriticalSection(&m_csCircleBuffer);
}

void CircleBuffer::SetRecordCallbackFunct(TRecordCallbackFunc rval, void *ruser_data)
{
    breccal    = rval;
    buser_data = ruser_data;
}

CircleBuffer::~CircleBuffer()
{
    DeleteCriticalSection(&m_csCircleBuffer);
    CloseHandle(m_evtDataAvailable);
    free(m_pBuffer);
}

void CircleBuffer::BufferWrite(const void* _pSourceBuffer, const unsigned int _iNumBytes)
{
    unsigned int iBytesToWrite = _iNumBytes;
    BYTE* pReadCursor = (BYTE*)_pSourceBuffer;

    EnterCriticalSection(&m_csCircleBuffer);


    if (m_iWriteCursor >= m_iReadCursor)
    {

        unsigned int iChunkSize = m_iBufferSize - m_iWriteCursor;

        if (iChunkSize > iBytesToWrite)
            iChunkSize = iBytesToWrite;


        memcpy(m_pBuffer + m_iWriteCursor,
               pReadCursor, iChunkSize);

        pReadCursor += iChunkSize;

        iBytesToWrite -= iChunkSize;
        m_iWriteCursor += iChunkSize;

        if (m_iWriteCursor >= m_iBufferSize)
            m_iWriteCursor -= m_iBufferSize;
    }


    if (iBytesToWrite)
    {
        memcpy(m_pBuffer + m_iWriteCursor,
               pReadCursor, iBytesToWrite);
        m_iWriteCursor += iBytesToWrite;
    }

    SetEvent(m_evtDataAvailable);

    LeaveCriticalSection(&m_csCircleBuffer);
}


BOOL CircleBuffer::BufferRead(void* pDestBuffer, const unsigned int _iBytesToRead, unsigned int* pbBytesRead)
{
    unsigned int iBytesToRead = _iBytesToRead;
    unsigned int iBytesRead = 0;
    DWORD dwWaitResult;
    BOOL bComplete = FALSE;

    while (iBytesToRead > 0 && bComplete == FALSE)
    {
        dwWaitResult = WaitForSingleObject(m_evtDataAvailable, CIC_WAITTIMEOUT);

        if (dwWaitResult == WAIT_TIMEOUT)
        {
            *pbBytesRead = iBytesRead;
            return FALSE;
        }

        EnterCriticalSection(&m_csCircleBuffer);


        if (m_iReadCursor > m_iWriteCursor)
        {
            unsigned int iChunkSize = m_iBufferSize - m_iReadCursor;

            if (iChunkSize > iBytesToRead)
                iChunkSize = iBytesToRead;

            // Perform the read
            memcpy((BYTE*)pDestBuffer + iBytesRead,
                   m_pBuffer + m_iReadCursor,
                   iChunkSize);

            iBytesRead += iChunkSize;
            iBytesToRead -= iChunkSize;

            m_iReadCursor += iChunkSize;

            if (m_iReadCursor >= m_iBufferSize)
                m_iReadCursor -= m_iBufferSize;
        }

        if (iBytesToRead && m_iReadCursor < m_iWriteCursor)
        {
            unsigned int iChunkSize = m_iWriteCursor - m_iReadCursor;

            if (iChunkSize > iBytesToRead)
                iChunkSize = iBytesToRead;

            memcpy((BYTE*)pDestBuffer + iBytesRead,
                   m_pBuffer + m_iReadCursor,
                   iChunkSize);

            iBytesRead += iChunkSize;
            iBytesToRead -= iChunkSize;
            m_iReadCursor += iChunkSize;
        }


        if (m_iReadCursor == m_iWriteCursor)
        {
            if (m_bComplete)
                bComplete = TRUE;
        }

        else
            SetEvent(m_evtDataAvailable);

        LeaveCriticalSection(&m_csCircleBuffer);
    }

    *pbBytesRead = iBytesRead;
    BOOL temp = bComplete ? FALSE : TRUE;
    if(breccal) breccal(buser_data, pDestBuffer, iBytesRead, temp);
    return temp;
}

void CircleBuffer::Flush()
{
    EnterCriticalSection(&m_csCircleBuffer);
    m_iReadCursor = 0;
    m_iWriteCursor = 0;
    LeaveCriticalSection(&m_csCircleBuffer);
}

unsigned int CircleBuffer::GetBufferFreeSpace()
{
    unsigned int iNumBytesFree;

    EnterCriticalSection(&m_csCircleBuffer);

    if (m_iWriteCursor < m_iReadCursor)
        iNumBytesFree = (m_iReadCursor - 1) - m_iWriteCursor;
    else if (m_iWriteCursor == m_iReadCursor)
        iNumBytesFree = m_iBufferSize;
    else
        iNumBytesFree = (m_iReadCursor - 1) + (m_iBufferSize - m_iWriteCursor);

    LeaveCriticalSection(&m_csCircleBuffer);

    return iNumBytesFree;
}

unsigned int CircleBuffer::GetBufferUsedSpace()
{
    return m_iBufferSize - GetBufferFreeSpace();
}

void CircleBuffer::SetComplete()
{
    EnterCriticalSection(&m_csCircleBuffer);
    m_bComplete = TRUE;
    SetEvent(m_evtDataAvailable);
    LeaveCriticalSection(&m_csCircleBuffer);
}

BOOL CircleBuffer::IsComplete()
{
    return m_bComplete;
}
