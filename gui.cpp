#include <Windows.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/imgui_impl_win32.h"
#include "global.h"
#include "c47map.h"
#include "edit.h"
#include "texture.h"
#include "window.h"

bool findsel = false;

#define swap_rb(a) ( (a & 0xFF00FF00) | ((a & 0xFF0000) >> 16) | ((a & 255) << 16) )

void IGOTNode(GameObject *o)
{
	bool op, colorpushed = 0;
	if ( o == Map->superroot )
		ImGui::SetNextTreeNodeOpen(true, ImGuiCond_Once);
	if ( findsel )
		if ( ObjInObj(Editor->selobj, o) )
			ImGui::SetNextTreeNodeOpen(true, ImGuiCond_Always);
	if ( o == Editor->viewobj )
	{
		colorpushed = 1;
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
	}
	else if ( o->hidden )
	{
		colorpushed = 1;
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1));
	}
	op = ImGui::TreeNodeEx(o, 
		(o->subobj.empty() ? ImGuiTreeNodeFlags_Leaf : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((o == Editor->selobj) ? ImGuiTreeNodeFlags_Selected : 0), "%s::%s", GetObjTypeStringNice(o->type), o->name);
	
	
	if ( colorpushed )
		ImGui::PopStyleColor();
	if ( findsel )
		if ( Editor->selobj == o )
			ImGui::SetScrollHere();
	if ( ImGui::IsItemHovered() && ImGui::IsMouseReleased(0) )
	{
		ImGuiIO& io = ImGui::GetIO();
		if ( io.KeyShift )
			Editor->viewobj = o;
		else
		{
			Editor->selobj = o;
			//Editor->cursorpos = Editor->selobj->position;
		}
	}
	if ( ImGui::IsItemActive() )
		if ( ImGui::BeginDragDropSource() )
		{
			ImGui::SetDragDropPayload("GameObject", &o, sizeof(GameObject*));
			ImGui::Text("GameObject: %s", o->name);
			ImGui::EndDragDropSource();
		}
	if ( op )
	{
		for ( auto e = o->subobj.begin(); e != o->subobj.end(); e++ )
			IGOTNode(*e);
		ImGui::TreePop();
	}
}

void IGObjectTree()
{
	ImGui::SetNextWindowPos(ImVec2(3, 3), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(316, 652), ImGuiCond_FirstUseEver);
	ImGui::Begin("Object tree", 0, ImGuiWindowFlags_HorizontalScrollbar);
	IGOTNode(Map->superroot);
	findsel = false;
	ImGui::End();
}




GameObject *objtogive = 0;

void IGEdit()
{
	ImGui::SetNextWindowPos(ImVec2(1205, 3), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(270, 445), ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit");
	if ( !Editor->selobj )
	{
		ImGui::Text("No object selected.");
		ImGui::End();
		return;
	}

	if ( ImGui::Button("Go to object") )
	{
		if ( Editor->selobj )
			Editor->campos = Editor->selobj->position;
	}
	ImGui::SameLine();
	if ( ImGui::Button("Go to cursor") )
	{
		if ( Editor->selobj )
			Editor->campos = Editor->cursorpos;
	}

	if ( ImGui::Button("Find in tree") )
		findsel = true;
	if ( Editor->selobj->parent )
	{
		ImGui::SameLine();
		if ( ImGui::Button("Select parent") )
		{
			if ( Editor->selobj->parent )
				Editor->selobj = Editor->selobj->parent;
		}
	}
	ImGui::Separator();
	
	bool wannadel = 0;
	if ( ImGui::Button("Delete") )
		wannadel = 1;

	ImGui::SameLine();
	if ( ImGui::Button("Duplicate") )
		if ( Editor->selobj->root )
			DuplicateObject(Editor->selobj, Editor->selobj->root);


	if ( ImGui::Button("Set to be given") )
		objtogive = Editor->selobj;
	ImGui::SameLine();
	if ( ImGui::Button("Give it here!") )
		if ( objtogive )
			GiveObject(objtogive, Editor->selobj);



	ImGui::Separator();
	ImGui::DragFloat3("Position", &Editor->selobj->position.x);
	/*for (int i = 0; i < 3; i++) {
		ImGui::PushID(i);
		ImGui::DragFloat3((i==0) ? "Matrix" : "", Editor->selobj->matrix.m[i]);
		ImGui::PopID();
	}*/

	Vector3 rota = GetYXZRotVecFromMatrix(&Editor->selobj->matrix);
	rota *= 180.0f / M_PI;
	if ( ImGui::DragFloat3("Orientation", &rota.x) )
	{
		rota *= M_PI / 180.0f;
		Matrix my, mx, mz;
		CreateRotationYMatrix(&my, rota.y);
		CreateRotationXMatrix(&mx, rota.x);
		CreateRotationZMatrix(&mz, rota.z);
		Editor->selobj->matrix = mz * mx * my;
	}

	if ( wannadel )
	{
		if ( Editor->selobj->refcount > 0 )
			Warn("It's not possible to remove an object that is referenced by other objects!");
		else
		{
			RemoveObject(Editor->selobj);
			Editor->selobj = 0;
		}
	}

	ImGui::End();
}


void IGObjectInfo()
{
	ImGui::SetNextWindowPos(ImVec2(1205, 3), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(270, 445), ImGuiCond_FirstUseEver);
	ImGui::Begin("Object information");
	if ( !Editor->selobj )
	{
		ImGui::Text("No object selected.");
		ImGui::End();
		return;
	}

	char tb[256]; tb[255] = 0; strcpy(tb, Editor->selobj->name);
	if ( ImGui::InputText("Name", tb, 255) )
	{
		free(Editor->selobj->name);
		Editor->selobj->name = strdup(tb);
	}
	ImGui::InputScalar("Type", ImGuiDataType_U32, &Editor->selobj->type);
	ImGui::InputScalar("State: On/Off", ImGuiDataType_U32, &Editor->selobj->state);
	ImGui::InputScalar("Flags", ImGuiDataType_U32, &Editor->selobj->flags);
	ImGui::Separator();

	// TEMP: Disabled
	//ImGui::InputScalar("PDBL offset", ImGuiDataType_U32, &Editor->selobj->pdbloff, 0, 0, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
	//ImGui::InputScalar("PEXC offset", ImGuiDataType_U32, &Editor->selobj->pexcoff, 0, 0, "%08X", ImGuiInputTextFlags_CharsHexadecimal);

	ImGui::Text("Num. references: %u", Editor->selobj->refcount);
	if ( ImGui::CollapsingHeader("DBL") )
	{
		ImGui::InputScalar("Flags", ImGuiDataType_U32, &Editor->selobj->dblflags);
		int i = 0;
		for ( auto e = Editor->selobj->dbl.begin(); e != Editor->selobj->dbl.end(); e++ )
		{
			ImGui::PushID(i++);
			//ImGui::Text("%1X", e->flags >> 4);
			//ImGui::SameLine();
			switch ( e->type )
			{
				case 1:
					ImGui::InputDouble("Double", &e->dbl); break;
				case 2:
					ImGui::InputFloat("Float", &e->flt); break;
				case 3:
				case 0xA:
				case 0xB:
				case 0xC:
				{
					char sb[10];
					sprintf(sb, "Int %X", e->type);
					ImGui::InputInt(sb, (int*)&e->u32); break;
				}
				case 4:
				case 5:
				{
					char sb[256];
					strncpy(sb, e->str, 255); sb[255] = 0;
					if ( ImGui::InputText((e->type == 5) ? "Filename" : "String", sb, 256) )
					{
						free(e->str);
						e->str = strdup(sb);
					}
					break;
				}
				case 6:
					ImGui::Separator(); break;
				case 7:
					ImGui::Text("Data (%X): %u bytes", e->type, e->datsize); break;
				case 8:
					if ( e->obj.valid() )
						ImGui::Text("Object: %s", e->obj->name);
					else
						ImGui::Text("Object: Invalid");
					if ( ImGui::BeginDragDropTarget() )
					{
						if ( const ImGuiPayload *pl = ImGui::AcceptDragDropPayload("GameObject") )
						{
							e->obj = *(GameObject**)pl->Data;
						}
						ImGui::EndDragDropTarget();
					}
					break;
				case 9:
					ImGui::Text("Objlist: %u objects", e->nobjs);
					ImGui::ListBoxHeader("Objlist", ImVec2(0, 64));
					for ( int i = 0; i < e->nobjs; i++ )
					{
						ImGui::Text("%s", e->objlist[i]->name);
						if ( ImGui::BeginDragDropTarget() )
						{
							if ( const ImGuiPayload *pl = ImGui::AcceptDragDropPayload("GameObject") )
							{
								e->objlist[i] = *(GameObject**)pl->Data;
							}
							ImGui::EndDragDropTarget();
						}
					}
					ImGui::ListBoxFooter();
					break;
				case 0x3F:
					ImGui::Text("End"); break;
				default:
					ImGui::Text("Unknown type %u", e->type); break;
			}
			ImGui::PopID();
		}
	}
	if ( ImGui::CollapsingHeader("Mesh") )
	{
		ImVec4 c = ImGui::ColorConvertU32ToFloat4(swap_rb(Editor->selobj->color));
		if ( ImGui::ColorEdit4("Color", &c.x, 0) )
			Editor->selobj->color = swap_rb(ImGui::ColorConvertFloat4ToU32(c));

		if ( Editor->selobj->mesh )
		{

			ImGui::Text("Vertex count: %u", Editor->selobj->mesh->numverts);
			ImGui::Text("Quad count:   %u", Editor->selobj->mesh->numquads);
			ImGui::Text("Tri count:    %u", Editor->selobj->mesh->numtris);
			ImGui::Text("Vertex start index: %u", Editor->selobj->mesh->vertstart);
			ImGui::Text("Quad start index:   %u", Editor->selobj->mesh->quadstart);
			ImGui::Text("Tri start index:    %u", Editor->selobj->mesh->tristart);
			ImGui::Text("FTXO offset: 0x%X", Editor->selobj->mesh->ftxo);
		}
	}
	if ( ImGui::CollapsingHeader("Light") )
	{
		if ( Editor->selobj->light )
		{
			//ImGui::Separator();
			//ImGui::Text("Light");
			char s[] = "Param ?\0";
			for ( int i = 0; i < 7; i++ )
			{
				s[6] = '0' + i;
				ImGui::InputScalar(s, ImGuiDataType_U32, &Editor->selobj->light->param[i], 0, 0, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
			}
		}
		else
			ImGui::Text("None");
	}

	ImGui::End();
}

void IGMain()
{
	ImGui::SetNextWindowPos(ImVec2(1005, 453), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(270, 203), ImGuiCond_FirstUseEver);
	ImGui::Begin("c47edit");
	ImGui::Text("c47edit - Version " APP_VERSION);
	if ( ImGui::Button("Save Scene") )
	{
		SaveScene();
	}
	ImGui::SameLine();
	if ( ImGui::Button("About...") )
		MessageBox(GetWindow(), "c47edit\nUnofficial scene editor for \"Hitman: Codename 47\"\n\n"
				   "(C) 2018 AdrienTD\nLicensed under the GPL 3.\nSee LICENSE file for details.\n\n"
				   "3rd party libraries used:\n- Dear ImGui (MIT license)\n- Miniz (MIT license)\nSee LICENSE_* files for copyright and licensing of these libraries.", "c47edit", 0);
	//ImGui::DragFloat("Scale", &objviewscale, 0.1f);
	ImGui::DragFloat("Cam speed", &Options->camspeed, 0.1f);
	ImGui::DragFloat3("Cam pos", &Editor->campos.x, 0.1f);
	//ImGui::DragFloat2("Cam ori", &Editor->camori.x, 0.1f);
	ImGui::DragFloat3("Cursor pos", &Editor->cursorpos.x);
	ImGui::Checkbox("Wireframe", &Options->wireframe);
	//ImGui::SameLine();
	//ImGui::Checkbox("Textured", &Options->rendertextures);

	ImGui::Checkbox("Cull backfaces", &Options->cullBackfaces);
	ImGui::SameLine();
	ImGui::Checkbox("Draw outlines", &Options->drawOutlines);
	ImGui::Separator();
	ImGui::Text("Show/hide objects:");
	ImGui::Checkbox("Solid", &Options->drawSolid);
	ImGui::Checkbox("Non-solid", &Options->drawNonSolid);
	ImGui::Checkbox("Bounds (28)", &Options->drawBounds);
	ImGui::Checkbox("Gates (21)", &Options->drawGates);
	ImGui::Checkbox("Other", &Options->drawOther);
	ImGui::Checkbox("Point objects", &Options->drawPointObjects);


	ImGui::Text("FPS: %u", Editor->framespersec);
	ImGui::End();
}

static uint curtexid = 0;
bool filterDbl = false;
int dblToFilter = 225;
void IGTest()
{
	ImGui::Begin("Debug/Test");
	if ( ImGui::Button("ReadTextures()") )
		ReadTextures();
	static const int one = 1;
	ImGui::InputScalar("Texture ID", ImGuiDataType_U32, &curtexid, &one);
	auto l = texmap.lower_bound(curtexid);
	if ( ImGui::Button("Next") )
		if ( l != texmap.end() )
		{
			auto ln = std::next(l);
			if ( ln != texmap.end() )
				curtexid = ln->first;
		}
	auto t = texmap.find(curtexid);
	if ( t != texmap.end() )
		ImGui::Image(t->second, ImVec2(256, 256));
	else
		ImGui::Text("Texture %u not found.", curtexid);

	ImGui::Separator();
	ImGui::Checkbox("Toggle dbl", &filterDbl);
	ImGui::End();
}

void GuiSetup()
{
	ImGui::CreateContext(0);
	ImGui_ImplWin32_Init((void*)GetWindow());
	ImGui_ImplOpenGL2_Init();
}

void GuiBegin()
{
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	IGMain();
	IGObjectTree();
	IGEdit();
	IGObjectInfo();
	IGTest();
}

void GuiEnd()
{
	ImGui::EndFrame();
}

void GuiRender()
{
	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

static GameObject *bestpickobj = 0;
static float bestpickdist;
static Vector3 bestpickintersectionpnt(0, 0, 0);
static Vector3 finalintersectpnt = Vector3(0, 0, 0);

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
	if ( planenorm_dot_raydir >= 0 && Options->cullBackfaces ) goto irifend;

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

	bool ignore = ShouldIgnore(o);
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

void HandleInput()
{
	Matrix m1, m2, crm; Vector3 cd(0, 0, 1), ncd;
	CreateRotationXMatrix(&m1, Editor->camori.x);
	CreateRotationYMatrix(&m2, Editor->camori.y);
	MultiplyMatrices(&crm, &m1, &m2);
	TransformVector3(&ncd, &cd, &crm);
	Vector3 crabnn;
	Vec3Cross(&crabnn, &Vector3(0, 1, 0), &ncd);
	Vector3 crab = crabnn.normal();

	Vector3 cammove(0, 0, 0);
	ImGuiIO& io = ImGui::GetIO();
	if ( !io.WantCaptureKeyboard )
	{
		if ( ImGui::IsKeyDown('A') )
			cammove -= crab;
		if ( ImGui::IsKeyDown('D') )
			cammove += crab;
		if ( ImGui::IsKeyDown('W') )
			cammove += ncd;
		if ( ImGui::IsKeyDown('S') )
			cammove -= ncd;
		if ( ImGui::IsKeyDown('E') )
			cammove.y += 1;
		if ( ImGui::IsKeyDown('C') )
			cammove.y -= 1;

		if ( ImGui::IsKeyPressed('G') )
			Editor->campos = Editor->cursorpos;

		if ( ImGui::IsKeyPressed('H') )
			EditHide(Editor->selobj);

		if ( ImGui::IsKeyPressed('U') )
			EditUnHide(Editor->selobj);

		if ( ImGui::IsKeyPressed('P') )
		{
			if ( Editor->selobj->parent )
				Editor->selobj = Editor->selobj->parent;
		}

		if ( ImGui::IsKeyPressed(VK_DELETE) )
			EditDelete();

		if ( ImGui::IsKeyPressed(VK_ESCAPE) )
			Editor->selobj = NULL;
	}

	Editor->campos += cammove * Options->camspeed * (io.KeyShift ? 2 : 1);
	if ( io.MouseDown[0] && !io.WantCaptureMouse && !(io.KeyAlt || io.KeyCtrl) )
	{
		Editor->camori.y += io.MouseDelta.x * 0.01f;
		Editor->camori.x += io.MouseDelta.y * 0.01f;
	}
	if ( !io.WantCaptureMouse && Editor->viewobj )
	{
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

			for ( int i = 0; i < 3; i++ )
				bestpickintersectionpnt.c[i] = round(bestpickintersectionpnt.c[i]);

			if ( io.KeyAlt )
			{
				if ( bestpickobj && Editor->selobj )
				{
					float oldy = Editor->selobj->position.y;
					Editor->selobj->position = bestpickintersectionpnt;
					Editor->selobj->position.y = oldy;
					
				}
			}
			else
				Editor->selobj = bestpickobj;
			Editor->cursorpos = bestpickintersectionpnt;
		}
	}
}