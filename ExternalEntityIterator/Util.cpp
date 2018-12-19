#include <Windows.h>
#include <stdio.h>
#include "Util.h"

bool StrCmpToLower(char* str1, char* str2 )
{
	DWORD i = 0;
	while (true)
	{
		if ( tolower(str1[i]) != tolower(str2[i]) )	
			return false;
		if (str1[i] == NULL ) //both should be the same so checking one is ok
			return true;
		i++;
	}
	return false;
}