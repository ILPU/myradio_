#include "myhdr.h"
#include "myradio.h"

BaseDialogWindow::BaseDialogWindow(MRadioApp *vmra)
{
    mra = vmra;
    result = 0;
}

BaseDialogWindow::~BaseDialogWindow()
{
}

void BaseDialogWindow::CreateModal()
{
    header.dwSize = sizeof(PROPSHEETHEADER);
    header.dwFlags = (PSH_PROPSHEETPAGE | PSH_PROPTITLE | PSH_NOAPPLYNOW);
    header.hwndParent = mra->hWin;

    header.hInstance = mra->MainInst;
    header.pszIcon = NULL;
    header.nPages = 0;
    header.nStartPage = 0;

    CreatePages();
    if(header.nPages > 0)
    {
        result = PropertySheet(&header);
    }
}

void BaseDialogWindow::CreatePages()
{
    header.nPages = 0;
    header.pszCaption = (LPCWSTR)_T("");
    header.ppsp = (LPCPROPSHEETPAGE)0;
}

DWORD BaseDialogWindow::GetResult()
{
    return result;
}

void BaseDialogWindow::InitPage(PROPSHEETPAGE *page, int id, DLGPROC dlgproc, LPARAM lparam, LPFNPSPCALLBACK callback)
{
    page->dwSize      = sizeof(*page);
    page->dwFlags     = PSP_DEFAULT | (callback ? PSP_USECALLBACK : 0);
    page->hInstance   = header.hInstance;
    page->pszTemplate = MAKEINTRESOURCE(id);
    page->pszIcon     = 0;
    page->pszTitle    = 0;
    page->pfnDlgProc  = dlgproc;
    page->lParam      = lparam;
    page->pfnCallback = callback;
    page->pcRefParent = 0;
}
