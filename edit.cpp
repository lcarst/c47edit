#include "global.h"
#include "edit.h"
#include <Windows.h>
#include <commdlg.h>

Editor_t *Editor;
Options_t *Options;

void InitEditor()
{
	Options = new Options_t();


	Editor = new Editor_t();
	Editor->campos = Vector3(0, 0, 0);
	Editor->camori = Vector3(0, 0, 0);
	Editor->cursorpos = Vector3(0, 0, 0);
}

void CamGoToPos(Vector3 newpos)
{
	Editor->campos = newpos;
	// Move back a bit so we can see it
	// TODO
}

void EditSelect(GameObject *obj, bool setCursor)
{
	Editor->selobj = obj;
	if ( setCursor )
		Editor->cursorpos = Editor->selobj->position;
}

void EditSelectParent()
{
	if ( Editor->selobj && Editor->selobj->parent )
		Editor->selobj = Editor->selobj->parent;
}

void EditHide(GameObject *obj)
{
	if ( !obj )
		return;
	obj->hidden = true;
	for ( auto e = obj->subobj.begin(); e != obj->subobj.end(); e++ )
		EditHide(*e);
}

void EditUnHide(GameObject *obj)
{
	if ( !obj )
		return;
	obj->hidden = false;
	for ( auto e = obj->subobj.begin(); e != obj->subobj.end(); e++ )
		EditUnHide(*e);
}

void EditGoToObj()
{
	if ( Editor->selobj )
		Editor->campos = Editor->selobj->position;
}

void EditGoToCursor()
{
	if ( Editor->selobj )
		Editor->campos = Editor->cursorpos;
}

void EditPosition(Vector3 pos)
{
	// ...
}

void EditRotate(Vector3 rot)
{
	// ...
}

void EditDelete()
{
	if ( Editor->selobj->refcount > 0 )
		warn("It's not possible to remove an object that is referenced by other objects!");
	else
	{
		RemoveObject(Editor->selobj);
		Editor->selobj = 0;
	}
}

void EditDuplicate()
{
	if ( Editor->selobj->root )
		DuplicateObject(Editor->selobj, Editor->selobj->root);
}

static GameObject *objtogive = 0;
void EditSetToBeGiven()
{
	objtogive = Editor->selobj;
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

bool ShouldIgnore(GameObject *o)
{
	if ( o == NULL )
		return true;

	if ( o->hidden )
		return true;

	if ( o->type == ZSTDOBJ )
	{
		bool solid = (bool)o->dbl[4].u32;
		if ( solid && !Options->drawSolid )
			return true;
		if ( !solid && !Options->drawNonSolid )
			return true;
		return false;
	}
	if ( o->type == ZBOUNDS_28 && !Options->drawBounds )
		return true;
	if ( o->type == ZGATE_21 && !Options->drawGates )
		return true;
	if ( !Options->drawOther && (o->type != ZSTDOBJ && o->type != ZBOUNDS_28 && o->type != ZGATE_21) )
		return true;

	return false;
}