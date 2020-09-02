#pragma once

struct Vector3;
struct GameObject;

struct Editor_t
{
	GameObject *selobj = NULL;
	GameObject *viewobj = NULL;
	Vector3 campos;
	Vector3 camori;

	bool wireframe = false;
	bool findsel = false;

	bool cullBackfaces = true;
	bool outline = true;
	bool drawSolid = true;
	bool drawNonSolid = true;
	bool drawBounds = false;
	bool drawGates = false;
	bool drawOther = false;
};

extern Editor_t *Editor;

void InitEditor();
