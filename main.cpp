// c47edit - Scene editor for HM C47
// Copyright (C) 2018 AdrienTD
// Licensed under the GPL3+.
// See LICENSE file for more details.

#include "global.h"
#include "video.h"
#include "texture.h"
#include "edit.h"
#include "c47map.h"
#include "gui.h"
#include "window.h"
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <commdlg.h>
#include <ctime>
#include <functional>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/imgui_impl_win32.h"


float objviewscale = 0.0f;

static GameObject *bestpickobj = 0;
static float bestpickdist;
static Vector3 bestpickintersectionpnt(0, 0, 0);

uint32_t framesincursec = 0, lastfpscheck;

#define swap_rb(a) ( (a & 0xFF00FF00) | ((a & 0xFF0000) >> 16) | ((a & 255) << 16) )



void RenderObject(GameObject *o, bool shade)
{
	glPushMatrix();
	glTranslatef(o->position.x, o->position.y, o->position.z);
	glMultMatrixf(o->matrix.v);

	bool ignore = ShouldIgnore(o);

	//if (o->mesh && (o->flags & 0x20)) {
	//if ( o->mesh && (o->flags & 0x20) && !ignore )
	if ( o->mesh && !ignore )
	{
		if ( !Editor->rendertextures )
		{
			uint clr = swap_rb(o->color);
			if ( shade )
			{
				glColor4ubv((uint8_t*)&clr);
			}
			else
			{
				if ( o == Editor->selobj )
				{
					glLineWidth(3);
					glColor4f(1, 1, 0, 1);
				}
				else
				{
					glLineWidth(1);
					glColor4f(0.9, 0.9, 0.9, 1);
				}
			}
		}
		o->mesh->draw();
	}
	for ( auto e = o->subobj.begin(); e != o->subobj.end(); e++ )
		RenderObject(*e, shade);
	glPopMatrix();
}

//int main(int argc, char* argv[])
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, char *args, int winmode)
{


	OPENFILENAME ofn; char zipfilename[1024]; zipfilename[0] = 0;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = GetModuleHandle(0);
	ofn.lpstrFilter = "Scene ZIP archive\0*.zip\0\0\0";
	ofn.lpstrFile = zipfilename;
	ofn.nMaxFile = 1023;
	ofn.lpstrTitle = "Select a Scene ZIP archive (containing Pack.SPK)";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = "zip";
	if ( !GetOpenFileName(&ofn) )
		exit(-1);

	InitEditor();
	InitMap();
	LoadSceneSPK(zipfilename);
	InitWindow();
	GuiSetup();

	bool appnoquit = true;
	lastfpscheck = GetTickCount();

	while ( appnoquit = HandleWindow() )
	{
		if ( win_minimized )
			Sleep(100);
		else
		{
			HandleInput();
			GuiBegin();
			GuiEnd();

			BeginDrawing();

			Matrix m1, m2, crm; Vector3 cd(0, 0, 1), ncd;
			CreateRotationXMatrix(&m1, Editor->camori.x);
			CreateRotationYMatrix(&m2, Editor->camori.y);
			MultiplyMatrices(&crm, &m1, &m2);
			TransformVector3(&ncd, &cd, &crm);

			glClearColor(0.5f, 0.0f, 0.0f, 1.0f);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glClearDepth(1.0f);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			Matrix lookat, persp;
			CreatePerspectiveMatrix(&persp, 60 * 3.141 / 180, (float)screen_width / (float)screen_height, 1, 20000);
			glMultMatrixf(persp.v);
			CreateLookAtLHViewMatrix(&lookat, &Editor->campos, &(Editor->campos + ncd), &Vector3(0, 1, 0));
			glMultMatrixf(lookat.v);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			float ovs = pow(2, objviewscale);
			glScalef(ovs, ovs, ovs);

			if ( Editor->cullBackfaces )
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
			}
			else
			{
				glDisable(GL_CULL_FACE);
			}

			glPolygonMode(GL_FRONT_AND_BACK, Editor->wireframe ? GL_LINE : GL_FILL);
			BeginMeshDraw();
			if ( Editor->viewobj )
			{
				//glTranslatef(-Editor->viewobj->position.x, -Editor->viewobj->position.y, -Editor->viewobj->position.z);
				RenderObject(Editor->viewobj, true);

				// Wireframe drawOutlines for flat-shaded 
				if ( Editor->drawOutlines && !Editor->wireframe )
				{
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					RenderObject(Editor->viewobj, false);
				}
			}

			glPointSize(10);
			glColor3f(1, 1, 1);
			glBegin(GL_POINTS);
			glVertex3f(Editor->cursorpos.x, Editor->cursorpos.y, Editor->cursorpos.z);
			glEnd();
			glPointSize(1);

			GuiRender();
			EndDrawing();

			//_sleep(16);

			framesincursec++;
			uint32_t newtime = GetTickCount();
			if ( (uint32_t)(newtime - lastfpscheck) >= 1000 )
			{
				Editor->framespersec = framesincursec;
				framesincursec = 0;
				lastfpscheck = newtime;
			}
		}
	}
}