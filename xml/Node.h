#pragma once
#include "Attribute.h"
#include <list>

class CNode
{
public:
	CNode(void);
	CNode(CNode* node);
	~CNode(void);
private:
	std::list<CAttribute*> m_AttributeList;
	std::list<CAttribute*>::iterator it;//used for various functions
	std::list<CAttribute*>::iterator m_AttrIterator;//global iterator for get next/prev attribute
	std::list<CNode*> m_NodeList;
	std::list<CNode*>::iterator m_NodeIterator;//global iterator for get next/prev node
	std::list<CNode*>::iterator i;//used for various functions
	std::list<_TCHAR*> m_CommentList;
	std::list<_TCHAR*>::iterator m_CommentIterator; //Global iterator for GetNext/prev comment
	std::list<_TCHAR*>::iterator ic; //Global iterator for GetNext/prev comment
	std::list<CAttribute*> GetAttributeList();//{return m_AttributeList;};
	std::list<CNode*> GetNodeList();//{return m_NodeList;);

public:
	// Adds an attribute to the attribute list
	bool AddAttribute(_TCHAR* szAttrName, _TCHAR* szValue=NULL);
	bool AddAttribute(CAttribute* pAttribute);
	bool SetValue(const wchar_t* szValue);
	bool SetName(const wchar_t* szName);
	bool SetParrent(CNode* pnode);
	bool IsRoot(void);
	bool SetComment(const wchar_t* szComment);
	bool SetNodeAsPI(void);
	bool IsPINode(void);
	bool IsCommentNode(void);
	int  GetAttributeListSize();
	int  GetNodeListSize();
	CNode* GetNodeByIdx(int Idx);
	// Getter for the attribut by name
	CAttribute* GetAttribute(_TCHAR* szName);
	// Gets the next attribute in the list, starting from head and increments the counter
	CAttribute* GetNextAttribute(void);
	// Gets the previous attribute in the list and decrements the counter
	CAttribute* GetPreviousAttribute(void);
	// //adds a child node to this one
	CNode &operator = ( CNode& a);
	CNode *operator = ( CNode* a);
	CNode* AdChildNode(void);
	CNode* GetNextChild(void);
	CNode* GetPreviousChild(void);
	CNode* GetParent(void);
	CNode* AdChildNode(CNode* child);
	wchar_t* GetValue(void);
	wchar_t* GetName(void);
private:
	bool m_bEmptyAttrList;
	//flag set for the allocation of space for tha value
	bool m_bValAlloc;
	bool m_bNameAlloc;
	bool m_bIsComment;
	bool m_bIsPI;
	wchar_t* m_szName;
	wchar_t* m_szValue;
	CNode* m_parrent;
};
