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

bool ObjInObj(GameObject *a, GameObject *b)
{
	GameObject *o = a;
	while ( o = o->parent )
	{
		if ( o == b )
			return true;
	}
	return false;
}



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
	op = ImGui::TreeNodeEx(o, (o->subobj.empty() ? ImGuiTreeNodeFlags_Leaf : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((o == Editor->selobj) ? ImGuiTreeNodeFlags_Selected : 0), "%s(0x%X)::%s", GetObjTypeString(o->type), o->type, o->name);
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
			Editor->cursorpos = Editor->selobj->position;
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

Vector3 GetYXZRotVecFromMatrix(Matrix *m)
{
	float b = atan2(m->_31, m->_33);
	float j = atan2(m->_12, m->_22);
	float a = asin(-m->_32);
	return Vector3(a, b, j);
}


GameObject *objtogive = 0;

void IGObjectInfo()
{
	ImGui::SetNextWindowPos(ImVec2(1205, 3), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(270, 445), ImGuiCond_FirstUseEver);
	ImGui::Begin("Object information");
	if ( !Editor->selobj )
		ImGui::Text("No object selected.");
	else
	{

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
				Editor->selobj = Editor->selobj->parent;
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

		char tb[256]; tb[255] = 0; strcpy(tb, Editor->selobj->name);
		if ( ImGui::InputText("Name", tb, 255) )
		{
			free(Editor->selobj->name);
			Editor->selobj->name = strdup(tb);
		}
		ImGui::InputScalar("State", ImGuiDataType_U32, &Editor->selobj->state);
		ImGui::InputScalar("Type", ImGuiDataType_U32, &Editor->selobj->type);
		ImGui::InputScalar("Flags", ImGuiDataType_U32, &Editor->selobj->flags);
		ImGui::Separator();

		// TEMP: Disabled
		//ImGui::InputScalar("PDBL offset", ImGuiDataType_U32, &Editor->selobj->pdbloff, 0, 0, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
		//ImGui::InputScalar("PEXC offset", ImGuiDataType_U32, &Editor->selobj->pexcoff, 0, 0, "%08X", ImGuiInputTextFlags_CharsHexadecimal);

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
		ImGui::Text("Num. references: %u", Editor->selobj->refcount);
		if ( ImGui::CollapsingHeader("DBL") )
		{
			ImGui::InputScalar("Flags", ImGuiDataType_U32, &Editor->selobj->dblflags);
			int i = 0;
			for ( auto e = Editor->selobj->dbl.begin(); e != Editor->selobj->dbl.end(); e++ )
			{
				ImGui::PushID(i++);
				ImGui::Text("%1X", e->flags >> 4);
				ImGui::SameLine();
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
		if ( Editor->selobj->mesh )
			if ( ImGui::CollapsingHeader("Mesh") )
			{
				//ImGui::Separator();
				//ImGui::Text("Mesh");
				ImVec4 c = ImGui::ColorConvertU32ToFloat4(swap_rb(Editor->selobj->color));
				if ( ImGui::ColorEdit4("Color", &c.x, 0) )
					Editor->selobj->color = swap_rb(ImGui::ColorConvertFloat4ToU32(c));
				ImGui::Text("Vertex start index: %u", Editor->selobj->mesh->vertstart);
				ImGui::Text("Quad start index:   %u", Editor->selobj->mesh->quadstart);
				ImGui::Text("Tri start index:    %u", Editor->selobj->mesh->tristart);
				ImGui::Text("Vertex count: %u", Editor->selobj->mesh->numverts);
				ImGui::Text("Quad count:   %u", Editor->selobj->mesh->numquads);
				ImGui::Text("Tri count:    %u", Editor->selobj->mesh->numtris);
				ImGui::Text("FTXO offset: 0x%X", Editor->selobj->mesh->ftxo);
			}
		if ( Editor->selobj->light )
			if ( ImGui::CollapsingHeader("Light") )
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
		if ( wannadel )
		{
			if ( Editor->selobj->refcount > 0 )
				warn("It's not possible to remove an object that is referenced by other objects!");
			else
			{
				RemoveObject(Editor->selobj);
				Editor->selobj = 0;
			}
		}
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
	ImGui::DragFloat("Cam speed", &Editor->camspeed, 0.1f);
	ImGui::DragFloat3("Cam pos", &Editor->campos.x, 0.1f);
	ImGui::DragFloat2("Cam ori", &Editor->camori.x, 0.1f);
	ImGui::DragFloat3("Cursor pos", &Editor->cursorpos.x);
	ImGui::Checkbox("Wireframe", &Editor->wireframe);
	ImGui::SameLine();
	ImGui::Checkbox("Textured", &Editor->rendertextures);

	ImGui::Checkbox("Cull backfaces", &Editor->cullBackfaces);
	ImGui::SameLine();
	ImGui::Checkbox("Draw outlines", &Editor->drawOutlines);
	ImGui::Separator();
	ImGui::Checkbox("Draw solid", &Editor->drawSolid);
	ImGui::SameLine();
	ImGui::Checkbox("Draw non-solid", &Editor->drawNonSolid);
	ImGui::Checkbox("Draw bounds (28)", &Editor->drawBounds);
	ImGui::Checkbox("Draw gates (21)", &Editor->drawGates);
	ImGui::Checkbox("Draw other", &Editor->drawOther);

	ImGui::Text("FPS: %u", Editor->framespersec);
	ImGui::End();
}

uint curtexid = 0;

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
	IGObjectInfo();
#if 0
	IGTest();
#endif
}

void GuiEnd()
{
	ImGui::EndFrame();
}