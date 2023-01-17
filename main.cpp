#include "myhdr.h"
#include "myradio.h"

/**/
static HANDLE mutex;

void closemymutex()
{
    CloseHandle(mutex);
    mutex = 0;
}

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
{
    int ret = 0;

    mutex = CreateMutex(0, FALSE, APP_ATOM);
    DWORD DW;

    if(mutex != 0)
    {
        DW = WaitForSingleObject(mutex, 0);
        if(DW == WAIT_OBJECT_0)
        {
            MRadioApp *rapp = new MRadioApp(hThisInstance, lpszArgument);
            ret = rapp->CreateApp();
            if (ret != 1)
            {
                delete rapp;
                ReleaseMutex(mutex);
                closemymutex();
                return ret;
            }

            ret = rapp->CreateMainForm();
            if (ret != 1)
            {
                delete rapp;
                ReleaseMutex(mutex);
                closemymutex();
                return ret;
            }

            rapp->RunProgramm();

            ret = rapp->GetMsgWParam();

            delete rapp;
            ReleaseMutex(mutex);
        }
        else
        {
            //MessageBox(0,_T("!!!!"),_T(""), 0);
            // уже есть копия программы
        }
    }

    closemymutex();
    return ret;
}
