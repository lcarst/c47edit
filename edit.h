#pragma once

struct Vector3;
struct GameObject;

struct Editor_t
{
	GameObject *selobj = NULL;
	GameObject *viewobj = NULL;
	Vector3 campos;
	Vector3 camori;
	float camspeed = 32;
	Vector3 cursorpos;

	bool findsel = false;

	bool cullBackfaces = true;
	bool wireframe = false;
	bool drawOutlines = true;
	bool drawSolid = true;
	bool drawNonSolid = true;
	bool drawBounds = false;
	bool drawGates = false;
	bool drawOther = false;

	bool rendertextures = false;
	int framespersec = 0;
};

extern Editor_t *Editor;

void InitEditor();
void Hide(GameObject *obj);
void UnHide(GameObject *obj);
void SaveScene();