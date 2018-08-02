// c47edit - Scene editor for HM C47
// Copyright (C) 2018 AdrienTD
// Licensed under the GPL3+.
// See LICENSE file for more details.

struct Mesh
{
	//float *vertices;
	//uint16_t *quadindices, *triindices;
	uint vertstart, quadstart, tristart, ftxo, numverts, numquads, numtris, weird;
	void Mesh::draw();
};

struct Light
{
	uint32_t param[7];
};

struct GameObject
{
	uint state;
	uint pdbloff, pexcoff;
	char *name;
	Matrix matrix;
	Vector3 position;
	uint type, flags;

	// Mesh
	Mesh *mesh;
	uint color;

	Light *light;

	std::vector<GameObject*> subobj;
	GameObject *parent;
	GameObject *root;

	GameObject(char *nName = "Unnamed", int nType = 0) : name(strdup(nName)), type(nType),
		pdbloff(0), pexcoff(0), flags(0), mesh(0), color(0), position(0,0,0), light(0), state(0), parent(0), root(0)
	{
		CreateIdentityMatrix(&matrix);
	}
	~GameObject() { free(name); }
};

extern Chunk *spkchk, *prot, *pclp, *phea, *pnam, *ppos, *pmtx, *pver, *pfac;
extern GameObject *rootobj, *cliprootobj, *superroot;
extern char *lastspkfn;

char* GetObjTypeString(uint ot);
void LoadSceneSPK(char *fn);
void ModifySPK();
void SaveSceneSPK(char *fn);
void RemoveObject(GameObject *o);
GameObject* DuplicateObject(GameObject *o, GameObject *parent = rootobj);
void GiveObject(GameObject *o, GameObject *t);
