#include "../myhdr.h"
#include "Cxml.h"
#include "utils.h"

Cxml::Cxml(void)
    : //m_s64(NULL)
      m_iCursor(0)
    , m_iLength(0)
    , m_dversion(0.0)
{
    m_RootNode = new CNode();
}

Cxml::~Cxml(void)
{
    delete(m_RootNode);
}

// parse a string containg xml code
bool Cxml::ParseString(const wchar_t* szXML, int szXMLLength)
{
    if(szXML == NULL)
        return false;

    //m_iLength = szXMLLength;
    m_iLength = szXMLLength;
    //wchar_t buf[20];
    //swprintf(buf, _T("%i"), m_iLength);
    //MessageBox(0, buf, NULL, 0);
    //MessageBox(0, _T("TEST XML", NULL, 0);
    return GetNode(szXML);
}

bool Cxml::GetNode(const wchar_t* szXML)
{
    int k = m_iCursor;
    //int j = 0;//second level cursor;
    //bool bCDATA = false;
    bool bIsPI = false;//Set to true if the cursor is curently inside a processing instruction
    _TCHAR cDelim = 0;
    _TCHAR c = szXML[k];
    const _TCHAR COPEN = _T('<');
    const _TCHAR CCLOSE = _T('>');
    const _TCHAR CSLASH = _T('/');
    const _TCHAR CSPACE = _T(' ');
    const _TCHAR CQUOTE = _T('\'');
    const _TCHAR CDQUOTE = _T('\"');
    const _TCHAR CEQUAL = _T('=');
    const _TCHAR CNEW = _T('\n');
    const _TCHAR CTAB = _T('\t');
    //const _TCHAR CEXCLAMATION = _T('!');
    //const _TCHAR CMINUS = _T('-');
    //const _TCHAR CSQRPO = _T('[');
    //const _TCHAR CSQRPC = _T(']');
    //const _TCHAR SZCDATA[9] = _T("![CDATA[");
    const _TCHAR CQM = _T('?');
    const _TCHAR CRET = 13;//carriage return
    _TCHAR *szNodeNameBuff = (_TCHAR *)calloc(256, sizeof(_TCHAR));
    _TCHAR *szNodeValBuff = (_TCHAR *)calloc(256, sizeof(_TCHAR));
    _TCHAR *szAttrNameBuff = (_TCHAR *)calloc(256, sizeof(_TCHAR));
    _TCHAR *szAttrValBuff = (_TCHAR *)calloc(256, sizeof(_TCHAR));
    if(k >= m_iLength)
        return false;
    m_RootNode->SetName(_T("XML_DOC"));
    CNode* Current = m_RootNode->AdChildNode();
    while(k<m_iLength)
    {
        c = szXML[k];
        if(c == CNEW || c == CTAB || c == CRET)
        {
            k++;
            continue;
        }
        if(c == COPEN)
        {
            //memset(szNodeNameBuff,0,80*sizeof(_TCHAR));
            voidstr(szNodeNameBuff);
            if(szXML[k+1] == CSLASH)
            {
                //closing tag for the last opend node
                Current = Current->GetParent();
                k++;
                while(szXML[k] != CCLOSE)
                {
                    k++;
                }
                k++;
                continue;
            }
            if(szXML[k+1] == CQM)
            {
                c = szXML[++k];
                bIsPI = true;
            }
            // open tag. It means we have a node so we create it
            c = szXML[++k];
            while(c != CSLASH && c != CSPACE && c != CCLOSE)
            {
                //loops until the node name has been entirely read
                if(c != CNEW && c != CTAB && c != CRET)
                    szNodeNameBuff = concat(szNodeNameBuff,c);
                c = szXML[++k];
            }
            if(Current != NULL)//we have seted this node, navigate to a child of it
                if(Current->GetName() != NULL)//we have seted this node, navigate to a child of it
                    Current = Current->AdChildNode();

            Current->SetName(szNodeNameBuff);
            while(c == CSPACE)
            {
                c = szXML[++k];
                if(c == CSLASH)
                {
                    break;
                }
                if(c == CQM && bIsPI)
                {
                    break;
                }
                //memset(szAttrNameBuff,0,80*sizeof(_TCHAR));
                //memset(szAttrValBuff,0,80*sizeof(_TCHAR));
                voidstr(szAttrNameBuff);
                voidstr(szAttrValBuff);
                CAttribute* pA = new CAttribute();
                while(c != CEQUAL)
                {
                    //loops until the attribute name has been entirely read
                    if(c != CNEW && c != CTAB && c != CRET)
                        szAttrNameBuff = concat(szAttrNameBuff, c);
                    c = szXML[++k];
                }
                c = szXML[++k];
                if(c == CQUOTE || c == CDQUOTE)
                {
                    cDelim = c;
                    c = szXML[++k];
                }
                while(c != cDelim && cDelim != 0)
                {
                    //loops until the attribute value has been entirely read
                    if(c != CNEW && c != CTAB && c != CRET)
                        szAttrValBuff = concat(szAttrValBuff, c);
                    c = szXML[++k];
                }
                cDelim = 0;
                c = szXML[++k];
                pA->SetName(szAttrNameBuff);
                pA->SetValue(szAttrValBuff);
                Current->AddAttribute(pA);
            }
            if(c == CSLASH)
            {
                Current = Current->GetParent();
                c=szXML[++k];
                while(c != CCLOSE)
                {
                    c = szXML[++k];
                }
            }
            if(c == CQM && bIsPI)
            {
                Current->SetNodeAsPI();
                Current = Current->GetParent();
                c=szXML[++k];
                bIsPI = false;
                while(c != CCLOSE)
                {
                    c = szXML[++k];
                }
            }
            if(c == CCLOSE)
            {
                ;
            }
        }
        if(c != COPEN && c != CCLOSE && c != CSLASH/* && c != CSPACE*/)
        {
            //memset(szNodeValBuff,0,80*sizeof(_TCHAR));
            voidstr(szNodeValBuff);
            while(c != COPEN)
            {
                if(c != CNEW && c != CTAB && c != CRET/* && c != CSPACE*/)
                    szNodeValBuff =concat(szNodeValBuff,c);
                c = szXML[++k];
            }
            Current->SetValue(szNodeValBuff);
            continue;
        }
        k++;
    }
    free(szNodeNameBuff);
    free(szNodeValBuff);
    free(szAttrNameBuff);
    free(szAttrValBuff);
    return true;
}
CNode* Cxml::GetRootNode()
{
    return m_RootNode;
}
