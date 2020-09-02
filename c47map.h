#pragma once

struct Chunk;
struct GameObject;

struct Map_t
{
	Chunk *spkchk, *prot, *pclp;
	Chunk *phea, *pnam, *ppos, *pmtx, *pver, *pfac, *pftx, *puvc, *pdbl;
	GameObject *rootobj, *cliprootobj, *superroot;
};

void InitMap();

extern Map_t *Map;