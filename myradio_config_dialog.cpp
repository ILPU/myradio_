#include "myhdr.h"
#include "myradio.h"
#include "res/resource.h"

void ConfigDialog::Show()
{
    CreateModal();
}

void ConfigDialog::CreatePages()
{
    InitPage(&psp[0], IDD_DIALOG_CONFIG_OUTPUT, (DLGPROC)OutputProc, (LPARAM)this, 0);
    InitPage(&psp[1], IDD_DIALOG_CONFIG_CONNECT, (DLGPROC)ConnectProc, (LPARAM)this, 0);
    //InitPage(&psp[2], IDD_CONFIG_DECODER, (DLGPROC)DecoderProc, (LPARAM)this, 0);
    //InitPage(&psp[3], IDD_CONFIG_OUTPUT, (DLGPROC)OutputProc, (LPARAM)this, 0);
    //InitPage(&psp[4], IDD_CONFIG_BUFFER, (DLGPROC)BufferProc, (LPARAM)this, 0);

    header.pszCaption = (LPCWSTR)_T("");
    header.nPages = 2;
    header.nStartPage = 0;
    header.ppsp = (LPCPROPSHEETPAGE)&psp;
    header.dwFlags |= PSH_USECALLBACK;
    header.pfnCallback = WindowCallback;
}

void ConfigDialog::EnableControlProxy(HWND wnd)
{
    bool prxon = GlobalConfig->proxy_settings.proxy_on;
    bool prxao = GlobalConfig->proxy_settings.proxy_athoriz_on;
    Dialog_EnableHWND(wnd, IDC_CONFIG_CON_PROXY_TYPE, prxon);
    Dialog_EnableHWND(wnd, IDC_CONFIG_CON_PROXY_SERVER, prxon);
    Dialog_EnableHWND(wnd, IDC_CONFIG_CON_PROXY_PORT, prxon);
    Dialog_EnableHWND(wnd, IDC_CONFIG_CON_PROXY_AVTORIZ, prxon);

    if(!prxon) prxao = false;
    Dialog_EnableHWND(wnd, IDC_CONFIG_CON_PROXY_LOGIN, prxao);
    Dialog_EnableHWND(wnd, IDC_CONFIG_CON_PROXY_PASS, prxao);
}

int WINAPI ConfigDialog::WindowCallback(HWND hwndDlg, UINT msg, LPARAM lParam)
{
    switch(msg)
    {
    case PSCB_PRECREATE:
        if(((LPDLGTEMPLATEEX)lParam)->signature == 0xFFFF)
        {
            ((LPDLGTEMPLATEEX)lParam)->style &= ~DS_CONTEXTHELP;
        }
        else
        {
            ((LPDLGTEMPLATE)lParam)->style &= ~DS_CONTEXTHELP;
        }
        return true;

    case PSCB_INITIALIZED:
        //SetDlgItemText(hwndDlg, IDOK, L"Apply");
        SetWindowText(hwndDlg, _T("Настройки"));

        ShowWindow(Dialog_GetItemHWND(hwndDlg, IDOK), SW_HIDE);
        ShowWindow(Dialog_GetItemHWND(hwndDlg, IDCANCEL), SW_HIDE);

        RECT rcDlg, rcBtnC;
        GetWindowRect(hwndDlg, &rcDlg);
        GetWindowRect(Dialog_GetItemHWND(hwndDlg, IDCANCEL), &rcBtnC);

        //POINT cp;
        //GetCursorPos(&cp);
        //ScreenToClient(hwndDlg, &cp);

        SetWindowPos(hwndDlg,
                     0,
                     0,
                     0,
                     rcDlg.right - rcDlg.left, (rcDlg.bottom - rcDlg.top) - (rcBtnC.bottom - rcBtnC.top) - 4,
                     SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOMOVE);
        //MoveWindow(hwndDlg, 100, 300, rcDlg.right - rcDlg.left, (rcDlg.bottom - rcDlg.top) - (rcBtnC.bottom - rcBtnC.top) - 4, FALSE);
        //CenterWindow(hwndDlg);
        SetFocus(PropSheet_GetTabControl(hwndDlg));
        break;
    }

    return false;
}

bool WINAPI ConfigDialog::OutputProc(HWND dialog, UINT message, WPARAM wparam, LPARAM lparam)
{
    ConfigDialog *_this = (ConfigDialog *)GetWindowLongPtr(dialog, GWLP_USERDATA);
    return _this->OutputPage(dialog, message, wparam, lparam);
}


bool WINAPI ConfigDialog::ConnectProc(HWND dialog, UINT message, WPARAM wparam, LPARAM lparam)
{
    ConfigDialog *_this = (ConfigDialog *)GetWindowLongPtr(dialog, GWLP_USERDATA);
    return _this->ConnectPage(dialog, message, wparam, lparam);
}

bool ConfigDialog::OutputPage(HWND dialog, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch(message)
    {

    case WM_INITDIALOG:
    {
        //
    }
    break;

    case WM_DESTROY:
        GlobalConfig->SaveOutputSettings();
        return false;

    case WM_NOTIFY:
        switch(((NMHDR *) lparam)->code)
        {
        case PSN_SETACTIVE:
            SetWindowLongPtr(dialog, DWL_MSGRESULT, 0);
            break;

        case PSN_KILLACTIVE:
            SetWindowLongPtr(dialog, DWL_MSGRESULT, FALSE);
            break;

        case PSN_APPLY:
            //
            SetWindowLongPtr(dialog, DWL_MSGRESULT, PSNRET_NOERROR);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

bool ConfigDialog::ConnectPage(HWND dialog, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch(message)
    {

    case WM_INITDIALOG:
    {
        SendDlgItemMessage(dialog, IDC_CONFIG_CON_PROXY_TYPE, CB_RESETCONTENT, 0, 0);
        SendDlgItemMessage(dialog, IDC_CONFIG_CON_PROXY_TYPE, CB_ADDSTRING, 0, (LPARAM)_T("HTTP"));
        SendDlgItemMessage(dialog, IDC_CONFIG_CON_PROXY_TYPE, CB_ADDSTRING, 0, (LPARAM)_T("HTTP CONNECT"));
        SendDlgItemMessage(dialog, IDC_CONFIG_CON_PROXY_TYPE, CB_ADDSTRING, 0, (LPARAM)_T("SOCKS4"));
        SendDlgItemMessage(dialog, IDC_CONFIG_CON_PROXY_TYPE, CB_ADDSTRING, 0, (LPARAM)_T("SOCKS5"));

        EnableControlProxy(dialog);
        SendDlgItemMessage(dialog, IDC_CONFIG_CON_PROXY_TYPE, CB_SETCURSEL, PrTypSet2CBCurSel(GlobalConfig->proxy_settings.proxy_type), 0);
        Dialog_SetItemCheck(dialog, IDC_CONFIG_CON_PROXY_ON, GlobalConfig->proxy_settings.proxy_on);
        Dialog_SetItemCheck(dialog, IDC_CONFIG_CON_PROXY_AVTORIZ, GlobalConfig->proxy_settings.proxy_athoriz_on);

        DSetItemText(dialog, GlobalConfig->proxy_settings.proxy_ip, IDC_CONFIG_CON_PROXY_SERVER);
        DSetItemInt(dialog, GlobalConfig->proxy_settings.proxy_port, IDC_CONFIG_CON_PROXY_PORT);
        DSetItemText(dialog, GlobalConfig->proxy_settings.proxy_username, IDC_CONFIG_CON_PROXY_LOGIN);
        DSetItemText(dialog, GlobalConfig->proxy_settings.proxy_password, IDC_CONFIG_CON_PROXY_PASS);
    }
    break;

    case WM_DESTROY:
        GlobalConfig->SaveConnectSettings();
        return false;

    case WM_COMMAND:
    {
        if(HIWORD(wparam) == BN_CLICKED)
        {
            if(LOWORD(wparam) == IDC_CONFIG_CON_PROXY_ON)
            {
                GlobalConfig->proxy_settings.proxy_on = bool(Dialog_GetItemCheck(dialog, IDC_CONFIG_CON_PROXY_ON));
                EnableControlProxy(dialog);
            }
            else if(LOWORD(wparam) == IDC_CONFIG_CON_PROXY_AVTORIZ)
            {
                GlobalConfig->proxy_settings.proxy_athoriz_on = bool(Dialog_GetItemCheck(dialog, IDC_CONFIG_CON_PROXY_AVTORIZ));
                EnableControlProxy(dialog);
            }
        }
        else if(HIWORD(wparam) == EN_CHANGE)
        {
            if(LOWORD(wparam) == IDC_CONFIG_CON_PROXY_SERVER)
            {
                Dialog_GetItemText2(dialog, IDC_CONFIG_CON_PROXY_SERVER, GlobalConfig->proxy_settings.proxy_ip, 32);
            }
            else if(LOWORD(wparam) == IDC_CONFIG_CON_PROXY_PORT)
            {
                GlobalConfig->proxy_settings.proxy_port = (unsigned short)Dialog_GetItemInt(dialog, IDC_CONFIG_CON_PROXY_PORT);
            }
            else if(LOWORD(wparam) == IDC_CONFIG_CON_PROXY_LOGIN)
            {
                Dialog_GetItemText2(dialog, IDC_CONFIG_CON_PROXY_LOGIN, GlobalConfig->proxy_settings.proxy_username, 128);
            }
            else if(LOWORD(wparam) == IDC_CONFIG_CON_PROXY_PASS)
            {
                Dialog_GetItemText2(dialog, IDC_CONFIG_CON_PROXY_PASS, GlobalConfig->proxy_settings.proxy_password, 128);
            }
        }
        else if(HIWORD(wparam) == CBN_SELCHANGE)
        {
            if(LOWORD(wparam) == IDC_CONFIG_CON_PROXY_TYPE)
            {
                GlobalConfig->proxy_settings.proxy_type = CBCurSelPrTypSet(SendDlgItemMessage(dialog, IDC_CONFIG_CON_PROXY_TYPE, CB_GETCURSEL, 0, 0));
            }
        }
    }
    break;

    case WM_NOTIFY:
        switch(((NMHDR *) lparam)->code)
        {
        case PSN_SETACTIVE:
            SetWindowLongPtr(dialog, DWL_MSGRESULT, 0);
            break;

        case PSN_KILLACTIVE:
            SetWindowLongPtr(dialog, DWL_MSGRESULT, FALSE);
            break;

        case PSN_APPLY:
            //
            SetWindowLongPtr(dialog, DWL_MSGRESULT, PSNRET_NOERROR);
            break;
        }
        return TRUE;
    }

    return FALSE;
}
