#ifndef MYHDR_H_INCLUDED
#define MYHDR_H_INCLUDED

#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
#define UNICODE
#endif

/*#define STRICT
#define NOCOMM
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x500
//#define WINVER 0x0500
#define _WIN32_IE 0x0400*/


#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define STRICT
#define NOCOMM
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#undef __STRICT_ANSI__

#include <windows.h>
#include <winsock.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <mmsystem.h>

#include <stdio.h>
#include <tchar.h>
#include <string>
#include <math.h>
//#include <time.h>
#include <cassert>
//
#include <vector>
#include <map>

using std::wstring;

#endif // MYHDR_H_INCLUDED
