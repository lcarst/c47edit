#include "global.h"
#include "edit.h"


Editor_t *Editor;

void InitEditor()
{
	Editor = new Editor_t();
	Editor->campos = Vector3(0, 0, 0);
	Editor->camori = Vector3(0, 0, 0);

}