#pragma once

struct Vector3;
struct GameObject;

struct Editor_t
{
	GameObject *selobj = NULL;
	GameObject *viewobj = NULL;
	Vector3 campos;
	Vector3 camori;

	Vector3 cursorpos;


	int framespersec = 0;
};

struct Options_t
{
	float camspeed = 32;
	bool cullBackfaces = true;
	bool wireframe = false;
	bool drawOutlines = true;
	bool drawSolid = true;
	bool drawNonSolid = true;
	bool drawBounds = false;
	bool drawGates = false;
	bool drawOther = false;
	bool drawPointObjects = true;
	bool rendertextures = false;
};

extern Editor_t *Editor;
extern Options_t *Options;

void InitEditor();
void CamGoToPos(Vector3 newpos);
void EditSelect(GameObject *obj, bool setCursor);
void EditSelectParent();
void EditHide(GameObject *obj);
void EditUnHide(GameObject *obj);
void EditGoToObj();
void EditGoToCursor();
void EditPosition(Vector3 pos);
void EditRotate(Vector3 rot);
void EditDelete();
void EditDuplicate();
void EditSetToBeGiven();
void SaveScene();
bool ShouldIgnore(GameObject *o);