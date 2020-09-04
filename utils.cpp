#include "global.h"
#include "utils.h"
#include <Windows.h>
#include <commdlg.h>
#include "window.h"

void ferr(char *str)
{
	//printf("Error: %s\n", str);
	MessageBox(GetWindow(), str, "Fatal Error", 16);
	exit(-1);
}

void warn(char *str)
{
	MessageBox(GetWindow(), str, "Warning", 48);
}