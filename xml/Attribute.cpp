#include "Attribute.h"
//#ifdef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
//	#undef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
//	#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
//#endif
CAttribute::CAttribute(void)
: m_szValue(NULL)
, m_szName(NULL)
, m_bValAlloc(false)
, m_bNameAlloc(false)
{
}
CAttribute::CAttribute(CAttribute *a)
: m_szValue(NULL)
, m_szName(NULL)
, m_bValAlloc(false)
, m_bNameAlloc(false)
{
	if(a != NULL)
	{
		SetValue(a->GetValue());
		SetName(a->GetName());
	}
}

CAttribute::~CAttribute(void)
{
	if(m_bValAlloc)
		free(m_szValue);
	if(m_bNameAlloc)
		free(m_szName);
}

// Gets The name of the attribute
_TCHAR* CAttribute::GetName(void)
{
	return m_szName;
}

// Gets the value of the attribute
_TCHAR* CAttribute::GetValue(void)
{
	return m_szValue;
}

// Setter for the value of the attribute
bool CAttribute::SetValue(_TCHAR* szValue)
{
	if(szValue != NULL)
	{
		int l;
		l=lstrlen(szValue)*sizeof(_TCHAR);
		if(l == 0)
			return false;
		if(m_bValAlloc)
			free(m_szValue);
		m_szValue = (_TCHAR*)malloc(l+sizeof(_TCHAR));
		memset(m_szValue,_T('\0'),l+sizeof(_TCHAR));
		lstrcpy(m_szValue,szValue);
		m_bValAlloc = true;
		return true;
	}
	return false;
}

// Setter for the name of the attribute
bool CAttribute::SetName(_TCHAR* szName)
{
	if(szName != NULL)
	{
		int l;
		l=lstrlen(szName)*sizeof(_TCHAR);
		if(l == 0)
			return false;
		if(m_bNameAlloc)
			free(m_szName);
		m_szName = (_TCHAR*)malloc(l+sizeof(_TCHAR));
		memset(m_szName,_T('\0'),l+sizeof(_TCHAR));
		lstrcpy(m_szName,szName);
		m_bNameAlloc = true;
		return true;
	}
	return false;
}
bool CAttribute::operator < (CAttribute &a)
{
	if (lstrcmp(m_szName,a.GetName()) < 0)
		return true;
	return false;
}
bool CAttribute::operator == (CAttribute &a)
{
	if (lstrcmp(m_szName,a.GetName()) == 0)
		return true;
	return false;
}
bool CAttribute::operator > (CAttribute &a)
{
	if (lstrcmp(m_szName,a.GetName()) > 0)
		return true;
	return false;
}
bool CAttribute::operator == (_TCHAR* a)
{
	if (lstrcmp(m_szName,a) == 0)
		return true;
	return false;
}
CAttribute& CAttribute::operator=( CAttribute& a)
{
	SetName(a.GetName());
	SetValue(a.GetValue());
	return *this;
}
CAttribute* CAttribute::operator = ( CAttribute* a)
{
	SetName(a->GetName());
	SetValue(a->GetValue());
	return this;
}
