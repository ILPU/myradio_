#include "myhdr.h"
#include "myradio.h"
#include "res/resource.h"

MRadioApp *MRadioApp::instance = NULL;

MRadioApp::MRadioApp(HINSTANCE phInst, LPSTR lpszCmdLine)
{
    MainInst                = phInst;
    hWin                    = NULL;
    hwndStatus              = NULL;
    hwndTV                  = NULL;
    hwndRadioList           = NULL;
    hwndPanel               = NULL;
    hwndToolBar             = NULL;

    SplitCursor             = NULL;

    tb_imagelist            = NULL;
    tb_bmp                  = NULL;

    UpDateBaseProc          = false;
    MainDbDataExists        = false;

    memset(&current_base_info, 0, sizeof(base_info));
}

MRadioApp::~MRadioApp()
{
    GlobalConfig->MyAddLog(_T("delete MRadioApp"));
    if(GlobalConfig)
        delete(GlobalConfig);
}

int MRadioApp::CreateApp()
{
    GetSysInfo();
    if(sysinfo.version < V_WINDOWS_NT)
    {
        MessageBoxA(0, "Cannot run on Windows 95/98/ME!\r\nThe program will be closed.", "", MB_OK|MB_ICONERROR|MB_TASKMODAL|MB_TOPMOST|MB_SETFOREGROUND);
        return 2;
    }

    ConfigDir.append(mySHGetFolderPath(1));
    StartDir.append(myGetStartDir(MainInst));

    ConfigDir+=_T("\\myradio_app");

    GlobalConfig = new Config(ConfigDir);
    GlobalConfig->LoadLogSetting();
    GlobalConfig->MyAddLog(_T("****** MyRadio Log File ******"), false);
    GlobalConfig->MyAddLog(_T("Start app, create MRadioApp"));
    //GlobalConfig->MyAddLog(_T("РУССКИЙ ТЕКСТ тест тест TEST tets РУСС"));
    wstring tmp1, tmp2;
    WinVer2WString(sysinfo.version, &tmp1);
    Arch2WString(sysinfo.arch, &tmp2);

    GlobalConfig->MyAddLogParam(_T("System: %s, %s"), tmp1.c_str(), tmp2.c_str());
    GlobalConfig->MyAddLogParam(_T("Start dir: %s"), StartDir.c_str());
    GlobalConfig->MyAddLogParam(_T("Config dir: %s"), ConfigDir.c_str());

    WNDCLASSEX wincl;
    wincl.hInstance = MainInst;
    wincl.lpszClassName = APP_CLASS;
    wincl.lpfnWndProc = WndMainProc_;
    wincl.style = CS_DBLCLKS;
    wincl.cbSize = sizeof(WNDCLASSEX);

    wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hbrBackground = (HBRUSH)COLOR_WINDOW; /*(HBRUSH)COLOR_BACKGROUND;*/

    if(!RegisterClassEx(&wincl))
    {
        GlobalConfig->MyAddLog(_T("Error RegisterClassEx"));
        return 2;
    }

//    InitializeMYTreeWindowClass(MainInst);

    return 1;
}

int MRadioApp::CreateMainForm()
{
    InitCommonControls();

    GlobalConfig->LoadSettings();

    instance = this;

    HWND hwin = CreateWindowEx(
                    WS_EX_CONTROLPARENT, /* Extended possibilites for variation */
                    APP_CLASS,         /* Classname */
                    APP_TITLE,       /* Title Text */
                    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, /* default window */
                    GlobalConfig->winpos.wleft,       /* Windows decides the position */
                    GlobalConfig->winpos.wtop,       /* where the window ends up on the screen */
                    GlobalConfig->winpos.w_width,                 /* The programs width */
                    GlobalConfig->winpos.w_height,                 /* and height in pixels */
                    HWND_DESKTOP,        /* The window is a child-window to desktop */
                    NULL,                /* No menu */
                    MainInst,       /* Program Instance handler */
                    (LPVOID)this                 /* No Window Creation data */
                );

    if(!hwin)
    {
        GlobalConfig->MyAddLog(_T("Error CreateWindowEx"));
        UnregisterClass(APP_CLASS, MainInst);
        return 3;
    }

    MainFont->ApplyFont(hwin);

    return 1;
}

void MRadioApp::RunProgramm()
{
    while (GetMessage(&msg, (HWND)NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

int MRadioApp::GetMsgWParam()
{
    return msg.wParam;
}

void MRadioApp::GetSysInfo()
{
    GetMySystemInfo(&sysinfo);
}

void MRadioApp::SetWinPos2Set()
{
    RECT rcPos;
    GetWindowRect(hWin,&rcPos);
    GlobalConfig->winpos.wstate = GetMainFormWindowState();
    if(GlobalConfig->winpos.wstate != 1) // если окно минимилизировано то не меняем значения
    {
        GlobalConfig->winpos.wleft    = DWORD(rcPos.left);
        GlobalConfig->winpos.wtop     = DWORD(rcPos.top);
        GlobalConfig->winpos.w_height = rcPos.bottom-rcPos.top;
        GlobalConfig->winpos.w_width  = rcPos.right-rcPos.left;
    }
}

void MRadioApp::MainFormStayOnTop(bool fon)
{
    if(fon)
        SetWindowPos(hWin, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
    else
        SetWindowPos(hWin, HWND_NOTOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
}

int MRadioApp::GetMainFormWindowState()
{
    if(!hWin) return 0; // 1 - Minimized, 2 - Maximized, 0 - Normal
    if(IsIconic(hWin)) return 1; // Minimized
    if(IsZoomed(hWin)) return 2; // Maximized
    return 0;                    // Normal
}

void MRadioApp::SetMainFormWindowState(int statetype)
{
    if(!hWin || statetype == 1/*hack Minimized because winform found*/) return;
    int WindowStateShowCommands[3] = {SW_SHOWNOACTIVATE, SW_SHOWMINNOACTIVE, SW_SHOWMAXIMIZED};
    ShowWindow(hWin, WindowStateShowCommands[statetype]);
}

void MRadioApp::MainFormShow_Hide(bool fShow)
{
    if(fShow)
    {
        ShowWindow(hWin, SW_SHOW);
    }
    else
    {
        ShowWindow(hWin, SW_HIDE);
    }
}

void MRadioApp::CloseWindows()
{
    if(!hWin) return;

    delete(mae);

    SetWinPos2Set();
    GlobalConfig->SaveSettings();
    DestroyCursor(SplitCursor);
    //destroy panel controls
    PostMessage(hwndTV, LVM_DELETEALLITEMS, 0, 0);
    PostMessage(hwndRadioList, LVM_DELETEALLITEMS, 0, 0);
    DestroyFormControl();

    delete(MainFont);
    delete(MainDb);
    delete(init_winsock);
    DestroyWindow(hWin);
    GlobalConfig->MyAddLog(_T("CloseWindow"));
}

void MRadioApp::CreateStatusBar()
{
    hwndStatus = CreateWindowEx(
                     0,
                     STATUSCLASSNAME,
                     NULL,
                     SBARS_SIZEGRIP |
                     WS_CLIPSIBLINGS |
                     WS_CHILD | WS_VISIBLE,
                     0, 0, 0, 0,
                     hWin,
                     NULL,
                     MainInst,
                     NULL);
    if(hwndStatus)
    {
        MainFont->ApplyFont(hwndStatus);
        CalcStatusBarSize();
    }
}

void MRadioApp::CalcStatusBarSize()
{
    RECT rcPos;
    if(!hwndStatus) return;
    GetWindowRect(hwndStatus,&rcPos);
    int stbarwidth = rcPos.right-rcPos.left;
    int Buf[] = {int(stbarwidth*0.85), -1};
    SendMessage(hwndStatus, SB_SETPARTS, sizeof(Buf)/sizeof(int), (LPARAM)Buf);
}

int MRadioApp::GetStatusBarWidth()
{
    if(hwndStatus)
    {
        RECT rcStatus;
        GetWindowRect(hwndStatus, &rcStatus);
        return rcStatus.right - rcStatus.left;
    }
    else return 0;
}

int MRadioApp::GetStatusBarHeight()
{
    if(hwndStatus)
    {
        RECT rcStatus;
        GetWindowRect(hwndStatus, &rcStatus);
        return rcStatus.bottom - rcStatus.top;
    }
    else return 0;
}

bool MRadioApp::GetWindowSize(HWND win, int *rwidth, int *rheight)
{
    if(win)
    {
        RECT rcStatus;
        GetWindowRect(win, &rcStatus);
        if(rwidth)
            *rwidth  = rcStatus.right - rcStatus.left;
        if(rheight)
            *rheight = rcStatus.bottom - rcStatus.top;
        return true;
    }
    else
    {
        if(rwidth)
            *rwidth = 0;
        if(rheight)
            *rheight = 0;
        return false;
    };
}

int MRadioApp::GetStatusBarLeft()
{
    if(hwndStatus)
    {
        RECT rcStatus;
        GetWindowRect(hwndStatus, &rcStatus);
        return rcStatus.left;
    }
    else return 0;
}

int MRadioApp::GetStatusBarTop()
{
    if(hwndStatus)
    {
        RECT rcStatus;
        GetWindowRect(hwndStatus, &rcStatus);
        return rcStatus.top;
    }
    else return 0;
}


void MRadioApp::SetStatusBarTxt(wchar_t *val1, wchar_t *val2)
{
    SendMessage(hwndStatus, SB_SETTEXT, 0, (LPARAM)val1);
    SendMessage(hwndStatus, SB_SETTEXT, 1, (LPARAM)val2);
}

// TOOLBAR
void MRadioApp::CreateToolBar()
{
    //INITCOMMONCONTROLSEX init;
    //init.dwSize = sizeof(INITCOMMONCONTROLSEX);
    //init.dwICC = ICC_BAR_CLASSES;
    //InitCommonControlsEx(&init);
    hwndToolBar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
                                 WS_CHILD|WS_VISIBLE|WS_BORDER|TBSTYLE_TOOLTIPS|TBSTYLE_FLAT|CCS_NORESIZE|CCS_NOPARENTALIGN|CCS_NODIVIDER,
                                 //WS_CHILD | TBSTYLE_NOPREFIX | CCS_NOPARENTALIGN |
                                 //CCS_NODIVIDER | CCS_NORESIZE | TBSTYLE_TOOLTIPS,
                                 //TBSTYLE_TRANSPARENT | CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN
                                 R2NP(2),
                                 MainFormGetClientRect().bottom-(GetStatusBarHeight()+R2NP(PANEL_HEIGHT+1)),//MainFormGetClientRect().bottom-(GetStatusBarHeight()+R2NP(PANEL_HEIGHT+1)),
                                 MainFont->R2NP(TOOLBAR_WIDTH),
                                 MainFont->R2NP(PANEL_HEIGHT), //hWin
                                 hWin, (HMENU)MY_TOOLBAR_CONTROL_ID, MainInst, NULL);
    if(hwndToolBar)
    {
        MainFont->ApplyFont(hwndToolBar);
        tb_bmp = LoadBitmap(MainInst, MAKEINTRESOURCE(IDB_BITMAP1));

        tb_imagelist = ImageList_Create(14, 14,   // Dimensions of individual bitmaps.
                                        ILC_COLOR24 | ILC_MASK,   // Ensures transparent background.
                                        0, 0); // 1st 0 - num buttons
        ImageList_AddMasked(tb_imagelist, tb_bmp, RGB(255,0,255));
        ToolBarSetImageList(tb_imagelist);

        ToolBarAddButton(BTNS_BUTTON, TBSTATE_ENABLED, 0, NULL, _T("Воспроизвести / Пауза"), ID_TOOLBAR_PLAY_PAUSE);
        ToolBarAddButton(BTNS_BUTTON, TBSTATE_ENABLED, 1, NULL, _T("Предыдущее радио"), ID_TOOLBAR_PREV);
        ToolBarAddButton(BTNS_BUTTON, TBSTATE_ENABLED, 2, NULL, _T("Следующие радио"), ID_TOOLBAR_NEXT);
        ToolBarAddButton(BTNS_BUTTON, TBSTATE_ENABLED, 3, NULL, _T("Остановить"), ID_TOOLBAR_STOP);
        ToolBarAddButton(BTNS_BUTTON, 0, 4, NULL, _T("Запись"), ID_TOOLBAR_REC);
        ToolBarAddButton(BTNS_SEP, TBSTATE_ENABLED, -1, NULL, NULL, -1);
        ToolBarAddButton(BTNS_BUTTON, TBSTATE_ENABLED, 5, NULL, _T("Поиск"), ID_TOOLBAR_SEA);
        ToolBarAddButton(BTNS_BUTTON, TBSTATE_ENABLED, 6, NULL, _T("Обновить базу радиостанций"), ID_TOOLBAR_UPD);
        ToolBarAddButton(BTNS_BUTTON, TBSTATE_ENABLED, 7, NULL, _T("Настройки"), ID_TOOLBAR_SET);
        //SendMessage(hwndToolBar, TB_SETSTYLE, 0, TBSTYLE_FLAT);
        //SendMessage(hwndToolBar, TB_SETSTYLE, 0, SendMessage(hwndToolBar, TB_GETSTYLE, 0,0 ) & ~TBSTYLE_TRANSPARENT);
        //SendMessage ( hwndToolBar, TB_AUTOSIZE, 0, 0);

        ShowWindow(hwndToolBar, SW_SHOW);
    }
}

bool MRadioApp::ToolBarInsertButton(int index, DWORD style, DWORD state, int nimage,
                                    LPCWSTR text, LPCWSTR tip, UINT id)
{
    if(!hwndToolBar) return false;
    TBBUTTON btn;
    btn.fsState=BYTE(state);
    btn.fsStyle=BYTE(style);
    btn.iBitmap=nimage;
    btn.iString=INT_PTR(text);
    btn.idCommand=id;
    btn.dwData=DWORD_PTR(tip);
    if(!SendMessage(hwndToolBar,TB_INSERTBUTTON,WPARAM(index),LPARAM(&btn))) return false;
    return true;
}

bool MRadioApp::ToolBarAddButton(DWORD style, DWORD state, int nimage, LPCWSTR text, LPCWSTR tip, UINT id)
{
    return ToolBarInsertButton(-1,style,state,nimage,text,tip,id);
}

bool MRadioApp::ToolBarInsertSeparator(int index)
{
    return ToolBarInsertButton(index,BTNS_SEP,0,0,0,0,0);
}

bool MRadioApp::ToolBarAddSeparator()
{
    return ToolBarInsertSeparator(-1);
}

bool MRadioApp::ToolBarDeleteButton(int index)
{
    if(!hwndToolBar) return false;
    if(!SendMessage(hwndToolBar,TB_DELETEBUTTON,WPARAM(index),0)) return false;
    return true;
}

bool MRadioApp::ToolBarMoveButton(int index,int new_index)
{
    if(!hwndToolBar) return false;
    if(!SendMessage(hwndToolBar,TB_MOVEBUTTON,WPARAM(index),LPARAM(index))) return false;
    return true;
}

bool MRadioApp::ToolBarSetButtonState(int id,DWORD state)
{
    if(!hwndToolBar) return false;
    if(!SendMessage(hwndToolBar,TB_SETSTATE,WPARAM(id),LPARAM(MAKELONG(state,0)))) return false;
    return true;
}

DWORD MRadioApp::ToolBarGetButtonState(int id)
{
    if(!hwndToolBar) return 0;
    return SendMessage(hwndToolBar,TB_GETSTATE,WPARAM(id),0);
}

bool MRadioApp::ToolBarEnableButton(int id, bool enable)
{
    if(!hwndToolBar) return false;
    if(!SendMessage(hwndToolBar,TB_ENABLEBUTTON,WPARAM(id),LPARAM(enable))) return false;
    return true;
}

bool MRadioApp::ToolBarSetImageList(HIMAGELIST& hlist)
{
    if(!hlist && !hwndToolBar) return false;
    if(!SendMessage(hwndToolBar,TB_SETIMAGELIST,WPARAM(0),LPARAM(hlist))) return false;
    return true;
}

int MRadioApp::ToolBarGetButtonCount()
{
    if(!hwndToolBar) return 0;
    return SendMessage(hwndToolBar, TB_BUTTONCOUNT, 0, 0);
}

TBBUTTON MRadioApp::ToolBarGetButton(int index)
{
    TBBUTTON btn;
    if(!hwndToolBar)
    {
        btn.iString=0;
        btn.idCommand=-1;
        return btn;
    }
    SendMessage(hwndToolBar,TB_GETBUTTON,WPARAM(index),LPARAM(&btn));
    return btn;
}
//

DWORD MRadioApp::TreeViewGetCount(HWND vhwnd)
{
    //(tp == type_tv)?hwndTV:hwndTVRlist
    return TreeView_GetCount(vhwnd);
}

HTREEITEM MRadioApp::TreeViewInsertItem(HWND vhwnd, LPCWSTR text, int image, LPARAM param,
                                        HTREEITEM parent, HTREEITEM insert_after)
{
    TVITEM tvi = {0};
    tvi.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.pszText = const_cast<LPWSTR>(text);
    tvi.lParam = param;

    if(image > -1)
    {
        tvi.mask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        tvi.iImage = image;
        tvi.iSelectedImage = image;
    }

    TVINSERTSTRUCT tvis = {0};
    tvis.item = tvi;
    tvis.hInsertAfter = insert_after;
    tvis.hParent = parent;

    return TreeView_InsertItem(vhwnd, &tvis);
}

void MRadioApp::TreeViewSetItemData(HWND vhwnd, HTREEITEM item, LPARAM param)
{
    TVITEM tvi = {0};
    tvi.mask = TVIF_HANDLE | TVIF_PARAM;
    tvi.hItem = item;
    tvi.lParam = param;
    TreeView_SetItem(vhwnd, &tvi);
}

LPARAM MRadioApp::TreeViewGetItemData(HWND vhwnd, HTREEITEM item)
{
    TVITEM tvi = {0};
    tvi.mask = TVIF_PARAM;
    tvi.hItem = item;
    TreeView_GetItem(vhwnd, &tvi);

    return tvi.lParam;
}

bool MRadioApp::TreeViewItemDelete(HWND vhwnd, HTREEITEM item)
{
    return TreeView_DeleteItem(vhwnd, item);
}

bool MRadioApp::TreeViewDeleteAllItems(HWND vhwnd)
{
    return TreeView_DeleteAllItems(vhwnd);
}

bool MRadioApp::TreeViewExpand(HWND vhwnd, HTREEITEM item, DWORD flg)
{
    return TreeView_Expand(vhwnd, item, flg);
}
//
int MRadioApp::R2NP(int v)
{
    return MainFont->RelativeToNormalPixels2(v);
}
#define LVS_EX_AUTOSIZECOLUMNS 0x10000000
void MRadioApp::CreateTreeView()
{
    hwndTV = CreateWindowEx(0,
                            WC_LISTVIEW,
                            NULL,
                            WS_VISIBLE| WS_TABSTOP | WS_CHILD | WS_CLIPSIBLINGS |
                            WS_BORDER |  LVS_OWNERDATA | LVS_NOSORTHEADER | LVS_SINGLESEL |
                            LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_OWNERDRAWFIXED,
                            R2NP(2),
                            R2NP(2),
                            MainFormGetClientRect().right*0.5,
                            MainFormGetClientRect().bottom-(GetStatusBarHeight()+R2NP(PANEL_HEIGHT)+R2NP(4)),
                            hWin,
                            (HMENU)MY_TREEVIEW_CONTROL_ID,
                            MainInst,
                            NULL);
    if(hwndTV)
    {
        MainFont->ApplyFont(hwndTV);

        DWORD style =  LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
        if(sysinfo.version >= V_WINDOWS_VISTA)
            style = style | LVS_EX_AUTOSIZECOLUMNS;
        ListView_SetExtendedListViewStyleEx(hwndTV, style, style);

        LVCOLUMN listViewColumn;
        listViewColumn.mask		= LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        listViewColumn.iSubItem = 0;
        listViewColumn.pszText	= NULL;
        listViewColumn.cx		= 100;
        ListView_InsertColumn(hwndTV, 0, &listViewColumn);
    }
}

int MRadioApp::GetTreeViewWidth()
{
    if(!hwndTV) return 0;
    RECT rect;
    GetClientRect(hwndTV, &rect);
    return rect.right - rect.left;
}

int MRadioApp::GetTreeViewHeight()
{
    if(!hwndTV) return 0;
    RECT rect;
    GetClientRect(hwndTV, &rect);
    return rect.bottom - rect.top;
}

void MRadioApp::UpDateTVColumSize()
{
    int scrollwidth = 0;
    if((GetWindowLong(hwndTV, GWL_STYLE) & WS_VSCROLL) != 0)
        scrollwidth = GetSystemMetrics(SM_CYHSCROLL);
    else scrollwidth = 0;
    ListView_SetColumnWidth(hwndTV, 0, GetSplitPos()-scrollwidth);
}
//
void MRadioApp::CreateListViewRadioList()
{
    hwndRadioList = CreateWindowEx(0,
                                   WC_LISTVIEW,
                                   NULL,
                                   WS_VISIBLE| WS_TABSTOP | WS_CHILD | WS_CLIPSIBLINGS |
                                   WS_BORDER |  LVS_OWNERDATA | LVS_NOSORTHEADER | LVS_SINGLESEL |
                                   LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_OWNERDRAWFIXED,
                                   GetTreeViewWidth()+R2NP(6),
                                   R2NP(2),
                                   MainFormGetClientRect().right*0.5,
                                   MainFormGetClientRect().bottom-(GetStatusBarHeight()+R2NP(PANEL_HEIGHT)+R2NP(4)),
                                   hWin,
                                   (HMENU)MY_LISTVIEW_CONTROL_ID,
                                   MainInst,
                                   NULL);
    if(hwndRadioList)
    {
        MainFont->ApplyFont(hwndRadioList);

        DWORD style =  LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER;
        if(sysinfo.version >= V_WINDOWS_VISTA)
            style = style | LVS_EX_AUTOSIZECOLUMNS;
        ListView_SetExtendedListViewStyleEx(hwndRadioList, style, style);

        LVCOLUMN listViewColumn;
        listViewColumn.mask		= LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        listViewColumn.iSubItem = 0;
        listViewColumn.pszText	= NULL;
        listViewColumn.cx		= MainFormGetClientRect().right*0.5;
        ListView_InsertColumn(hwndRadioList, 0, &listViewColumn);
    }

    /*if(hwndRadioList)
    {

        SetWindowLong(hwndRadioList, GWL_USERDATA, LONG(this));
        _PrevControlRadioListProc = WNDPROC(SetWindowLong(hwndRadioList, GWL_WNDPROC, LONG(WndControlRadioListProc_)));
        SetWindowLong(hwndRadioList, GWL_WNDPROC, LONG(WndControlRadioListProc_));
    }*/
}

void MRadioApp::NewDateInBase()
{
    PostMessage(hwndTV, LVM_SETITEMCOUNT, 0, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
    list_end_idx[2] = 0;
    list_end_idx[3] = 0;
    list_end_idx[0] = 0;
    list_end_idx[1] = 0;
    list_genre.clear();
    list_bitrate.clear();
    cur_idx_item = -1;
    GetCountListRadio();
    CreateList2TV();

    PostMessage(hwndRadioList, LVM_SETITEMCOUNT, 0, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
    list_radio_size = 0;
    if(list_radio.size() > 0)
        list_radio.clear();
}

void MRadioApp::CreateList2TV()
{
    int tlsize = 4;
    list_end_idx[0] = 0;
    list_end_idx[1] = 0;
    if(MainDbRadioCount > 0)
    {

        if(list_genre.size() == 0)
        {
            try
            {
                MainDb->BeginTransaction();
                Statement i = MainDb->Query(_T("SELECT COUNT(*) FROM TableGenres"));
                i.Next();
                int count_genre = (int)i.GetField(0)-1;

                if(count_genre)
                {
                    list_genre.assign(count_genre, TreeViewItem());
                    list_genre_idx.clear();
                    list_genre_idx.assign(count_genre, 0);
                    Statement i2 = MainDb->Query(_T("SELECT ID,Value FROM TableGenres ORDER BY Value"));
                    int c=0;
                    while(i2.FetchRow())
                    {
                        int idx = (int)i2.GetField(0);
                        if(idx != 0)
                        {
                            list_genre.at(c).int_data = idx;
                            list_genre.at(c).SetText((const wchar_t*)i2.GetField(1));
                            list_genre_idx.at(idx-1) = c;
                            c++;
                        }
                    }
                }
                MainDb->CommitTransaction();
            }
            catch(const sqdb::Exception& e)
            {
                MainDb->RollbackTransaction();
                list_genre.clear();
                list_genre_idx.clear();
            }
        }

        if(list_end_idx[2])
        {
            tlsize = tlsize + list_genre.size();
            list_end_idx[0] = 2 + list_genre.size();
        }
        else
        {
            list_end_idx[0] = 2;
        }

        if(list_bitrate.size() == 0)
        {
            try
            {
                MainDb->BeginTransaction();

                Statement i = MainDb->Query(_T("SELECT COUNT(*) FROM TableBitRate"));
                i.Next();
                int count_bitrate = (int)i.GetField(0);
                if(count_bitrate)
                {
                    list_bitrate.assign(count_bitrate, TreeViewItem());
                    list_bitrate_idx.clear();
                    list_bitrate_idx.assign(count_bitrate, 0);
                    Statement i2 = MainDb->Query(_T("SELECT ID,Value FROM TableBitRate ORDER BY Value"));
                    int c=0;
                    while(i2.FetchRow())
                    {
                        int idx = (int)i2.GetField(0);
                        list_bitrate.at(c).int_data = idx;
                        list_bitrate.at(c).SetText((const wchar_t*)i2.GetField(1));
                        list_bitrate_idx.at(idx) = c;
                        c++;
                    }
                }
                MainDb->CommitTransaction();
            }
            catch(const sqdb::Exception& e)
            {
                MainDb->RollbackTransaction();
                list_bitrate.clear();
                list_bitrate_idx.clear();
            }
        }

        if(list_end_idx[3])
        {
            tlsize = tlsize + list_bitrate.size();
            list_end_idx[1] = list_end_idx[0] + 1 + list_bitrate.size();
        }
        else
        {
            list_end_idx[1] = list_end_idx[0] + 1;
        }

        //PostMessage(hwndTV, LVM_SETITEMCOUNT, tlsize, !repaint ? LVSICF_NOINVALIDATEALL : 0 | LVSICF_NOSCROLL);
        PostMessage(hwndTV, LVM_SETITEMCOUNT, tlsize, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
    }
    else
    {
        PostMessage(hwndTV, LVM_SETITEMCOUNT, 0, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
    }
}

void MRadioApp::GetFavStation()
{
    PostMessage(hwndRadioList, LVM_SETITEMCOUNT, 0, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
    if(list_radio.size() > 0)
        list_radio.clear();
    list_radio_size = 0;

    MYQueryStr str;
    str.Format(_T("%i"), list_radio_size);
    SetStatusBarTxt(NULL, (wchar_t*)str.Get());
}

void MRadioApp::GetAllStation()
{
    if(!MainDbRadioCount) return;
    PostMessage(hwndRadioList, LVM_SETITEMCOUNT, 0, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
    if(list_radio.size() > 0)
        list_radio.clear();

    {
        list_radio_size = 0;
        Statement i = MainDb->Query(_T("SELECT COUNT(*) FROM TableRadios"));
        i.Next();
        list_radio_size = (int)i.GetField(0);
    }

    if(list_radio_size)
    {
        list_radio.assign(list_radio_size, RadioStationInfo());
        try
        {
            MainDb->BeginTransaction();
            Statement i = MainDb->Query(_T("SELECT ID,RadioName,RadioURL,BitRateIdx,GenreIdx FROM TableRadios ORDER BY RadioName"));
            int c = 0;
            while(i.FetchRow())
            {
                list_radio.at(c).SetRadioNum(c+1);
                list_radio.at(c).SetRadioID(i.GetField(0));
                list_radio.at(c).SetRadioName((const wchar_t*)i.GetField(1));
                list_radio.at(c).SetRadioUrl((const wchar_t*)i.GetField(2));
                list_radio.at(c).SetRadioBaseBitRate((int)i.GetField(3));
                list_radio.at(c).SetRadioBaseGenre((const wchar_t*)i.GetField(4));
                c++;
            }
            MainDb->CommitTransaction();
        }
        catch(const sqdb::Exception& e)
        {
            MainDb->RollbackTransaction();
        }

        PostMessage(hwndRadioList, LVM_SETITEMCOUNT, list_radio_size, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
        MYQueryStr str;
        str.Format(_T("%i"), list_radio_size);
        SetStatusBarTxt(NULL, (wchar_t*)str.Get());
    }
}

void MRadioApp::GetStationByGenre(int genre_idx)
{
    if(!MainDbRadioCount) return;
    PostMessage(hwndRadioList, LVM_SETITEMCOUNT, 0, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
    if(list_radio.size() > 0)
        list_radio.clear();

    {
        list_radio_size = 0;
        MYQueryStr str;
        Statement i = MainDb->Query(str.Format(_T("SELECT COUNT(*) FROM TableRadios WHERE (GenreIdx LIKE '%i' OR GenreIdx LIKE '%%%;%i;%%%' OR  GenreIdx LIKE '%%%;%i' OR GenreIdx LIKE '%i;%%%')"), genre_idx, genre_idx, genre_idx, genre_idx));
        i.Next();
        list_radio_size = (int)i.GetField(0);
    }

    if(list_radio_size)
    {
        list_radio.assign(list_radio_size, RadioStationInfo());
        try
        {
            MainDb->BeginTransaction();
            MYQueryStr str;
            Statement i = MainDb->Query(str.Format(_T("SELECT ID,RadioName,RadioURL,BitRateIdx,GenreIdx FROM TableRadios WHERE (GenreIdx LIKE '%i' OR GenreIdx LIKE '%%%;%i;%%%' OR  GenreIdx LIKE '%%%;%i' OR GenreIdx LIKE '%i;%%%') ORDER BY RadioName"), genre_idx, genre_idx, genre_idx, genre_idx));
            int c = 0;
            while(i.FetchRow())
            {
                list_radio.at(c).SetRadioNum(c+1);
                list_radio.at(c).SetRadioID(i.GetField(0));
                list_radio.at(c).SetRadioName((const wchar_t*)i.GetField(1));
                list_radio.at(c).SetRadioUrl((const wchar_t*)i.GetField(2));
                list_radio.at(c).SetRadioBaseBitRate((int)i.GetField(3));
                list_radio.at(c).SetRadioBaseGenre((const wchar_t*)i.GetField(4));
                c++;
            }
            MainDb->CommitTransaction();
        }
        catch(const sqdb::Exception& e)
        {
            MainDb->RollbackTransaction();
        }

        PostMessage(hwndRadioList, LVM_SETITEMCOUNT, list_radio_size, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
        MYQueryStr str;
        str.Format(_T("%i"), list_radio_size);
        SetStatusBarTxt(NULL, (wchar_t*)str.Get());
    }
}

void MRadioApp::GetStationByBitRate(int br_idx)
{
    if(!MainDbRadioCount) return;
    PostMessage(hwndRadioList, LVM_SETITEMCOUNT, 0, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
    if(list_radio.size() > 0)
        list_radio.clear();

    {
        list_radio_size = 0;
        MYQueryStr str;
        Statement i = MainDb->Query(str.Format(_T("SELECT COUNT(*) FROM TableRadios WHERE BitRateIdx = '%i'"), br_idx));
        i.Next();
        list_radio_size = (int)i.GetField(0);
    }

    if(list_radio_size)
    {
        list_radio.assign(list_radio_size, RadioStationInfo());
        try
        {
            MainDb->BeginTransaction();
            MYQueryStr str;
            Statement i = MainDb->Query(str.Format(_T("SELECT ID,RadioName,RadioURL,BitRateIdx,GenreIdx FROM TableRadios WHERE BitRateIdx = '%i' ORDER BY RadioName"), br_idx));
            int c = 0;
            while(i.FetchRow())
            {
                list_radio.at(c).SetRadioNum(c+1);
                list_radio.at(c).SetRadioID(i.GetField(0));
                list_radio.at(c).SetRadioName((const wchar_t*)i.GetField(1));
                list_radio.at(c).SetRadioUrl((const wchar_t*)i.GetField(2));
                list_radio.at(c).SetRadioBaseBitRate((int)i.GetField(3));
                list_radio.at(c).SetRadioBaseGenre((const wchar_t*)i.GetField(4));
                c++;
            }
            MainDb->CommitTransaction();
        }
        catch(const sqdb::Exception& e)
        {
            MainDb->RollbackTransaction();
        }

        PostMessage(hwndRadioList, LVM_SETITEMCOUNT, list_radio_size, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
        MYQueryStr str;
        str.Format(_T("%i"), list_radio_size);
        SetStatusBarTxt(NULL, (wchar_t*)str.Get());
    }
}

void MRadioApp::GetCountListRadio(bool DbExist)
{
    if(DbExist)
        MainDbDataExists = true;
    if(MainDbDataExists)
    {
        Statement i = MainDb->Query(_T("SELECT * FROM BaseInfo"));
        i.Next();
        MainDbRadioCount = (int)i.GetField(1);
        current_base_info.radio_count   = MainDbRadioCount;
        current_base_info.base_ver      = (int)i.GetField(0);
        Time642SysTime((long long)i.GetField(2), &current_base_info.time_create);
        Time642SysTime((long long)i.GetField(3), &current_base_info.time_modif);
    }
    else
    {
        MainDbRadioCount = 0;
        memset(&current_base_info, 0, sizeof(base_info));
    }
    //TESTMESS(LVState->GetField(2));
}

/*if(LPNMHDR(lParam)->idFrom == MY_TOOLBAR_CONTROL_ID)
{

}
if(LPNMHDR(lParam)->code == TTN_GETDISPINFO)
{
    MessageBox(0, _T("TEST"), NULL, 0);
}
/* if(LPNMHDR(lParam)->code == TTN_GETDISPINFO)
 {

     LPTOOLTIPTEXT tip = LPTOOLTIPTEXT(lParam);
     tip->hinst = MainInst;
     int id = tip->hdr.idFrom;
     int tbc = ToolBarGetButtonCount();
     for(int i=0; i<tbc; i++)
     {
         TBBUTTON btn = ToolBarGetButton(i);
         if(btn.idCommand==id)
         {
             tip->lpszText = LPWSTR(btn.dwData);
             break;
         }
     }
 }*/

//
void MRadioApp::CreatePanel()
{
    hwndPanel = CreateWindowEx(0,
                               WC_STATIC,
                               NULL,
                               WS_VISIBLE | WS_BORDER | WS_CHILD | WS_TABSTOP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,// | SS_NOTIFY,
                               //SS_LEFTNOWORDWRAP | SS_NOPREFIX,
                               R2NP(TOOLBAR_WIDTH + 3),
                               MainFormGetClientRect().bottom-(GetStatusBarHeight()+R2NP(PANEL_HEIGHT+1)),
                               GetStatusBarWidth()-R2NP(TOOLBAR_WIDTH+5),
                               R2NP(PANEL_HEIGHT),
                               hWin,
                               (HMENU)MY_PANEL_CONTROL_ID,
                               MainInst,
                               NULL);
    if(hwndPanel)
    {
        MainFont->ApplyFont(hwndPanel);
    }
}
//
void MRadioApp::DestroyFormControl()
{
    if(hwndTV) DestroyWindow(hwndTV);
    if(hwndRadioList) DestroyWindow(hwndRadioList);
    if(hwndToolBar) DestroyWindow(hwndToolBar);

    if(hwndPanel) DestroyWindow(hwndPanel);
    if(hwndStatus) DestroyWindow(hwndStatus);
    DeleteObject(tb_bmp);
    ImageList_Destroy(tb_imagelist);
}
//
DWORD MRadioApp::GetSplitPos()
{
    if(!GlobalConfig) return 0;
    return GlobalConfig->winpos.wsp_pos;
}

void MRadioApp::SetSplitPos(DWORD val)
{
    if(!GlobalConfig) return;
    if(val < 0) return;
    if(val > DWORD(MainFormGetClientRect().right-10)) return;
    GlobalConfig->winpos.wsp_pos = val;
}
//
LRESULT WINAPI MRadioApp::WndMainProc_(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    MRadioApp *_this = (MRadioApp*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if((NULL == _this) && (message != WM_NCCREATE))
    {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    switch(message)
    {
    case WM_NCCREATE:
    {
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
        _this = (MRadioApp*)(lpcs->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)_this);
        _this->hWin = hWnd;
    }
    break;
    case WM_CREATE:
        return _this->WM_CreateForm();
    case WM_CLOSE:
        return _this->WM_CLOSE_(wParam, lParam);
    case WM_QUERYENDSESSION:
        return _this->WM_QUERYENDSESSION_(wParam, lParam);
    case WM_SIZE:
        return _this->WM_SIZE_(wParam, lParam);
    case WM_MOUSEMOVE:
        return _this->WM_MOUSEMOVE_(wParam, lParam);
    case WM_LBUTTONDOWN:
        return _this->WM_LBUTTONDOWN_(wParam, lParam);
    case WM_LBUTTONUP:
        return _this->WM_LBUTTONUP_(wParam, lParam);
    case WM_NOTIFY:
        return _this->WM_NOTIFY_(wParam, lParam);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_DRAWITEM:
        return _this->WM_DRAWITEM_(wParam, lParam);
    case WM_MEASUREITEM:
        return _this->WM_MEASUREITEM_(wParam, lParam);
    case WM_COMMAND:
        return _this->WM_COMMAND_(wParam, lParam);
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT MRadioApp::WM_CreateForm()
{
    GlobalConfig->MyAddLog(_T("CreateWindow"));

    MainFont        = new MyFont(hWin);
    init_winsock    = new InitWinSock();

    sqlite3_file.append(ConfigDir);
    sqlite3_file+=_T("\\myradio_base.db");
    MainDb = new Db(sqlite3_file.c_str());
    MainDbDataExists = MainDb->TableExists(_T("BaseInfo"));
    GlobalConfig->MyAddLogParam(_T("Db data exists? = %i"), (int)MainDbDataExists);

    lv_format_str1 = _T("%num%. $if2(%title%,%url%)"); //
    lv_format_str2 = _T("%url% :: %genre% :: %bitrate% :: %id%");

    //for(int Idx = 0; Idx < 10; Idx++)
    // {
    //     MYQueryStr str;
    //     str.Format(_T("%i"), Idx);
    //     TESTMESS(str.Get());
    // }

    /*

    %tagname% - Inserts field named 'tagname' eg %artist%, %title%, %album%, %filename% etc.;
                Also takes ID3v2 frames eg 'TCOP', ID3v2 'TXXX' titles and APEv2 field names.;
    $if(A,B,C) - If A contains a valid field then display B, else display C;
    $if2(A,B) - If A contains a valid field then display A, else display B;
    $up
    $lo
    $caps
    [...] - Displays the contents of the brackets only if it contains a valid field;
    '...' - Текст в кавычках вставляется без изменений,


    */

    CreateStatusBar();
    CreatePanel();
    CreateTreeView();
    CreateListViewRadioList();
    CreateToolBar();

    list_radio_size = 0;

    NewDateInBase();

    SplitCursor = LoadCursor(NULL, IDC_SIZEWE);
    bSplitterMoving = false;

    MainFormShow_Hide(true);
    UpdateWindow(hWin);
    SetForegroundWindow(hWin);
    SetFocus(hWin);
    SetMainFormWindowState(GlobalConfig->winpos.wstate);

    mae = new MyAudioEngine(&sysinfo);
    mae->InitOutputDevice();

    return 0;
}

LRESULT MRadioApp::WM_CLOSE_(WPARAM wParam, LPARAM lParam)
{
    CloseWindows();
    return 0;
}

LRESULT MRadioApp::WM_QUERYENDSESSION_(WPARAM wParam, LPARAM lParam)
{
    CloseWindows();
    return 1;
}

LRESULT MRadioApp::WM_SIZE_(WPARAM wParam, LPARAM lParam)
{
    //if ((wParam != SIZE_MINIMIZED) && (HIWORD(lParam) < GetSplitPos())
    //    dwSplitterPos = HIWORD(lParam) - 10;
    /**
    _In_ int  X, left
    _In_ int  Y, top
    _In_ int  nWidth,
    _In_ int  nHeight,
    _In_ BOOL bRepaint
     **/
    if(!bSplitterMoving)
        if((GetSplitPos())>=DWORD(LOWORD(lParam)-R2NP(10))) SetSplitPos(LOWORD(lParam)-R2NP(15));

    SendMessage(hwndStatus, WM_SIZE, 0, 0);
    CalcStatusBarSize();

    MoveWindow(hwndPanel, R2NP(TOOLBAR_WIDTH+3), HIWORD(lParam)-(GetStatusBarHeight()+R2NP(PANEL_HEIGHT+1)), GetStatusBarWidth()-R2NP(TOOLBAR_WIDTH+5), R2NP(PANEL_HEIGHT), TRUE);
    MoveWindow(hwndToolBar, R2NP(2), HIWORD(lParam)-(GetStatusBarHeight()+R2NP(PANEL_HEIGHT+1)), R2NP(TOOLBAR_WIDTH), R2NP(PANEL_HEIGHT), TRUE);

    MoveWindow(hwndTV, R2NP(2), R2NP(2), GetSplitPos()+R2NP(2),
               HIWORD(lParam)-(GetStatusBarHeight()+R2NP(PANEL_HEIGHT)+R2NP(4)), TRUE);
    MoveWindow(hwndRadioList, GetSplitPos()+R2NP(6), R2NP(2), LOWORD(lParam)-GetSplitPos()-R2NP(8),
               HIWORD(lParam)-(GetStatusBarHeight()+R2NP(PANEL_HEIGHT)+R2NP(4)), TRUE);

    int scrollwidth = 0;
    if((GetWindowLong(hwndRadioList, GWL_STYLE) & WS_VSCROLL) != 0)
        scrollwidth = GetSystemMetrics(SM_CYHSCROLL);
    else scrollwidth = 0;
    ListView_SetColumnWidth(hwndRadioList, 0, LOWORD(lParam)-GetSplitPos()-R2NP(8)-scrollwidth);

    UpDateTVColumSize();
    return 0;
}

LRESULT MRadioApp::WM_MOUSEMOVE_(WPARAM wParam, LPARAM lParam)
{
    DWORD xPos;
    DWORD yPos;
    xPos = (DWORD)LOWORD(lParam);
    yPos = (DWORD)HIWORD(lParam);

    if((wParam == MK_LBUTTON) && bSplitterMoving)
    {
        SetCursor(SplitCursor);
        RECT rect = MainFormGetClientRect();
        SetSplitPos(xPos);
        SendMessage(hWin, WM_SIZE, 0, MAKELPARAM(rect.right, rect.bottom));
    }
    else if((xPos >= GetSplitPos()+R2NP(4)) && (xPos <= GetSplitPos()+R2NP(6))
            && (yPos >= DWORD(R2NP(2))) && (yPos <= DWORD(MainFormGetClientRect().bottom-((DWORD)GetStatusBarHeight()+R2NP(PANEL_HEIGHT)+R2NP(2)))))
    {
        SetCursor(SplitCursor);
    }

    return 0;
}

LRESULT MRadioApp::WM_LBUTTONDOWN_(WPARAM wParam, LPARAM lParam)
{
    DWORD xPos;
    DWORD yPos;
    xPos = (DWORD)LOWORD(lParam);
    yPos = (DWORD)HIWORD(lParam);

    if((xPos >= GetSplitPos()+R2NP(4)) && (xPos <= GetSplitPos()+R2NP(6)) && (yPos >= DWORD(R2NP(2))) && (yPos <= ((DWORD)MainFormGetClientRect().bottom-((DWORD)GetStatusBarHeight()+R2NP(PANEL_HEIGHT)+R2NP(2)))))
    {
        SetCursor(SplitCursor);
        bSplitterMoving = true;
        SetCapture(hWin);
    }
    return 0;
}

LRESULT MRadioApp::WM_LBUTTONUP_(WPARAM wParam, LPARAM lParam)
{
    ReleaseCapture();
    bSplitterMoving = false;
    return 0;
}

LRESULT MRadioApp::WM_COMMAND_(WPARAM wParam, LPARAM lParam)
{
    if(lParam == (LPARAM)hwndToolBar)
        switch(LOWORD(wParam))
        {

        case ID_TOOLBAR_PLAY_PAUSE:
        {
            MyString str(_T("http://78.129.251.122:8599/stream"));
            mae->OpenURL(str);
        }
        break;

        case ID_TOOLBAR_UPD:
        {
            AddRadioClass *upd_base;
            upd_base = new AddRadioClass(this);

            upd_base->ShowAddRadioDialog();

            if(upd_base->UpdateBaseOk())
                NewDateInBase();

            delete upd_base;
        }
        break;

        case ID_TOOLBAR_SET:
        {
            ConfigDialog *conf_d;
            conf_d = new ConfigDialog(this);

            conf_d->Show();

            delete conf_d;
        }
        break;

        case ID_TOOLBAR_PREV:
            break;

        }
    return 0;
}

LRESULT MRadioApp::WM_DRAWITEM_(WPARAM wParam, LPARAM lParam)
{
    DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT*)lParam;

    if(dis->CtlType == ODT_LISTVIEW)
        switch(dis->CtlID)
        {
        case MY_LISTVIEW_CONTROL_ID:
        {
            if (dis->itemID % 2)
            {
                HBRUSH color = CreateSolidBrush((COLORREF)RGB(0xF7, 0xF7, 0xF7));
                FillRect(dis->hDC, &dis->rcItem, color);
                DeleteObject(color);
            }

            bool selected = false;
            if(dis->itemState & (ODS_FOCUS || ODS_SELECTED))
            {
                HBRUSH color = CreateSolidBrush((COLORREF)GetSysColor(COLOR_HIGHLIGHT));
                FillRect(dis->hDC, &dis->rcItem, color);
                DeleteObject(color);
                selected = true;
            }

            LVCOLUMN lvc = {0};
            lvc.mask = LVCF_WIDTH;
            SendMessage(dis->hwndItem, LVM_GETCOLUMN, 0, (LPARAM)&lvc);

            RECT last_rc;
            //int last_left = 0;

            last_rc = dis->rcItem;

            last_rc.left = 5;
            last_rc.right = lvc.cx - 5;

            int width = last_rc.right - last_rc.left;
            int height = last_rc.bottom - last_rc.top;

            // Normal text position.
            RECT rc;
            rc.top = 0;
            rc.left = 0;
            rc.right = width;
            rc.bottom = height;

            HDC hdcMem = CreateCompatibleDC(dis->hDC);
            HBITMAP hbm = CreateCompatibleBitmap(dis->hDC, width, height);
            HBITMAP ohbm = (HBITMAP)SelectObject(hdcMem, hbm);
            DeleteObject(ohbm);
            DeleteObject(hbm);
            HFONT ohf = (HFONT)SelectObject(hdcMem, MainFont->GetHWNDFont());// MainFont->GetHWNDFont());
            DeleteObject(ohf);

            SetBkMode(hdcMem, TRANSPARENT);

            int idx = dis->itemID;

            if(list_radio.at(idx).RadioGenreEmpty())
                list_radio.at(idx).UpDateRadioGenre(&list_genre_idx, &list_genre);
            if(list_radio.at(idx).RadioBitRateEmpty())
                list_radio.at(idx).UpDateRadioBitRate(&list_bitrate_idx, &list_bitrate);
            if(selected == true)
            {
                HBRUSH color = CreateSolidBrush((COLORREF)GetSysColor(COLOR_HIGHLIGHT));
                FillRect(hdcMem, &rc, color);
                DeleteObject(color);

                SetTextColor(hdcMem, RGB(0xFF, 0xFF, 0xFF));
                rc.top = 3;
                DrawText(hdcMem, list_radio.at(idx).GetFormatString1(lv_format_str1), -1, &rc, DT_NOPREFIX | DT_SINGLELINE | DT_TOP | DT_END_ELLIPSIS);
                rc.top = 14;
                SetTextColor(hdcMem, RGB(0xFF, 0xFF, 0xFF));
                DrawText(hdcMem, list_radio.at(idx).GetFormatString2(lv_format_str2), -1, &rc, DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

                BitBlt(dis->hDC, dis->rcItem.left + last_rc.left, last_rc.top, width, height, hdcMem, 0, 0, SRCCOPY);
            }
            else	// Draw normal text.
            {
                HBRUSH color = CreateSolidBrush((COLORREF)GetSysColor(COLOR_WINDOW));
                FillRect(hdcMem, &rc, color);
                DeleteObject(color);

                //rc.top = ;
                SetTextColor(hdcMem, RGB(0x00, 0x80, 0x00));
                rc.top = 3;
                DrawText(hdcMem, list_radio.at(idx).GetFormatString1(lv_format_str1), -1, &rc, DT_NOPREFIX | DT_SINGLELINE | DT_TOP | DT_END_ELLIPSIS);
                rc.top = 14;
                SetTextColor(hdcMem, RGB(0x00, 0x00, 0x00));
                DrawText(hdcMem, list_radio.at(idx).GetFormatString2(lv_format_str2), -1, &rc, DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

                BitBlt(dis->hDC, dis->rcItem.left + last_rc.left, last_rc.top, width, height, hdcMem, 0, 0, SRCAND);
            }

            DeleteDC(hdcMem);
            // Save the last left position of our column.
            //last_left += lvc.cx;
            return 1;
        }

        case MY_TREEVIEW_CONTROL_ID:
        {
            bool selected    = false;
            bool current     = false;
            int idx = dis->itemID;

            if(dis->itemState & (ODS_FOCUS || ODS_SELECTED))
            {
                HBRUSH color = CreateSolidBrush((COLORREF)GetSysColor(COLOR_HIGHLIGHT));
                FillRect(dis->hDC, &dis->rcItem, color);
                DeleteObject(color);
                selected = true;
            }
            else if(cur_idx_item == idx)
            {
                if(cur_idx_item >= 0 && cur_idx_item <= 1)
                {
                    current = true;
                }
                else if(cur_idx_item >= 2 && cur_idx_item <= (2 + list_genre.size()))
                {
                    if(list_end_idx[2])
                        current = true;
                    else current = false;
                    MYQueryStr str;
                    str.Format(_T("cur = %i, list_end_idx[0] = %i"), cur_idx_item, list_end_idx[0]);
                    SetStatusBarTxt((wchar_t*)str.Get(), NULL);
                }
                else if((cur_idx_item >= (3 + list_genre.size())) && (cur_idx_item <= (3 + list_genre.size() + list_bitrate.size())))
                {
                    if(list_end_idx[3])
                        current = true;
                    else current = false;
                    MYQueryStr str;
                    str.Format(_T("cur = %i, list_end_idx[0]+1 = %i, list_end_idx[1] = %i"), cur_idx_item, list_end_idx[0]+1, list_end_idx[1]);
                    SetStatusBarTxt((wchar_t*)str.Get(), NULL);
                }
            }

            if (current == true)
            {
                HBRUSH color = CreateSolidBrush((COLORREF)RGB(0x80, 0x80, 0x80));
                FillRect(dis->hDC, &dis->rcItem, color);
                DeleteObject(color);
            }

            LVCOLUMN lvc = {0};
            lvc.mask = LVCF_WIDTH;
            SendMessage(dis->hwndItem, LVM_GETCOLUMN, 0, (LPARAM)&lvc);

            RECT last_rc;
            int last_left = 0;

            last_rc = dis->rcItem;

            last_rc.left = 5 + last_left;
            last_rc.right = lvc.cx + last_left - 5;

            int width = last_rc.right - last_rc.left;
            int height = last_rc.bottom - last_rc.top;

            // Normal text position.
            RECT rc;
            rc.top = 0;
            rc.left = 0;
            rc.right = width;
            rc.bottom = height;

            HDC hdcMem = CreateCompatibleDC(dis->hDC);
            HBITMAP hbm = CreateCompatibleBitmap(dis->hDC, width, height);
            HBITMAP ohbm = (HBITMAP)SelectObject(hdcMem, hbm);
            DeleteObject(ohbm);
            DeleteObject(hbm);
            HFONT ohf = (HFONT)SelectObject(hdcMem, MainFont->GetHWNDFont());
            DeleteObject(ohf);

            SetBkMode(hdcMem, TRANSPARENT);

            RECT rc_text(rc);
            wstring str;

            if(idx >= 0 && idx <= 1)
            {
                if(idx == 0)
                    str = _T("Избранное");
                else
                    str = _T("Все радиостанции");
            }
            else if(idx >= 2 && idx <= list_end_idx[0])
            {
                if(idx == 2)
                {
                    if(list_end_idx[2])
                        str = _T("[-] Жанры");
                    else str = _T("[+] Жанры");
                }
                else
                {
                    str.assign(list_genre.at(idx-3).GetText());
                    //str = _T("genre");
                    rc_text.left = 15;
                    rc_text.right = rc.right - 15;
                }
            }
            else if(idx >= list_end_idx[0]+1 && idx <= list_end_idx[1])
            {
                if(idx == list_end_idx[0]+1)
                {
                    if(list_end_idx[3])
                        str = _T("[-] Битрейт");
                    else str = _T("[+] Битрейт");
                }
                else
                {
                    str = list_bitrate.at(idx - (list_end_idx[0]+2)).GetText();
                    rc_text.left = 15;
                    rc_text.right = rc.right - 15;
                }
            }

            if(selected == true)
            {
                HBRUSH color = CreateSolidBrush((COLORREF)GetSysColor(COLOR_HIGHLIGHT));
                FillRect(hdcMem, &rc, color);
                DeleteObject(color);
                SetTextColor(hdcMem, (COLORREF)RGB(0xFF, 0xFF, 0xFF));
                DrawText(hdcMem, str.c_str(), -1, &rc_text, DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
                BitBlt(dis->hDC, dis->rcItem.left + last_rc.left, last_rc.top, width, height, hdcMem, 0, 0, SRCCOPY);
            }
            else if(current == true)
            {
                HBRUSH color1 = CreateSolidBrush((COLORREF)RGB(0x80, 0x80, 0x80));
                FillRect(hdcMem, &rc, color1);
                DeleteObject(color1);
                SetTextColor(hdcMem, (COLORREF)RGB(0xFF, 0xFF, 0xFF));
                DrawText(hdcMem, str.c_str(), -1, &rc_text, DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
                BitBlt(dis->hDC, dis->rcItem.left + last_rc.left, last_rc.top, width, height, hdcMem, 0, 0, SRCCOPY);
            }
            else	// Draw normal text.
            {
                HBRUSH color1 = CreateSolidBrush((COLORREF)GetSysColor(COLOR_WINDOW));
                FillRect(hdcMem, &rc, color1);
                DeleteObject(color1);
                SetTextColor(hdcMem, (COLORREF)RGB(0x00, 0x80, 0x00));
                DrawText(hdcMem, str.c_str(), -1, &rc_text, DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
                BitBlt(dis->hDC, dis->rcItem.left + last_rc.left, last_rc.top, width, height, hdcMem, 0, 0, SRCAND);
            }

            DeleteDC(hdcMem);
            // Save the last left position of our column.
            last_left += lvc.cx;
            return 1;

        }
        break;
        }



    /*  HDC hdcMem = CreateCompatibleDC(dstr->hDC);
      HBITMAP hbmMem = CreateCompatibleBitmap(
                           dstr->hDC,
                           dstr->rcItem.right - dstr->rcItem.left,
                           dstr->rcItem.bottom - dstr->rcItem.top
                       );
      HBITMAP hbmOld = NULL;
      if(hbmMem)
          hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

      SetBkColor(hdcMem, RGB(255, 255, 255));
      ExtTextOut(hdcMem, 0, 0, ETO_OPAQUE,
                 &dstr->rcItem, 0, 0, 0);

      /*HBRUSH hbrush = CreateSolidBrush(RGB(0, 0, 0));
      HBRUSH oldhbrush = (HBRUSH)SelectObject(hdcMem, hbrush);
      FrameRect(hdcMem, &dstr->rcItem, hbrush);

      SelectObject(hdcMem, oldhbrush);
      DeleteObject(hbrush);

      BitBlt(dstr->hDC,
             dstr->rcItem.left, dstr->rcItem.top,
             dstr->rcItem.right - dstr->rcItem.left,
             dstr->rcItem.bottom - dstr->rcItem.top,
             hdcMem, 0, 0, SRCCOPY);

      SelectObject(hdcMem, hbmOld);
      DeleteObject(hbmMem);
      DeleteDC(hdcMem);*/
    return 0;
}

LRESULT MRadioApp::WM_NOTIFY_(WPARAM wParam, LPARAM lParam)
{
    NMHDR* custom_notify = reinterpret_cast<NMHDR*>(lParam);

    if(LPNMHDR(lParam)->code == TTN_GETDISPINFO) //if(LPNMHDR(lParam)->idFrom == MY_TOOLBAR_CONTROL_ID)
    {
        LPTOOLTIPTEXT tip = LPTOOLTIPTEXT(lParam);
        tip->hinst = MainInst;
        int id = tip->hdr.idFrom;
        int tbc = ToolBarGetButtonCount();
        for(int i=0; i<tbc; i++)
        {
            TBBUTTON btn = ToolBarGetButton(i);
            if(btn.idCommand==id)
            {
                tip->lpszText = LPWSTR(btn.dwData);
                break;
            }
        }
    }
    else if(custom_notify->hwndFrom == hwndTV)
    {
        switch (custom_notify->code)
        {

        case NM_DBLCLK:
        {
            LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;
            int idx = lpnmitem->iItem;
            if(idx != -1)
            {
                if(idx == 0)
                {
                    if(cur_idx_item != idx)
                    {
                        //favorit
                        GetFavStation();
                        cur_idx_item = idx;
                    }
                }
                else if(idx == 1)
                {
                    if(cur_idx_item != idx)
                    {
                        GetAllStation();
                        cur_idx_item = idx;
                    }
                }
                else if(idx == 2)
                {
                    list_end_idx[2] = !list_end_idx[2];
                    CreateList2TV();
                }
                else if(idx > 2 && list_end_idx[2] && idx < list_end_idx[0]+1)
                {
                    if(cur_idx_item != idx)
                    {
                        if(cur_idx_item > 1)
                        {

                        }
                        GetStationByGenre(list_genre.at(idx-3).int_data);
                        cur_idx_item = idx;
                        list_genre.at(idx-3).bool_data = true;
                    }
                }
                else if(list_end_idx[0]+1 == idx)
                {
                    list_end_idx[3] = !list_end_idx[3];
                    CreateList2TV();
                }
                else if(list_end_idx[3] && idx > list_end_idx[0]+1 && idx <= list_end_idx[1])
                {
                    if(cur_idx_item != idx)
                    {
                        GetStationByBitRate(list_bitrate.at(idx - (list_end_idx[0]+2)).int_data);
                        cur_idx_item = idx;
                        list_bitrate.at(idx - (list_end_idx[0]+2)).bool_data = true;
                    }
                }
                RedrawWindow(hwndTV, NULL, NULL, RDW_INVALIDATE);
            }
            break;
        }

        default:
            break;
        }
    }
    return 0;
}
LRESULT MRadioApp::WM_MEASUREITEM_(WPARAM wParam, LPARAM lParam)
{
    PMEASUREITEMSTRUCT pMI = PMEASUREITEMSTRUCT(lParam);
    if(pMI->CtlID == MY_LISTVIEW_CONTROL_ID)
    {
        pMI->itemHeight = 40;
        return 1;
    }
    return 0;
}

//
RECT MRadioApp::MainFormGetRect()
{
    RECT rect;
    GetWindowRect(hWin,&rect);
    return rect;
}

RECT MRadioApp::MainFormGetClientRect()
{
    RECT rect;
    GetClientRect(hWin,&rect);
    return rect;
}

int MRadioApp::MainFormGetWidth()
{
    RECT rect = MainFormGetRect();
    return rect.right - rect.left;
}

int MRadioApp::MainFormGetHeight()
{
    RECT rect = MainFormGetRect();
    return rect.bottom - rect.top;
}

int MRadioApp::MainFormGetX()
{
    return MainFormGetRect().left;
}

int MRadioApp::MainFormGetY()
{
    return MainFormGetRect().top;
}

//

TreeViewItem::TreeViewItem(wstring &v)
{
    text = v;
    int_data = 0;
}

TreeViewItem::TreeViewItem()
{
    int_data = 0;
    bool_data = false;
}

TreeViewItem::~TreeViewItem()
{
    text.clear();
}

//void TreeViewItem::SetText(wstring &v)
//{
//    text = v;
//}

const wchar_t* TreeViewItem::GetText()
{
    return text.c_str();
}

void TreeViewItem::SetText(const wchar_t *t)
{
    text.assign(t);
}

//

RadioStationInfo::RadioStationInfo()
{
    radio_site_type = 0;
    radio_data_type = 0;
    radio_id = 0;
    radio_num = 0;
}

RadioStationInfo::~RadioStationInfo()
{
    radio_name.clear();
    radio_site.clear();
    radio_url.clear();
    radio_id_str.clear();
    radio_base_genre.clear();
    radio_genre.clear();
    radio_base_bitrate = 0;
    radio_bitrate.clear();
}

void RadioStationInfo::SetRadioID(int v)
{
    radio_id = v;
    wchar_t buf[10];
    swprintf(buf, _T("%i"), radio_id);
    radio_id_str.assign(buf);
}

int RadioStationInfo::GetRadioID()
{
    return radio_id;
}

void RadioStationInfo::SetRadioNum(int v)
{
    radio_num = v;
    wchar_t buf[10];
    swprintf(buf, _T("%i"), radio_num);
    radio_num_str.assign(buf);
}

int RadioStationInfo::GetRadioNum()
{
    return radio_num;
}

const wchar_t* RadioStationInfo::GetRadioNumStr()
{
    if(!radio_num_str.empty()) return radio_num_str.c_str();
    else return _T("0");
}

const wchar_t* RadioStationInfo::GetRadioNamePlusID()
{
    wstring tmp;
    tmp.assign(radio_id_str);
    tmp.append(_T(". "));
    tmp.append(radio_name);
    if(!tmp.empty()) return tmp.c_str();
    else return _T("0");
}

const wchar_t* RadioStationInfo::GetRadioIDStr()
{
    if(!radio_id_str.empty()) return radio_id_str.c_str();
    else return _T("0");
}

const wchar_t* RadioStationInfo::GetRadioName()
{
    if(!radio_name.empty()) return radio_name.c_str();
    else return _T("");
}

void RadioStationInfo::SetRadioName(const wchar_t *t)
{
    radio_name.assign(t);
    //if(radio_name.empty()) radio_name.assign(_T("<название отсутствует>"));
}

bool RadioStationInfo::RadioNameEmpty()
{
    return radio_name.empty();
}

const wchar_t* RadioStationInfo::GetRadioUrl()
{
    if(!radio_url.empty()) return radio_url.c_str();
    else return _T("");
}

void RadioStationInfo::SetRadioUrl(const wchar_t *t)
{
    radio_url.assign(t);
}

bool RadioStationInfo::RadioUrlEmpty()
{
    return radio_url.empty();
}

const wchar_t* RadioStationInfo::GetRadioBaseGenre()
{
    if(!radio_base_genre.empty()) return radio_base_genre.c_str();
    else return _T("");
}

void RadioStationInfo::SetRadioBaseGenre(const wchar_t *t)
{
    radio_base_genre.assign(t);
}

bool RadioStationInfo::RadioBaseGenreEmpty()
{
    return radio_base_genre.empty();
}

const wchar_t* RadioStationInfo::GetRadioGenre()
{
    if(!radio_genre.empty()) return radio_genre.c_str();
    else return _T("");
}

void RadioStationInfo::SetRadioGenre(const wchar_t *t)
{
    radio_genre.assign(t);
}

bool RadioStationInfo::RadioGenreEmpty()
{
    return radio_genre.empty();
}
//
void RadioStationInfo::UpDateRadioGenre(std::vector<int>* gidx, std::vector<TreeViewItem>* genre_idx)
{
    if(radio_base_genre.empty()) return;
    std::vector<wstring> ret;
    wStringSplit(radio_base_genre, _T(";"), &ret);

    for(int idx = 0; idx != ret.size(); idx++)
    {
        int gid = _wtoi(ret.at(idx).c_str());
        gid = gidx->at(gid-1);
        if(idx != 0)
            radio_genre+=wstring(_T(", "))+genre_idx->at(gid).text;
        else
            radio_genre+=genre_idx->at(gid).text;
    }
}

// BITRATE
void RadioStationInfo::SetRadioBaseBitRate(int val)
{
    radio_base_bitrate = val;
}
//
const wchar_t* RadioStationInfo::GetRadioBitRate()
{
    if(!radio_bitrate.empty()) return radio_bitrate.c_str();
    else return _T("0");
}

void RadioStationInfo::SetRadioBitRate(const wchar_t *t)
{
    radio_bitrate.assign(t);
}

bool RadioStationInfo::RadioBitRateEmpty()
{
    return radio_bitrate.empty();
}

void RadioStationInfo::UpDateRadioBitRate(std::vector<int>* bidx, std::vector<TreeViewItem>* bitrate_idx)
{
    radio_bitrate.assign(bitrate_idx->at(bidx->at(radio_base_bitrate)).text);
}
//
/*std::vector<wstring> ret;
wStringSplit(radio_genre, _T(";"), &ret);
radio_genre.clear();
for(int idx = 0; idx != ret.size(); idx++)
{
    int gid = _wtoi(ret.at(idx).c_str());
    radio_genre+=genre_idx->at(gid);
}
TESTMESS(radio_genre.c_str());*/

const wchar_t* RadioStationInfo::GetFormatString2(wstring &format)
{
    wstring str;
    if(ParseFormattedString(format, str))
        return str.c_str();
    else return GetRadioName();
}

const wchar_t* RadioStationInfo::GetFormatString1(wstring &format)
{
    wstring str;
    if(ParseFormattedString(format, str))
        return str.c_str();
    else return GetRadioName();
}

//
bool RadioStationInfo::ParseFormattedString(wstring& str, wstring& text)
{
    bool ret = false;
    size_t i = 0;

    while(i < str.length())
    {
        if(str[i] == _T('%'))
        {
            if(ParseFormattedToken(str, i, text) == true)
            {
                ret = true;
            }
        }
        else if(str[i] == _T('$'))
        {
            if(ParseFormattedCommand(str, i, text) == true)
            {
                ret = true;
            }
        }
        else if(str[i] == _T('['))
        {
            if(ParseOptionalParameters(str, i, text) == true)
            {
                ret = true;
            }
        }
        else if(str[i] == _T('\''))
        {
            size_t pos = str.find(_T("\'"), i+1);
            if (pos == wstring::npos)
            {
                text += _T("?");
                i++;
            }
            else
            {
                text += str.substr(i+1, pos - i - 1);
                i = pos+1;
            }
        }
        else
        {
            wchar_t ch[2];
            ch[0] = str[i];
            ch[1] = _T('\0');
            text += ch;
            i++;
        }
    }

    return ret;
}

bool RadioStationInfo::ReadToken(wstring& value, wstring& field)
{
    if(value.find(_T("title")) != wstring::npos)
    {
        if(!radio_name.empty())
        {
            field.assign(radio_name);
            return true;
        }
    }
    if(value.find(_T("genre")) != wstring::npos)
    {
        if(!radio_genre.empty())
        {
            field.assign(radio_genre);
            return true;
        }
    }
    if(value.find(_T("bitrate")) != wstring::npos)
    {
        if(!radio_bitrate.empty())
        {
            field.assign(radio_bitrate);
            return true;
        }
    }
    else if(value.find(_T("url")) != wstring::npos)
    {
        field.assign(radio_url);
        return true;
    }
    else if(value.find(_T("siteurl")) != wstring::npos)
    {
        if(!radio_site.empty())
        {
            field.assign(radio_site);
            return true;
        }
    }
    else if(value.find(_T("id")) != wstring::npos)
    {
        field.assign(radio_id_str);
        return true;
    }
    else if(value.find(_T("num")) != wstring::npos)
    {
        field.assign(radio_num_str);
        return true;
    }
    field = _T("");
    return false;
}

bool RadioStationInfo::ParseFormattedToken(wstring& str, size_t& x, wstring& text)
{
    bool ret = false;
    size_t i = x;

    if(str[i] == _T('%'))
    {
        wstring string = str;
        size_t pos = string.find(_T("%"), i + 1);
        if(pos == wstring::npos)
        {
            text += _T("?");
            x = i+1;
            return false;
        }

        wstring value;
        wstring token = str.substr(i + 1, pos - i - 1);
        ret = ReadToken(token, value);
        if(ret == true) text += value;
        x = pos+1;
    }
    return ret;
}

bool RadioStationInfo::ParseFormattedCommand(wstring& str, size_t& x, wstring& text)
{
    bool ret = false;
    size_t i = x;

    if(str[i] == _T('$'))
    {
        wstring result;
        std::vector<wstring> params;
        size_t pos = str.find(_T("("), i + 1);
        if(pos == wstring::npos)
        {
            text += _T("?");
            x = i+1;
            return false;
        }
        wstring token = str.substr(i + 1, pos - i - 1);
        if(ParseNestedParameters(str, pos, params) == false)
        {
            text += _T("?");
            x = i+1;
            return false;
        }

        if(token == _T("if") && (params.size() == 2 || params.size() == 3))
        {
            if(params.size() == 2) params.push_back(_T(""));
            if(ParseFormattedString(params[0], result) == true)
            {
                result = _T("");
                if(ParseFormattedString(params[1], result) == true)
                {
                    ret = true;
                }
                text += result;
            }
            else
            {
                result = _T("");
                if(ParseFormattedString(params[2], result) == true)
                {
                    ret = true;
                }
                text += result;
            }
        }
        else if(token == _T("if2") && params.size() == 2)
        {
            if(ParseFormattedString(params[0], result) == true)
            {
                ret = true;
                text += result;
            }
            else
            {
                result = _T("");
                if(ParseFormattedString(params[1], result) == true)
                {
                    ret = true;
                }
                text += result;
            }
        }
        else if (token == _T("lo") && params.size() == 1)
        {
            //if(ParseFormattedString(params[0], result) == TRUE)
            //{
            ParseFormattedString(params[0], result);
            ret = true;
            my_unicode_lower(result);
            // }
            text += result;
        }
        else if (token == _T("up") && params.size() == 1)
        {
            // if(ParseFormattedString(params[0], result) == TRUE)
            // {
            ParseFormattedString(params[0], result);
            ret = true;
            my_unicode_upper(result);
            // }
            text += result;
        }
        else if (token == _T("caps") && params.size() == 1)
        {
            ParseFormattedString(params[0], result);
            ret = true;
            //TESTMESS(result.c_str());
            my_unicode_caps(result);
            text += result;
        }
        else
        {
            text += _T("?");
            x = i+1;
            return false;
        }

        x = pos;
    }

    return ret;
}

bool RadioStationInfo::ParseOptionalParameters(wstring& str, size_t& x, wstring& text)
{
    wstring opts, value;
    wchar_t ch[2];
    int level = 0;
    size_t i = x;
    ch[1] = _T('\0');

    if(str[i] != _T('['))
    {
        x++;
        text += _T("?");
        return false;
    }
    i++;
    while(i < str.length())
    {
        if(str[i] == _T('['))
        {
            level++;
        }
        else if(str[i] == _T(']'))
        {
            level--;
            if(level < 0)
            {
                i++;
                break;
            }
        }
        ch[0] = str[i];
        opts += ch;
        i++;
    }

    x = i;
    if(level >= 0)
    {
        text += _T("?");
        return false;
    }

    if(ParseFormattedString(opts, value) == true)
    {
        text += value;
        return true;
    }
    return false;
}

bool RadioStationInfo::ParseNestedParameters(wstring& str, size_t& x, std::vector<wstring>& list)
{
    int level = 0;
    list.clear();
    size_t i = x, start = x+1;

    if (str[i] != _T('(')) return false;
    i++;
    while(i < str.length())
    {
        if(str[i] == _T('('))
        {
            level++;
        }
        else if(str[i] == _T(')'))
        {
            level--;
            if(level < 0)
            {
                if(i - start > 0)
                {
                    wstring token = str.substr(start, i - start);
                    list.push_back(token);
                }
                i++;
                break;
            }
        }
        else if(str[i] == _T(','))
        {
            if(level == 0)
            {
                wstring token = str.substr(start, i - start);
                list.push_back(token);
                start = i+1;
            }
        }
        i++;
    }

    x = i;
    if(list.size() > 0) return true;
    return false;
}

//
