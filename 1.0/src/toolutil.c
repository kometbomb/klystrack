/*
Copyright (c) 2009 Tero Lindeman (kometbomb)

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "toolutil.h"
#include "msgbox.h"
#include <windows.h>


FILE *open_dialog(const char *mode, wchar_t *title, wchar_t *filter)
{
	OPENFILENAMEW ofn;
	memset(&ofn,0,sizeof(OPENFILENAMEW));
	ofn.lStructSize = sizeof(OPENFILENAMEW);
	wchar_t szFile[5000];
	szFile[0]=L'\0';
	ofn.lpstrFile = szFile;
	ofn.hwndOwner=0;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrTitle = title;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.Flags =OFN_HIDEREADONLY | ((mode[0] == 'r')?OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST:OFN_OVERWRITEPROMPT);
	
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 2;
	
	wchar_t *wmode;
	
	if (strcmp(mode,"r") == 0)
		wmode = L"r";
	else if (strcmp(mode,"rb") == 0)
		wmode = L"rb";
	else if (strcmp(mode,"w") == 0)
		wmode = L"w";
	else if (strcmp(mode,"wb") == 0)
		wmode = L"wb";
	else return NULL;
		
	if (mode[0] == 'w')
	{
		if (GetSaveFileNameW(&ofn))
			return _wfopen(szFile, wmode);
		else
			return NULL;
	}
	else	
	{
		if (GetOpenFileNameW(&ofn)) {
			return _wfopen(szFile, wmode);
		}
		else
			return NULL;

	}
	
	return NULL;
}


int confirm(const char *msg)
{
	return msgbox(msg, YES|NO) == YES; // MessageBox(0, msg, "Confirm", MB_YESNO) == IDYES;
}

int confirm_ync(const char *msg)
{
	int r = msgbox(msg, YES|NO|CANCEL);
	
	if (r == YES)
	{
		return 1;
	}
	if (r == NO)
	{
		return -1;
	}
	
	return 0;
	
}
