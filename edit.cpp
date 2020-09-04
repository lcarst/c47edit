#include "global.h"
#include "edit.h"
#include <Windows.h>
#include <commdlg.h>

Editor_t *Editor;

void InitEditor()
{
	Editor = new Editor_t();
	Editor->campos = Vector3(0, 0, 0);
	Editor->camori = Vector3(0, 0, 0);
	Editor->cursorpos = Vector3(0, 0, 0);

}

void Hide(GameObject *obj)
{
	if ( !obj )
		return;
	obj->hidden = true;
	for ( auto e = obj->subobj.begin(); e != obj->subobj.end(); e++ )
		Hide(*e);
}

void UnHide(GameObject *obj)
{
	if ( !obj )
		return;
	obj->hidden = false;
	for ( auto e = obj->subobj.begin(); e != obj->subobj.end(); e++ )
		UnHide(*e);
}

void SaveScene()
{
		char newfn[300]; newfn[299] = 0;
		_splitpath(lastspkfn, 0, 0, newfn, 0);
		strcat(newfn, ".zip");
		char *s = strrchr(newfn, '@');
		if ( s )
			s += 1;
		else
			s = newfn;

		OPENFILENAME ofn; char zipfilename[1024];
		strcpy(zipfilename, s);
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = GetWindow();
		ofn.hInstance = GetModuleHandle(0);
		ofn.lpstrFilter = "Scene ZIP archive\0*.zip\0\0\0";
		ofn.lpstrFile = zipfilename;
		ofn.nMaxFile = 1023;
		ofn.lpstrTitle = "Save Scene ZIP archive as...";
		ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
		ofn.lpstrDefExt = "zip";
		if ( GetSaveFileName(&ofn) )
			SaveSceneSPK(zipfilename);
}