#include "toolutil.h"
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
	return MessageBox(0, msg, "Confirm", MB_YESNO) == IDYES;
}

int confirm_ync(const char *msg)
{
	int r = MessageBox(0, msg, "Confirm", MB_YESNOCANCEL);
	
	if (r == IDYES)
	{
		return 1;
	}
	if (r == IDNO)
	{
		return -1;
	}
	
	return 0;
	
}
