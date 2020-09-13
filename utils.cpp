#include "global.h"
#include "utils.h"
#include "edit.h"
#include <Windows.h>
#include <commdlg.h>
#include "c47map.h"
#include "window.h"

void Error(char *str)
{
	//printf("Error: %s\n", str);
	MessageBox(GetWindow(), str, "Fatal Error", 16);
	exit(-1);
}

void Warn(char *str)
{
	MessageBox(GetWindow(), str, "Warning", 48);
}

