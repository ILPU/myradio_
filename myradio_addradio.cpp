#include "myhdr.h"
#include "myradio.h"
#include "res/resource.h"

AddRadioClass::AddRadioClass(MRadioApp *vmra)
{
    mra = vmra;
    hDialogWin = 0;
    hUpdateThread = NULL;
    pThreadTerminate = true;
    newdatainbase = false;
}

AddRadioClass::~AddRadioClass()
{
    //
}

bool AddRadioClass::ShowAddRadioDialog()
{
    if(!DialogBoxParam(mra->MainInst, MAKEINTRESOURCE(IDD_DIALOG_UPDATE_BASE), mra->hWin,
                       _WndProc, LPARAM(this))) return false;
    return true;
}

void AddRadioClass::CloseAddRadioDialog()
{
    if(hDialogWin)
        EndDialog(hDialogWin, TRUE);
}

LRESULT AddRadioClass::_OnMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {

    case WM_INITDIALOG:
    {
        if(mra->current_base_info.radio_count > 0)
        {
            DSetItemText(hDialogWin, MyGetFormatDateAndTimeSystemFormat(&mra->current_base_info.time_create).c_str(), IDC_STATIC_BASE_CREATE);
            DSetItemText(hDialogWin, MyGetFormatDateAndTimeSystemFormat(&mra->current_base_info.time_modif).c_str(), IDC_STATIC_BASE_MODIF);
            DSetItemInt(hDialogWin, mra->current_base_info.radio_count, IDC_STATIC_BASE_COUNT);
            DSetItemInt(hDialogWin, mra->current_base_info.base_ver, IDC_STATIC_BASE_VER);
        }
        else
        {
            DSetItemInt(hDialogWin, 0, IDC_STATIC_BASE_COUNT);
        }
    }
    break;

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {

        case IDC_UPDATE_BASE_BTN_START:
        {
            if(pThreadTerminate)
                StartUpDateRadioBase();
            else StopUpdateBase();
        }
        break;

        case IDC_UPDATE_BASE_BTN_ABORT:
        {
            StopUpdateBase();
            EndDialog(hwnd, wparam);
            return TRUE;
        }
        break;

        }
    }
    return 0;
}

INT_PTR WINAPI AddRadioClass::_WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    AddRadioClass *dlg = 0;
    if(message == WM_INITDIALOG)
    {
        dlg = (AddRadioClass*)(lparam);
        dlg->hDialogWin = hwnd;

        SetWindowLong(hwnd, GWL_USERDATA, LONG(dlg));
        dlg->_OnMessage(hwnd, message, wparam, lparam);
        return true;
    }

    dlg = (AddRadioClass*)GetWindowLong(hwnd, GWL_USERDATA);
    if(dlg)
        dlg->_OnMessage(hwnd, message, wparam, lparam);
    return false;
}

void AddRadioClass::StopUpdateBase()
{
    if(hUpdateThread)
    {
        pThreadTerminate = true;
        //WaitForSingleObject(hUpdateThread, INFINITE);
    }
}

void AddRadioClass::StartUpDateRadioBase()
{
    newdatainbase = false;
    unsigned uiThreadID;
    //hUpdateThread = CreateThread(0, 0, UpdateBaseThreadProc, this, 0, &uiThreadID);
    hUpdateThread = (HANDLE)_beginthreadex(0, 0, UpdateBaseThreadProc, this, 0, &uiThreadID);
    DSetItemText(hDialogWin, _T("Стоп"), IDC_UPDATE_BASE_BTN_START);
}

unsigned WINAPI AddRadioClass::UpdateBaseThreadProc(void *pArg)
{
    AddRadioClass *_this = (AddRadioClass *)pArg;
    return _this->UpdateBaseThread();
}

bool AddRadioClass::UpdateBaseOk()
{
    return newdatainbase;
}

void AddRadioClass::myfill(bool newbase, CNode* xml_node)
{
    if(xml_node == NULL)
        return;
    DSetItemText(hDialogWin, _T("Удаление старых данных..."), IDC_STATIC_PRORESS_2);
    if(!newbase)
    {
        try
        {
            mra->MainDb->BeginTransaction();
            mra->MainDb->Query(_T("DELETE FROM TableRadios")).Next();
            mra->MainDb->Query(_T("DELETE FROM TableGenres")).Next();
            mra->MainDb->Query(_T("DELETE FROM TableFormats")).Next();
            mra->MainDb->Query(_T("DELETE FROM TableBitRate")).Next();
            mra->MainDb->Query(_T("DELETE FROM sqlite_sequence WHERE NAME='TableRadios'")).Next();
            //mra->MainDb->Vacuum();
            mra->MainDb->CommitTransaction();
        }
        catch(const sqdb::Exception& e)
        {
            mra->MainDb->RollbackTransaction();
        }
    }

    int radio_count = 0;
    radio_count = xml_node->GetNodeListSize();
    GlobalConfig->MyAddLogParam(_T("icycast radio count = %d"), radio_count);

    Statement stt2 = mra->MainDb->Query(_T("UPDATE BaseInfo SET RadioCount=?"));
    stt2.Bind(1, radio_count);
    stt2.Next();
    DSetItemText(hDialogWin, _T("Запись в базу..."), IDC_STATIC_PRORESS_2);
    std::vector<wstring> all_genre;
    std::vector<wstring> all_bitrate;
    std::vector<wstring> all_format;
    all_genre.push_back(_T(""));
    all_bitrate.push_back(_T("0"));
    all_format.push_back(_T(""));
    try
    {
        mra->MainDb->BeginTransaction();
        Statement stt = mra->MainDb->Query(_T("INSERT INTO TableRadios(SiteType, RadioName, RadioURL, FormatIdx, BitRateIdx, GenreIdx) VALUES(?, ?, ?, ?, ?, ?)"));
        for(int i = 0; i != radio_count; i++)
        {
            wstring cur_genre(myhtml_decode(xml_node->GetNodeByIdx(i)->GetNodeByIdx(1)->GetValue()));
            wstring cur_genre_db;
            std::vector<wstring> splits;
            if(!cur_genre.empty())
            {
                wStringSplit(cur_genre, _T(" "), &splits);
                if(splits.size() != 0)
                {
                    for(unsigned int y = 0; y < splits.size(); y++)
                    {
                        int ret = VectorFindNameIdx(&all_genre, splits.at(y));
                        wchar_t lpszString[50];
                        swprintf(lpszString, _T("%i"),(int)ret);
                        if(!cur_genre_db.empty())
                        {
                            cur_genre_db+=_T(";");
                            cur_genre_db+=wstring(lpszString);
                        }
                        else cur_genre_db+=wstring(lpszString);
                    }
                }
                else cur_genre_db.assign(_T("0"));
            }
            else cur_genre_db.assign(_T("0"));

            if(pThreadTerminate) break;

            wstring cur_bitrate(xml_node->GetNodeByIdx(i)->GetNodeByIdx(4)->GetValue());
            int cur_bitrate_int = 0;
            if(!cur_bitrate.empty())
            {
                int tmp = (int)_wtoi(xml_node->GetNodeByIdx(i)->GetNodeByIdx(4)->GetValue());
                if(tmp > 9999)
                    tmp/=1000;
                wchar_t lpszString[50];
                swprintf(lpszString, _T("%i"),(int)tmp);
                cur_bitrate.assign(lpszString);
                cur_bitrate_int = VectorFindNameIdx(&all_bitrate, cur_bitrate);
            }

            wstring cur_format(xml_node->GetNodeByIdx(i)->GetNodeByIdx(5)->GetValue());
            int cur_format_int = 0;
            if(!cur_format.empty())
            {
                cur_format_int = VectorFindNameIdx(&all_format, cur_format);
            }
            //stt.Reset();
            stt.Bind(1, SITE_TYPE_ICECAST);
            stt.Bind(2, myhtml_decode(xml_node->GetNodeByIdx(i)->GetNodeByIdx(7)->GetValue()));
            stt.Bind(3, xml_node->GetNodeByIdx(i)->GetNodeByIdx(6)->GetValue());
            stt.Bind(4, cur_format_int);

            stt.Bind(5, cur_bitrate_int);
            stt.Bind(6, cur_genre_db);
            stt.Next();
            DSetItemInt(hDialogWin, i+1, IDC_STATIC_FIND_RADIO);
        }
        mra->MainDb->CommitTransaction();
    }
    catch(const sqdb::Exception& e)
    {
        mra->MainDb->RollbackTransaction();
    }

    try
    {
        mra->MainDb->BeginTransaction();
        Statement stt = mra->MainDb->Query(_T("INSERT INTO TableGenres(ID, Value) VALUES(?, ?)"));
        for(unsigned int i = 0; i < all_genre.size(); i++)
        {
            if(pThreadTerminate) break;
            stt.Bind(1, (int)i);
            stt.Bind(2, all_genre.at(i));
            stt.Next();
        }
        mra->MainDb->CommitTransaction();
    }
    catch(const sqdb::Exception& e)
    {
        mra->MainDb->RollbackTransaction();
    }

    try
    {
        mra->MainDb->BeginTransaction();
        Statement stt = mra->MainDb->Query(_T("INSERT INTO TableFormats(ID, Value) VALUES(?, ?)"));
        for(unsigned int i = 0; i < all_format.size(); i++)
        {
            if(pThreadTerminate) break;
            stt.Bind(1, (int)i);
            stt.Bind(2, all_format.at(i));
            stt.Next();
        }
        mra->MainDb->CommitTransaction();
    }
    catch(const sqdb::Exception& e)
    {
        mra->MainDb->RollbackTransaction();
    }

    try
    {
        mra->MainDb->BeginTransaction();
        Statement stt = mra->MainDb->Query(_T("INSERT INTO TableBitRate(ID, Value) VALUES(?, ?)"));
        for(unsigned int i = 0; i < all_bitrate.size(); i++)
        {
            if(pThreadTerminate) break;
            stt.Bind(1, (int)i);
            stt.Bind(2, all_bitrate.at(i));
            stt.Next();
        }
        mra->MainDb->CommitTransaction();
    }
    catch(const sqdb::Exception& e)
    {
        mra->MainDb->RollbackTransaction();
    }

    all_genre.clear();
    all_bitrate.clear();
    all_format.clear();
    //szText.clear();
    //szText.append(xml_node->GetNodeByIdx(7)->GetName());
    //szText.append(_T(" | "));
    //szText.append(xml_node->GetNodeByIdx(7)->GetValue());
    //TESTMESS(szText.c_str());
    DSetItemText(hDialogWin, _T("Готово!"), IDC_STATIC_PRORESS_2);
    if(pThreadTerminate) return;
}

unsigned AddRadioClass::UpdateBaseThread()
{
    pThreadTerminate = false;

    wstring icycast_bufstr(_T(""));
    while(!pThreadTerminate)
    {
        DSetItemText(hDialogWin, _T("Обновление базы IceCast"), IDC_STATIC_PRORESS);
        DSetItemText(hDialogWin, _T("Подключаюсь..."), IDC_STATIC_PRORESS_2);

        DWORD download_byte = 0;

        SockProxy *prs = new SockProxy();
        MyString str(_T("http://dir.xiph.org/yp.xml"));
        prs->PreFileDownload(str);
        prs->SetProxyfromConfig();
        int sockm = SOCKET_ERROR, serr;
        if(prs->StartFileDownload(sockm, serr))
        {
            GlobalConfig->MyAddLogParam(_T("SockOpen StartFileDownload s = %d, serr = %d"), sockm, serr);
            DSetItemText(hDialogWin, _T("Подключился"), IDC_STATIC_PRORESS_2);

            int fsize = prs->GetFileDownloadSize(), bytedn = 0, retb = 0;
            do
            {
                char buf[1024+1];
                retb = prs->SockRecv(sockm, (char*)buf, 1024);
                if(retb > 0)
                {
                    buf[retb] = 0;
                    //int length = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)buf, -1, 0, 0);
                    //wchar_t *output_buffer = new wchar_t[length];
                    //MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)buf, -1, output_buffer, length);
                    //TESTMESS(output_buffer);
                    //icycast_bufstr+=wstring(output_buffer);
                    //delete[] output_buffer;
                    icycast_bufstr.append(UTF8ToUTF16(buf));
                    bytedn += retb;
                    download_byte += retb;
                    wchar_t txt[32];
                    swprintf(txt, _T("Загрузка базы %d%%"), (bytedn*100 / fsize)); //Bytes2WString(bytedn).c_str()
                    //GlobalConfig->MyAddLog(txt);
                    DSetItemText(hDialogWin, txt, IDC_STATIC_PRORESS_2);
                    DSetItemText(hDialogWin, Bytes2WString(download_byte).c_str(), IDC_STATIC_RECV_MB);
                }
            }
            while(retb > 0);

            GlobalConfig->MyAddLog(_T("SockOpen while(retb > 0) end"));
            GlobalConfig->MyAddLogInt(icycast_bufstr.size());
        }
        else
        {
            if(serr == ERROR_SOCK_PROXY_AUTHFAILED)
                MyMessageBox(hDialogWin, _T("Ошибка авторизации прокси сервера.\r\nВключите авторизацию в настройках или проверьте правильность данных: пользователь/пароль"), MY_MES_ERROR);

            MYQueryStr ms;
            DSetItemText(hDialogWin, ms.Format(_T("Ошибка (%d) при подключении"), serr), IDC_STATIC_PRORESS);
            DSetItemText(hDialogWin, _T(""), IDC_STATIC_PRORESS_2);
        }
        prs->StopFileDownload(sockm);
        delete prs;

        if(icycast_bufstr.empty())
        {
            GlobalConfig->MyAddLog(_T("Error: buf str empty!"));
            if(!serr < 0)
            {
                DSetItemText(hDialogWin, _T("Не удалось скачать данные"), IDC_STATIC_PRORESS);
                DSetItemText(hDialogWin, _T(""), IDC_STATIC_PRORESS_2);
            }
            break;
        }

        /*wstring tmp1(mra->ConfigDir);
        tmp1+=_T("\\yp2.xml");
        FILE* f1 = _wfopen(tmp1.c_str(), _T("wtS, ccs=UTF-8")); //wtS, ccs=UTF-8
        // fwrite(&(bufstr.front()), sizeof(char), bufstr.size(), f1);
        fwrite(&(icycast_bufstr.front()), sizeof(wchar_t), icycast_bufstr.size(), f1);
        fclose(f1);*/

        mra->UpDateBaseProc = true;

        bool newbase = !mra->MainDb->TableExists(_T("BaseInfo"));
        GlobalConfig->MyAddLogParam(_T("Db data exists? = %i"), (int)!newbase);

        DSetItemText(hDialogWin, _T("Обновление базы радиостанций"), IDC_STATIC_PRORESS);
        DSetItemText(hDialogWin, _T(""), IDC_STATIC_PRORESS_2);
        newdatainbase = true;
        if(newbase)
        {
            SYSTEMTIME lt;
            GetLocalTime(&lt);
            mra->MainDb->Query(_T("CREATE TABLE BaseInfo(Ver INTEGER, RadioCount INTEGER, CreateTime INT64, LastModifiedTime INT64)")).Next();

            long long timecreate = SysTime264Time(&lt);
            Statement i = mra->MainDb->Query(_T("INSERT INTO BaseInfo values(?, ?, ?, ?)"));
            i.Bind(1, BASE_RADIO_VER); //Current version (in HIBYTE) and build (in LOBYTE).
            i.Bind(2, 0);
            i.Bind(3, timecreate);
            i.Bind(4, timecreate);
            i.Next();

            mra->MainDb->Query(_T("CREATE TABLE TableRadios(ID INTEGER PRIMARY KEY AUTOINCREMENT, SiteType INTEGER, RadioName TEXT COLLATE UNICODE, RadioURL TEXT COLLATE UNICODE, FormatIdx INTEGER, BitRateIdx INTEGER, GenreIdx TEXT)")).Next();
            //mra->MainDb->Query(_T("CREATE INDEX TableRadios_ID_index ON TableRadios(ID)")).Next();
            //mra->MainDb->Query(_T("CREATE INDEX TableRadios_RadioName_index ON TableRadios(RadioName)")).Next();
            mra->MainDb->Query(_T("CREATE TABLE TableGenres(ID INTEGER, Value TEXT COLLATE UNICODE)")).Next();
            mra->MainDb->Query(_T("CREATE TABLE TableFormats(ID INTEGER, Value TEXT)")).Next();
            mra->MainDb->Query(_T("CREATE TABLE TableBitRate(ID INTEGER, Value TEXT)")).Next();
            /*i.Next(); // следующая строка =>>>>
            i.Bind(1, 2323);
            i.Bind(2, 1992);
            i.Next();*/
        }
        else
        {
            SYSTEMTIME lt;
            GetLocalTime(&lt);
            long long timecreate = SysTime264Time(&lt);
            Statement i = mra->MainDb->Query(_T("UPDATE BaseInfo SET LastModifiedTime=?"));
            i.Bind(1, timecreate);
            i.Next();
        }

        //sqlite3_file.append(mra->ConfigDir);
        //sqlite3_file+=_T("\\myradio_base.db");
        //wstring tmp(mra->ConfigDir);
        //tmp+=_T("\\yp.xml");

        //wstring buffer;
        //FILE* f = _wfopen(tmp.c_str(), _T("rtS, ccs=UTF-8"));

        //struct _stat fileinfo;
        //_wstat(tmp.c_str(), &fileinfo);
        //size_t filesize = fileinfo.st_size;

        //if(filesize > 0)
        //{
        //    buffer.resize(filesize);
        //    size_t wchars_read = fread(&(buffer.front()), sizeof(wchar_t), filesize, f);
        //    buffer.resize(wchars_read);
        //    buffer.shrink_to_fit();
        //}

        //fclose(f);

        Cxml *xml = new Cxml();
        if(xml->ParseString(icycast_bufstr.c_str(), icycast_bufstr.size()))
        {
            GlobalConfig->MyAddLog(_T("xml parsestring return ok"));
            myfill(newbase, xml->GetRootNode()->GetNextChild()->GetNextChild());
        }
        delete xml;
        break;
    }

    mra->UpDateBaseProc = false;
    icycast_bufstr.clear();

    if(newdatainbase)
    {
        mra->GetCountListRadio(true);
        DSetItemText(hDialogWin, MyGetFormatDateAndTimeSystemFormat(&mra->current_base_info.time_create).c_str(), IDC_STATIC_BASE_CREATE);
        DSetItemText(hDialogWin, MyGetFormatDateAndTimeSystemFormat(&mra->current_base_info.time_modif).c_str(), IDC_STATIC_BASE_MODIF);
        DSetItemInt(hDialogWin, mra->current_base_info.radio_count, IDC_STATIC_BASE_COUNT);
        DSetItemInt(hDialogWin, mra->current_base_info.base_ver, IDC_STATIC_BASE_VER);
    }

    pThreadTerminate = true;
    CloseHandle(hUpdateThread);
    hUpdateThread = NULL;
    DSetItemText(hDialogWin, _T("Старт"), IDC_UPDATE_BASE_BTN_START);
    return 0;
}
