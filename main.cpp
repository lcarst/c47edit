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

GameObject *bestpickobj = 0;
float bestpickdist;
Vector3 bestpickintersectionpnt(0, 0, 0);

uint32_t framesincursec = 0, lastfpscheck; 

#define swap_rb(a) ( (a & 0xFF00FF00) | ((a & 0xFF0000) >> 16) | ((a & 255) << 16) )






void CamGoToPos(Vector3 newpos)
{
	Editor->campos = newpos;
	// Move back a bit so we can see it
	// TODO
}



GameObject* FindObjectNamed(char *name, GameObject *sup)
{
	if ( !strcmp(sup->name, name) )
		return sup;
	else
		for ( auto e = sup->subobj.begin(); e != sup->subobj.end(); e++ )
		{
			GameObject *r = FindObjectNamed(name, *e);
			if ( r ) return r;
		}
	return 0;
}

static bool shouldIgnore(GameObject *o)
{
	if ( o == NULL )
		return true;

	if ( o->hidden )
		return true;

	if ( o->type == ZSTDOBJ )
	{
		bool solid = (bool)o->dbl[4].u32;
		if ( solid && !Editor->drawSolid )
			return true;
		if ( !solid && !Editor->drawNonSolid )
			return true;
		return false;
	}
	if ( o->type == ZBOUNDS_28 && !Editor->drawBounds )
		return true;
	if ( o->type == ZGATE_21 && !Editor->drawGates )
		return true;
	if ( !Editor->drawOther && (o->type != ZSTDOBJ && o->type != ZBOUNDS_28 && o->type != ZGATE_21) )
		return true;




	return false;
}

void RenderObject(GameObject *o, bool shade)
{
	glPushMatrix();
	glTranslatef(o->position.x, o->position.y, o->position.z);
	glMultMatrixf(o->matrix.v);

	bool ignore = shouldIgnore(o);

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

Vector3 finalintersectpnt = Vector3(0, 0, 0);

bool IsRayIntersectingFace(Vector3 *raystart, Vector3 *raydir, int startvertex, int startface, int numverts, Matrix *worldmtx)
{
	uint16_t *bfac = (uint16_t*)Map->pfac->maindata + startface;
	float *bver = (float*)Map->pver->maindata + startvertex;

	Vector3 *pnts = new Vector3[numverts];
	for ( int i = 0; i < 3; i++ )
	{
		Vector3 v(bver[bfac[i] * 3 / 2], bver[bfac[i] * 3 / 2 + 1], bver[bfac[i] * 3 / 2 + 2]);
		TransformVector3(&pnts[i], &v, worldmtx);
	}

	Vector3 *edges = new Vector3[numverts];
	for ( int i = 0; i < 2; i++ )
		edges[i] = pnts[i + 1] - pnts[i];

	Vector3 planenorm = edges[1].cross(edges[0]);
	float planeord = -planenorm.dot(pnts[0]);

	float planenorm_dot_raydir = planenorm.dot(*raydir);

	// Only select by front faces if backfaces are culled
	if ( planenorm_dot_raydir >= 0 && Editor->cullBackfaces ) goto irifend;

	float param = -(planenorm.dot(*raystart) + planeord) / planenorm_dot_raydir;
	if ( param < 0 ) goto irifend;

	Vector3 interpnt = *raystart + *raydir * param;

	for ( int i = 3; i < numverts; i++ )
	{
		Vector3 v(bver[bfac[i] * 3 / 2], bver[bfac[i] * 3 / 2 + 1], bver[bfac[i] * 3 / 2 + 2]);
		TransformVector3(&pnts[i], &v, worldmtx);
	}

	for ( int i = 2; i < numverts - 1; i++ )
		edges[i] = pnts[i + 1] - pnts[i];
	edges[numverts - 1] = pnts[0] - pnts[numverts - 1];

	// Check if plane/ray intersection point is inside face

	for ( int i = 0; i < numverts; i++ )
	{
		Vector3 edgenorm = -planenorm.cross(edges[i]);
		Vector3 ptoi = interpnt - pnts[i];
		if ( edgenorm.dot(ptoi) < 0 )
			goto irifend;
	}

	finalintersectpnt = interpnt;
	delete[] pnts;
	delete[] edges;
	return true;

irifend:
	delete[] pnts;
	delete[] edges;
	return false;
}

GameObject *IsRayIntersectingObject(Vector3 *raystart, Vector3 *raydir, GameObject *o, Matrix *worldmtx)
{
	float d;
	Matrix objmtx = o->matrix;
	objmtx._41 = o->position.x;
	objmtx._42 = o->position.y;
	objmtx._43 = o->position.z;
	objmtx *= *worldmtx;

	bool ignore = shouldIgnore(o);
	if ( o->mesh && !ignore )
	{
		Mesh *m = o->mesh;
		for ( int i = 0; i < m->numquads; i++ )
			if ( IsRayIntersectingFace(raystart, raydir, m->vertstart, m->quadstart + i * 4, 4, &objmtx) )
				if ( (d = (finalintersectpnt - Editor->campos).sqlen2xz()) < bestpickdist )
				{
					bestpickdist = d;
					bestpickobj = o;
					bestpickintersectionpnt = finalintersectpnt;
				}
		for ( int i = 0; i < m->numtris; i++ )
			if ( IsRayIntersectingFace(raystart, raydir, m->vertstart, m->tristart + i * 3, 3, &objmtx) )
				if ( (d = (finalintersectpnt - Editor->campos).sqlen2xz()) < bestpickdist )
				{
					bestpickdist = d;
					bestpickobj = o;
					bestpickintersectionpnt = finalintersectpnt;
				}
	}
	for ( auto c = o->subobj.begin(); c != o->subobj.end(); c++ )
		IsRayIntersectingObject(raystart, raydir, *c, &objmtx);
	return 0;
}

//int main(int argc, char* argv[])
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, char *args, int winmode)
{
	InitMap();
	InitEditor();

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
	LoadSceneSPK(zipfilename);

	bool appnoquit = true;
	InitWindow();

	GuiSetup();

	lastfpscheck = GetTickCount();

	while ( appnoquit = HandleWindow() )
	{
		if ( win_minimized )
			Sleep(100);
		else
		{
			Matrix m1, m2, crm; Vector3 cd(0, 0, 1), ncd;
			CreateRotationXMatrix(&m1, Editor->camori.x);
			CreateRotationYMatrix(&m2, Editor->camori.y);
			MultiplyMatrices(&crm, &m1, &m2);
			//CreateRotationYXZMatrix(&crm, Editor->camori.y, Editor->camori.x, 0);
			TransformVector3(&ncd, &cd, &crm);
			Vector3 crabnn;
			Vec3Cross(&crabnn, &Vector3(0, 1, 0), &ncd);
			Vector3 crab = crabnn.normal();

			bool use_wasd = true;

			Vector3 cammove(0, 0, 0);
			ImGuiIO& io = ImGui::GetIO();
			if ( !io.WantCaptureKeyboard )
			{
				if ( use_wasd )
				{
					if ( io.KeysDown['A'] )
						cammove -= crab;
					if ( io.KeysDown['D'] )
						cammove += crab;
					if ( io.KeysDown['W'] )
						cammove += ncd;
					if ( io.KeysDown['S'] )
						cammove -= ncd;
					if ( io.KeysDown['E'] )
						cammove.y += 1;
					if ( io.KeysDown['C'] )
						cammove.y -= 1;
				}
				else
				{
					if ( io.KeysDown[VK_LEFT] )
						cammove -= crab;
					if ( io.KeysDown[VK_RIGHT] )
						cammove += crab;
					if ( io.KeysDown[VK_UP] )
						cammove += ncd;
					if ( io.KeysDown[VK_DOWN] )
						cammove -= ncd;
					if ( io.KeysDown['E'] )
						cammove.y += 1;
					if ( io.KeysDown['D'] )
						cammove.y -= 1;
				}

				if ( io.KeysDown['G'] )
				{
					Editor->campos = Editor->cursorpos;
				}

				if ( io.KeysDown['H'] )
				{
					Hide(Editor->selobj);
					//Editor->selobj = NULL;
				}

				if ( io.KeysDown['U'] )
				{
					UnHide(Editor->selobj);
				}

			}
			Editor->campos += cammove * Editor->camspeed * (io.KeyShift ? 2 : 1);
			if ( io.MouseDown[0] && !io.WantCaptureMouse && !(io.KeyAlt || io.KeyCtrl) )
			{
				Editor->camori.y += io.MouseDelta.x * 0.01f;
				Editor->camori.x += io.MouseDelta.y * 0.01f;
			}
			if ( !io.WantCaptureMouse && Editor->viewobj )
				if ( io.MouseClicked[1] || (io.MouseClicked[0] && (io.KeyAlt || io.KeyCtrl)) )
				{
					Matrix lookat, persp;
					CreatePerspectiveMatrix(&persp, 60 * 3.141 / 180, screen_width / screen_height, 1, 10000);
					CreateLookAtLHViewMatrix(&lookat, &Editor->campos, &(Editor->campos + ncd), &Vector3(0, 1, 0));
					Matrix matView = lookat * persp;

					Vector3 raystart, raydir;
					float ys = 1 / tan(60 * 3.141 / 180 / 2);
					float xs = ys / ((float)screen_width / (float)screen_height);
					ImVec2 mspos = ImGui::GetMousePos();
					float msx = mspos.x * 2.0f / (float)screen_width - 1.0f;
					float msy = mspos.y * 2.0f / (float)screen_height - 1.0f;
					Vector3 hi = ncd.cross(crab);
					raystart = Editor->campos + ncd + crab * (msx / xs) - hi * (msy / ys);
					raydir = raystart - Editor->campos;

					bestpickobj = 0;
					bestpickdist = HUGE_VAL; //100000000000000000.0f;
					Matrix mtx; CreateIdentityMatrix(&mtx);
					IsRayIntersectingObject(&raystart, &raydir, Editor->viewobj, &mtx);
					if ( io.KeyAlt )
					{
						if ( bestpickobj && Editor->selobj )
							Editor->selobj->position = bestpickintersectionpnt;
					}
					else
						Editor->selobj = bestpickobj;
					Editor->cursorpos = bestpickintersectionpnt;
				}

			GuiBegin();
			GuiEnd();


			BeginDrawing();
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

			ImGui::Render();
			ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
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