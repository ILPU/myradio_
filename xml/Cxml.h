#pragma once
#include "utils.h"
/*
#define UNKNOWN			0
#define UTF_8			1
#define UTF_16			2
#define UTF_32			3
#define UTF_EBCDIC		4
#define GB_18030		5
#define ASCII			6
#define UNICODE_UNKNOWN	7
*/

class Cxml
{
public:
	Cxml(void);
	~Cxml(void);
	bool ParseString(const wchar_t* szXML, int szXMLLength);

private:

	int m_iCursor;
	int m_iLength;
	//int m_iencoding;
	// searches for the encoding statement
	double m_dversion;
	CNode* m_RootNode;
	bool GetNode(const wchar_t* szXML);
public:
	CNode* GetRootNode();
};
