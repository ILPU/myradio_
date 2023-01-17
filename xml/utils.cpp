#include "utils.h"

_TCHAR* concat(_TCHAR *str, _TCHAR c)
{
	_TCHAR buf[2] ={0};
	//_TCHAR *ret = NULL;
	buf[0] = c;
	if((lstrlen(str) + (2*sizeof(_TCHAR))) < 256)
	{
		lstrcat(str,buf);
		return str;
	}
	str = (_TCHAR*)realloc(str,(lstrlen(str) + 2)*sizeof(_TCHAR));
	lstrcat(str,buf);
	return str;
}
void copyx(_TCHAR* dst, _TCHAR* src, int start, int stop)
{
	memcpy(dst, &src[start], (stop - start + 1)*sizeof(_TCHAR));
}
void voidstr(_TCHAR* str)
{
	memset(str,0,lstrlen(str)*sizeof(_TCHAR));
}
