#ifndef MYRADIO_H_INCLUDED
#define MYRADIO_H_INCLUDED

#include "font/font.h"
#include "audio_engine/types.h"

#define APP_ATOM  _T("MYRADIO_ONE_RUN")
#define APP_CLASS _T("MYRADIO_CLASS")

#define APP_VER   _T("0.0.2.6")
#define APP_NAME  _T("MyRadio")
#define APP_TITLE APP_NAME _T(" v") APP_VER

#define INI_SECTION_NAME _T("myradio")

#define PANEL_HEIGHT 22
#define TOOLBAR_WIDTH 178

#define MY_TREEVIEW_CONTROL_ID 1
#define MY_LISTVIEW_CONTROL_ID 2
#define MY_PANEL_CONTROL_ID 3
#define MY_TOOLBAR_CONTROL_ID 4

#define BASE_RADIO_VER 1    //Current version my SQlite3 Base ver

#define LVS_EX_DOUBLEBUFFER 0x00010000

#define SITE_TYPE_ICECAST 1

// ERRORS
#define ERROR_SOCK_CONNFAILED           -10
#define ERROR_SOCK_PROXY_AUTHFAILED     -11
#define ERROR_SOCK_PROXY_CONNFAILED     -12
#define ERROR_SOCK_PROXY_BROKEN         -13
//
struct base_info
{
    int radio_count;
    int base_ver;
    SYSTEMTIME time_create;
    SYSTEMTIME time_modif;
};

/*
typedef enum
{
    type_tv,
    type_tvrlist,
} type_treeview;
*/

#include "myradio_string.h"

/* TREEVIEW WIN */

class TreeViewItem
{
public:
    TreeViewItem(wstring &v);
    TreeViewItem();
    ~TreeViewItem();
    //void SetText(wstring &v);
    void SetText(const wchar_t *t);
    const wchar_t* GetText();
    wstring text;
    int int_data;
    bool bool_data;
};

class RadioStationInfo
{
public:
    RadioStationInfo();
    ~RadioStationInfo();
    //
    void UpDateRadioGenre(std::vector<int>* gidx, std::vector<TreeViewItem>* genre_idx);
    void UpDateRadioBitRate(std::vector<int>* bidx, std::vector<TreeViewItem>* bitrate_idx);
    //
    const wchar_t* GetRadioNamePlusID();
    const wchar_t* GetFormatString1(wstring &format);
    const wchar_t* GetFormatString2(wstring &format);
    //
    void SetRadioID(int v);
    int GetRadioID();
    const wchar_t* GetRadioIDStr();
    //
    void SetRadioNum(int v);
    int GetRadioNum();
    const wchar_t* GetRadioNumStr();
    //
    const wchar_t *GetRadioName();
    void SetRadioName(const wchar_t *t);
    bool RadioNameEmpty();
    //
    const wchar_t *GetRadioUrl();
    void SetRadioUrl(const wchar_t *t);
    bool RadioUrlEmpty();
    //
    const wchar_t *GetRadioBaseGenre();
    void SetRadioBaseGenre(const wchar_t *t);
    bool RadioBaseGenreEmpty();
    //
    const wchar_t *GetRadioGenre();
    void SetRadioGenre(const wchar_t *t);
    bool RadioGenreEmpty();
    //
    void SetRadioBaseBitRate(int val);
    //
    const wchar_t *GetRadioBitRate();
    void SetRadioBitRate(const wchar_t *t);
    bool RadioBitRateEmpty();
private:
    int radio_id;
    int radio_num;
    short   radio_site_type;
    short   radio_data_type;
    wstring radio_name;
    wstring radio_id_str;
    wstring radio_num_str;
    wstring radio_site;
    wstring radio_url;
    wstring radio_base_genre;
    wstring radio_genre;
    int radio_base_bitrate;
    wstring radio_bitrate;
    bool ParseFormattedString(wstring& str, wstring& text);
    bool ParseFormattedToken(wstring& str, size_t& x, wstring& text);
    bool ParseFormattedCommand(wstring& str, size_t& x, wstring& text);
    bool ParseOptionalParameters(wstring& str, size_t& x, wstring& text);
    bool ParseNestedParameters(wstring& str, size_t& x, std::vector<wstring>& list);
    bool ReadToken(wstring& value, wstring& field);
};

/* */

/* TREEVIEW WIN */

// Color definitions
/*#define INVERTED_SELECTION_COLOR	RGB(225, 225, 225)
#define LAST_CHANGED_COLOR			RGB(180, 0, 0)
#define LAST_CHANGED_HIGHLIGHT		RGB(255, 100, 100)
#define GRID_COLOR					RGB(200, 220, 255)
#define HIERARCHY_COLOR				RGB(180, 180, 180)*/

/*#include "mytreeview/TreeData.h"

void InitializeMYTreeWindowClass(HINSTANCE hInstance);

class WinMYTreeView
{
public:
    WinMYTreeView(HWND hwndP, HINSTANCE mainI, MyFont *mf);
    ~WinMYTreeView();

    WNDPROC m_wndProcListView;

    int CreateControl(int cWidth, int cHeight);
    void DestroyControl();
    int CreateListView(HWND pParentView);

    void RedrawVisibleItems();
    void InitializeDrawingDimensions();

    void ListViewVScroll(HWND hWnd, UINT code, int pos);

    void SetScrollPosition(int vPos);

    void OnKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
    //
    bool IsPointInExpandCollapseBox(int iRow, int xPos, int yPos);
    bool MouseWheel(HWND hWnd, WORD fwKeys, short zDelta, int xPos, int yPos);
    int LeftMouseClick(int xPos, int yPos);
    int LeftMouseDblClick(int xPos, int yPos);
    //
    void UpdateScrollPosition();

protected:
    HWND m_pListView;
    HWND m_pRootContainer;

    std::vector<wstring> MainItem;

    TreeData *MainTD;

    MyFont *MainFont;

    HPEN m_hPenBlack;				// Normally, text are written in black
    HPEN m_hPenWhite;				// Highlighted text color
    HPEN m_hPenRed;					// Last changed text color
    HPEN m_hPenLightRed;			// Last changed text color if highlighted
    HPEN m_hPenGrid;				// Grid pen - light blue :)
    HPEN m_hPenHierarchy;			// Hierarchy pen - some sort of grey
    HBRUSH m_hBrushWhite;			// White background brush
    HBRUSH m_hBrushHighlight;		// Default highlight color brush

    HWND hwndParent;
    HINSTANCE mainInst;

    int m_iColumn0Width;

    int m_iInitialIndent;
    int m_iRowHeight;
    int m_iBoxWidth;
    int m_iBoxHeight;
    int m_iBoxYOffset;
    int m_iHeaderHeight;
    int m_iScrollPos;
    int m_iVisibleRange;

    int m_iTextYOffset;
    int m_iPaintYOffset;

public:
    void GetBoxRect(int iRow, RECT &rBoxRect);
    //
    void CollapseTree(int iRow);
    void ExpandTree(int iRow);

    inline HWND GetListView()
    {
        return m_pListView;
    }

    inline int GetDepth(int iRow)
    {
        return MainTD->GetDepth(iRow);
    }
    inline int GetParent(int iRow)
    {
        return MainTD->GetParent(iRow);
    }
    inline void *GetData(int iRow)
    {
        return MainTD->GetData(iRow);
    }
    inline bool HasChildren(int iRow)
    {
        return MainTD->HasChildren(iRow);
    }
    inline bool IsLastItem(int iRow)
    {
        return MainTD->IsLastItem(iRow);
    }
    inline bool IsDirty(int iRow)
    {
        return MainTD->IsDirty(iRow);
    }
    inline int GetNumEntries()
    {
        return MainTD->GetNumEntries();
    }
    inline bool GetExpanded(int iRow)
    {
        return MainTD->GetExpanded(iRow);
    }
    inline void SetExpanded(int iRow, bool bExpand)
    {
        MainTD->SetExpanded(iRow, bExpand);
    }

}; // WinMYTreeView

/* */
#define ID_TOOLBAR_PLAY_PAUSE   201
#define ID_TOOLBAR_PREV         202
#define ID_TOOLBAR_NEXT         203
#define ID_TOOLBAR_STOP         204
#define ID_TOOLBAR_REC          205
#define ID_TOOLBAR_SEA          206
#define ID_TOOLBAR_UPD          207
#define ID_TOOLBAR_SET          208

#include "xml/Cxml.h"
#include "sqlite3/sqdb.h"
#include "audio_engine/myaudio_engine.h"
#include "net/mysock_proxy.h"
using namespace sqdb;
class MRadioApp
{
public:

    static MRadioApp *instance;

    MRadioApp(HINSTANCE phInst, LPSTR lpszCmdLine);
    ~MRadioApp();

    int CreateApp();
    int CreateMainForm();
    void RunProgramm();
    int GetMsgWParam();

    HWND hWin;
    HINSTANCE MainInst;

    sys_info_t sysinfo;

    wstring StartDir;
    wstring ConfigDir;

    InitWinSock *init_winsock;

    wstring sqlite3_file;
    bool MainDbDataExists;
    int MainDbRadioCount;
    bool UpDateBaseProc;
    Db *MainDb;
    MyFont *MainFont;

    int R2NP(int v);
    void GetCountListRadio(bool DbExist = false);

    base_info current_base_info;

protected:

    MyAudioEngine* mae = NULL;

    HWND hwndStatus;
    HWND hwndTV;
    HWND hwndRadioList;
    HWND hwndPanel;
    HWND hwndToolBar;

    HIMAGELIST tb_imagelist;
    HBITMAP tb_bmp;

    MSG msg;

    HCURSOR SplitCursor;
    bool bSplitterMoving;

    //std::vector<wstring> results;
    // sqlite3
    int cur_idx_item;
    int list_radio_size;
    std::vector<RadioStationInfo> list_radio;
    void GetFavStation();
    void GetAllStation();
    void GetStationByGenre(int genre_idx);
    void GetStationByBitRate(int br_idx);
    wstring lv_format_str1, lv_format_str2;
    //
    WNDPROC _PrevControlPanelProc;

    std::vector<TreeViewItem> list_genre;
    std::vector<TreeViewItem> list_bitrate;
    std::vector<TreeViewItem> list_format;
    std::vector<int> list_genre_idx;
    std::vector<int> list_bitrate_idx;
    int list_end_idx[4] = {0, 0, 0, 0};
    //WNDPROC _PrevWndTVListProc;
    //static LRESULT WINAPI WndTVListProc_(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    //LRESULT WndTVListProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    static LRESULT WINAPI WndMainProc_(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    //
    LRESULT WM_CreateForm();
    LRESULT WM_CLOSE_(WPARAM wParam, LPARAM lParam);
    LRESULT WM_QUERYENDSESSION_(WPARAM wParam, LPARAM lParam);
    LRESULT WM_SIZE_(WPARAM wParam, LPARAM lParam);
    LRESULT WM_LBUTTONDOWN_(WPARAM wParam, LPARAM lParam);
    LRESULT WM_LBUTTONUP_(WPARAM wParam, LPARAM lParam);
    LRESULT WM_MOUSEMOVE_(WPARAM wParam, LPARAM lParam);
    LRESULT WM_DRAWITEM_(WPARAM wParam, LPARAM lParam);
    LRESULT WM_COMMAND_(WPARAM wParam, LPARAM lParam);
    LRESULT WM_NOTIFY_(WPARAM wParam, LPARAM lParam);
    LRESULT WM_MEASUREITEM_(WPARAM wParam, LPARAM lParam);
    //
    void MainFormStayOnTop(bool fon);
    void MainFormShow_Hide(bool fShow);
    int GetMainFormWindowState();
    void SetMainFormWindowState(int statetype);
    void CloseWindows();
    void SetWinPos2Set();
    // STATUS BAR
    void CreateStatusBar();
    void CalcStatusBarSize();
    int GetStatusBarHeight();
    int GetStatusBarWidth();
    int GetStatusBarLeft();
    int GetStatusBarTop();
    void SetStatusBarTxt(wchar_t *val1, wchar_t *val2);
    // TOOLBAR
    void CreateToolBar();
    bool ToolBarInsertButton(int index, DWORD style, DWORD state, int nimage,
                             LPCWSTR text, LPCWSTR tip, UINT id);
    bool ToolBarAddButton(DWORD style, DWORD state, int nimage, LPCWSTR text, LPCWSTR tip, UINT id);
    bool ToolBarInsertSeparator(int index);
    bool ToolBarAddSeparator();
    bool ToolBarDeleteButton(int index);
    bool ToolBarMoveButton(int index, int new_index);
    bool ToolBarSetButtonState(int id, DWORD state);
    DWORD ToolBarGetButtonState(int id);
    bool ToolBarEnableButton(int id, bool enable);
    bool ToolBarSetImageList(HIMAGELIST& hlist);
    int ToolBarGetButtonCount();
    TBBUTTON ToolBarGetButton(int index);
    //
    WNDPROC _PrevTreeViewRadioListProc;
    static LRESULT WINAPI WndTreeViewRadioListProc_(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT WndTreeViewRadioListProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    HTREEITEM TreeViewInsertItem(HWND vhwnd, LPCWSTR text, int image, LPARAM param,
                                 HTREEITEM parent, HTREEITEM insert_after);
    void TreeViewSetItemData(HWND vhwnd, HTREEITEM item, LPARAM param);
    LPARAM TreeViewGetItemData(HWND vhwnd, HTREEITEM item);
    DWORD TreeViewGetCount(HWND vhwnd);
    bool TreeViewItemDelete(HWND vhwnd, HTREEITEM item);
    bool TreeViewDeleteAllItems(HWND vhwnd);
    /*
    TVE_COLLAPSE            Collapses the list.
    TVE_COLLAPSERESET       Collapses the list and removes the child items. Note
    that TVE_COLLAPSE       must also be specified.
    TVE_EXPAND	            Expands the list.
    TVE_TOGGLE	            Collaps
    */
    bool TreeViewExpand(HWND vhwnd, HTREEITEM item, DWORD flg);
    //
    void CreateTreeView();
    int GetTreeViewWidth();
    int GetTreeViewHeight();

    //
    void CreateListViewRadioList();
    //
    void CreatePanel();
    //
    void DestroyFormControl();

    void GetSysInfo();

    bool GetWindowSize(HWND win, int *rwidth, int *rheight);
    RECT MainFormGetRect();
    RECT MainFormGetClientRect();
    int MainFormGetWidth();
    int MainFormGetHeight();
    int MainFormGetX();
    int MainFormGetY();
    //
    DWORD GetSplitPos();
    void SetSplitPos(DWORD val);
    //
    void NewDateInBase();
    void CreateList2TV();
    void UpDateTVColumSize();
};

/* */
/* myradio_addradio.cpp */

class AddRadioClass
{
public:
    AddRadioClass(MRadioApp *vmra);
    ~AddRadioClass();
    bool ShowAddRadioDialog();
    void CloseAddRadioDialog();
    bool UpdateBaseOk();

private:
    MRadioApp *mra;
    HWND hDialogWin;
    bool newdatainbase;

    BOOL pThreadTerminate;
    HANDLE hUpdateThread;
    static unsigned WINAPI UpdateBaseThreadProc(void *pArg);
    unsigned UpdateBaseThread();

    LRESULT _OnMessage(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam);
    void StartUpDateRadioBase();
    void StopUpdateBase();

protected:
    static INT_PTR WINAPI _WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    void myfill(bool newbase, CNode* xml_node);
};

/* Base Dialog */

typedef struct
{
    WORD dlgVer;
    WORD signature;
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;
    //sz_Or_Ord menu;
    //sz_Or_Ord windowClass;
    //WCHAR title[titleLen];
    //WORD pointsize;
    //WORD weight;
    //BYTE italic;
    //BYTE charset;
    // WCHAR typeface[stringLen];
} DLGTEMPLATEEX;
typedef DLGTEMPLATEEX *LPDLGTEMPLATEEXA;
typedef DLGTEMPLATEEX *LPDLGTEMPLATEEXW;
typedef LPDLGTEMPLATEEXW LPDLGTEMPLATEEX;

class BaseDialogWindow
{
public:
    BaseDialogWindow(MRadioApp *vmra);
    ~BaseDialogWindow();

    void CreateModal();
    void InitPage(PROPSHEETPAGE *page, int id, DLGPROC dlgproc, LPARAM lparam, LPFNPSPCALLBACK callback);
    virtual void CreatePages();
    DWORD GetResult();

protected:
    MRadioApp *mra;
    PROPSHEETHEADER header;
    DWORD result;
};

/* Config Dialog */
class ConfigDialog : public BaseDialogWindow
{
public:
    PROPSHEETPAGE psp[2];

    ConfigDialog(MRadioApp *vmra) : BaseDialogWindow(vmra) {}

    virtual void CreatePages();
    void Show();
    void EnableControlProxy(HWND wnd);

    static int WINAPI WindowCallback(HWND hwndDlg, UINT uMsg, LPARAM lParam);
    static bool WINAPI OutputProc(HWND dialog, UINT message, WPARAM wparam, LPARAM lparam);
    static bool WINAPI ConnectProc(HWND dialog, UINT message, WPARAM wparam, LPARAM lparam);

    bool OutputPage(HWND dialog, UINT message, WPARAM wparam, LPARAM lparam);
    bool ConnectPage(HWND dialog, UINT message, WPARAM wparam, LPARAM lparam);

protected:

};
/* */

/* myutils.cpp */
long long SysTime264Time(const SYSTEMTIME *time);
void Time642SysTime(long long in, SYSTEMTIME* out);
//
wstring myhtml_decode(wchar_t* src, bool rep_space = false);
//
HWND Dialog_GetItemHWND(HWND dhwnd, UINT id);
int Dialog_GetItemTextLen(HWND dhwnd, UINT id);
wstring Dialog_GetItemText(HWND dhwnd, UINT id);
void Dialog_GetItemText2(HWND dhwnd, UINT id, wchar_t* text, int len);
void Dialog_GetItemText2ANSI(HWND dhwnd, UINT id, char* text, int len);
bool DSetItemText(HWND dhwnd, LPCWSTR text, UINT id);
int Dialog_GetItemCheck(HWND dhwnd, UINT id);
bool Dialog_SetItemCheck(HWND dhwnd, UINT id, int check);
int Dialog_GetItemInt(HWND dhwnd, UINT id);
bool DSetItemInt(HWND dhwnd, int value, UINT id);
bool Dialog_EnableHWND(HWND dhwnd, UINT id, bool en);
//
bool CenterWindow(HWND hwnd);
wstring Bytes2WString(DWORD inBytes);
//
#define MY_MES_ERROR        1
#define MY_MES_QUESTION     2
#define MY_MES_WARNING      3
#define MY_MES_INFORMATION  4
#define MY_MES_STOP         5
int MyMessageBox(HWND hwnd, wstring text, unsigned int type);
//
wstring mySHGetFolderPath(int dir_type);
wstring myGetStartDir(HINSTANCE phInst);
void WinVer2WString(version_e v, wstring *vstr);
void Arch2WString(architecture_e a, wstring *vstr);
void GetMySystemInfo(sys_info_t* si);
unsigned char PrTypSet2CBCurSel(unsigned short type);
unsigned short CBCurSelPrTypSet(unsigned char cursel);
//wstring utf8to16(const char* src);
//std::string wstring2string(const wstring& str);
//
wstring MySystemTime2WStr(LCID loc, DWORD flags, const SYSTEMTIME *time, LPCWSTR format);
wstring MySystemDate2WStr(LCID loc, DWORD flags, const SYSTEMTIME *time, LPCWSTR format);
wstring MyDateAndTime2WStrShort();
wstring MyGetFormatDateAndTimeSystemFormat(const SYSTEMTIME *date_time);
//
#include "my_global_utils.h"

void make_lowercaseW_LATIN(wstring &s);
void toLower(wstring &string);
void toUpper(wstring &string);
void wStringSplit(wstring str, wstring delim, std::vector<wstring>* results);
int VectorFindNameIdx(std::vector<wstring>* vect, wstring find_me);
/* */
#define ETDT_ENABLE 0x00000002
#define ETDT_USETABTEXTURE 0x00000004
#define ETDT_ENABLETAB (ETDT_ENABLE|ETDT_USETABTEXTURE)
HRESULT WINAPI MyEnableThemeDialogTexture(HWND hWnd, DWORD dwFlags);

// UNICODE FUNCT
uint16_t unicode_lower(uint16_t c);
uint16_t unicode_upper(uint16_t c);
void my_unicode_lower(wstring &str);
void my_unicode_upper(wstring &str);
void my_unicode_caps(wstring &str);
//void my_unicode_fold(wstring &str);
//void my_unicode_title(wstring &str);
//

#endif // MYRADIO_H_INCLUDED
