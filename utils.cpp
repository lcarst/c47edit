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

Vector3 GetYXZRotVecFromMatrix(Matrix *m)
{
	float b = atan2(m->_31, m->_33);
	float j = atan2(m->_12, m->_22);
	float a = asin(-m->_32);
	return Vector3(a, b, j);
}

float RoundToZero(float val, float error = 1e-6)
{
	return abs(val) < error ? 0.0 : val;
}