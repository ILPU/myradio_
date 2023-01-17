#include "Node.h"

CNode::CNode(void)
    : m_bEmptyAttrList(true)
    , m_szValue(NULL)
    , m_bValAlloc(false)
    , m_bNameAlloc(false)
    , m_bIsComment(false)
    , m_szName(NULL)
    , m_parrent(NULL)
    , m_bIsPI(false)
{
    m_NodeIterator = m_NodeList.begin();
    m_AttrIterator = m_AttributeList.begin();
}
CNode::CNode(CNode* node)
    : m_bEmptyAttrList(true)
    , m_szValue(NULL)
    , m_bValAlloc(false)
    , m_bNameAlloc(false)
    , m_bIsComment(false)
    , m_szName(NULL)
    , m_parrent(NULL)
{
    if(node != NULL)
    {
        SetValue(node->GetValue());
        SetName(node->GetName());
        m_AttributeList = node->GetAttributeList();
        m_NodeList = node->GetNodeList();
    }
    m_NodeIterator = m_NodeList.begin();
    m_AttrIterator = m_AttributeList.begin();
}

CNode::~CNode(void)
{
    if(m_bValAlloc)
        free(m_szValue);
    if(m_bNameAlloc)
        free(m_szName);
    if(m_bEmptyAttrList == false)
    {
        it = m_AttributeList.begin();
        while (it != m_AttributeList.end())
        {
            delete(*it);
            it++;
        }
    }
    i = m_NodeList.begin();
    while(i != m_NodeList.end())
    {
        delete(*i);
        i++;
    }
}

int  CNode::GetAttributeListSize()
{
    return m_AttributeList.size();
}

int  CNode::GetNodeListSize()
{
    return m_NodeList.size();
}

CNode* CNode::GetNodeByIdx(int Idx)
{
    std::list<CNode*>::iterator m_GetNodeIt;
    m_GetNodeIt = m_NodeList.begin();

    int i = 0;
    for(m_GetNodeIt=m_NodeList.begin(); m_GetNodeIt!=m_NodeList.end(); ++m_GetNodeIt)
    {
        if(i == Idx)
        {
            return (CNode*)*m_GetNodeIt;
        }
        i++;
    }
    return NULL;
}

// Adds an attribute to the attribute list
bool CNode::AddAttribute(_TCHAR* szAttrName, _TCHAR* szValue)
{
    if(szAttrName == NULL)
        return false;
    if(lstrlen(szAttrName) == 0)
        return false;

    CAttribute* attribute = new CAttribute();
    attribute->SetName(szAttrName);
    if(szValue!=NULL)
        if(lstrlen(szValue)!= 0)
            attribute->SetValue(szValue);
    m_AttributeList.push_front(attribute);
    m_AttrIterator = m_AttributeList.begin();
    m_bEmptyAttrList = false;
    return true;
}
bool CNode::AddAttribute(CAttribute* pAttribute)
{
    if(pAttribute == NULL)
        return false;
    m_AttributeList.push_front(pAttribute);
    m_AttrIterator = m_AttributeList.begin();
    m_bEmptyAttrList = false;
    return true;
}

// Getter for the attribut by name
CAttribute* CNode::GetAttribute(_TCHAR* szName)
{
    it = m_AttributeList.begin();
    while (it != m_AttributeList.end())
    {
        if(lstrcmp((*it)->GetName(), szName) == 0)
        {
            return ((CAttribute*) *it);
            break;
        }
        it++;
    }
    return NULL;
}

// Gets the next attribute in the list, starting from head and increments the counter
CAttribute* CNode::GetNextAttribute(void)
{
    if(m_AttrIterator != m_AttributeList.end())
    {
        CAttribute* res = (CAttribute*)*m_AttrIterator;
        m_AttrIterator++;
        return res;
    }
    return NULL;
}

// Gets the previous attribute in the list and decrements the counter
CAttribute* CNode::GetPreviousAttribute(void)
{
    m_AttrIterator--;
    if(m_AttrIterator != m_AttributeList.begin())
    {
        CAttribute* res = (CAttribute*)*m_AttrIterator;
        return res;
    }
    return NULL;
}

wchar_t* CNode::GetValue(void)
{
    if(m_szValue)
        return m_szValue;
    else return _T("");
}

bool CNode::SetValue(const wchar_t* szValue)
{
    if(szValue != NULL)
    {
        register int l;
        l=lstrlen(szValue)*sizeof(_TCHAR);
        if(l == 0)
            return false;
        if(m_bValAlloc)
            free(m_szValue);
        m_szValue = (_TCHAR*)malloc(l+sizeof(_TCHAR));
        memset(m_szValue, _T('\0'), l+sizeof(_TCHAR));
        lstrcpy(m_szValue, szValue);
        m_bValAlloc = true;
        return true;
    }
    return false;
}

// adds a child node to this one
CNode* CNode::AdChildNode(CNode* child)
{
    if (child == NULL)
        return NULL;
    m_NodeList.push_front(child);
    i = m_NodeList.begin();
    return (*i);
}

bool CNode::SetName(const wchar_t* szName)
{
    if(szName != NULL)
    {
        register int l;
        l=lstrlen(szName)*sizeof(_TCHAR);
        if(l == 0)
            return false;
        if(m_bNameAlloc)
            free(m_szName);
        m_szName = (_TCHAR*)malloc(l+sizeof(_TCHAR));
        memset(m_szName, _T('\0'), l+sizeof(_TCHAR));
        lstrcpy(m_szName, szName);
        m_bNameAlloc = true;
        return true;
    }
    return false;
}

CNode* CNode::GetParent(void)
{
    return m_parrent;
}

bool CNode::SetParrent(CNode* pnode)
{
    if(pnode == NULL)
        return false;
    m_parrent = pnode;
    return true;
}

bool CNode::IsRoot(void)
{
    return (m_parrent == NULL);
}

wchar_t* CNode::GetName(void)
{
    if(m_szName)
        return m_szName;
    else return _T("");
}

CNode* CNode::AdChildNode()
{
    CNode* child = new CNode();
    child->SetParrent(this);
    m_NodeList.push_front(child);
    i = m_NodeList.begin();
    m_NodeIterator = m_NodeList.begin();
    return (*i);
}

CNode* CNode::GetPreviousChild(void)
{
    m_NodeIterator--;
    if(m_NodeIterator != m_NodeList.begin())
    {
        CNode* res =   (CNode*)*m_NodeIterator;
        return res;
    }
    return NULL;
}

CNode* CNode::GetNextChild(void)
{
    if(m_NodeIterator != m_NodeList.end())
    {
        CNode* res =  (CNode*)*m_NodeIterator;
        m_NodeIterator++;
        return res;
    }
    return NULL;
}
CNode& CNode::operator=(CNode& node)
{
    SetName(node.GetName());
    SetValue(node.GetValue());
    m_NodeList = node.GetNodeList();
    m_AttributeList = node.GetAttributeList();
    return *this;
}
CNode* CNode::operator = (CNode* node)
{
    SetName(node->GetName());
    SetValue(node->GetValue());
    m_NodeList = node->GetNodeList();
    m_AttributeList = node->GetAttributeList();
    return this;
}

std::list<CAttribute*> CNode::GetAttributeList()
{
    return m_AttributeList;
}
std::list<CNode*> CNode::GetNodeList()
{
    return m_NodeList;
}
bool CNode::SetComment(const wchar_t* szComment)
{
    SetName(_T("Comment"));
    SetValue(szComment);
    m_bIsComment = true;
    return true;
}
bool CNode::IsCommentNode(void)
{
    return m_bIsComment;
}

bool CNode::SetNodeAsPI(void)
{
    m_bIsPI = true;
    return m_bIsPI;
}

bool CNode::IsPINode(void)
{
    return false;
}
