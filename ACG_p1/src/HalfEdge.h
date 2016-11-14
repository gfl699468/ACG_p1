#pragma once
#include "Vertex.h"
#include "Face.h"

#define nil_pair make_pair(-1, -1)

class HalfEdge;

class HalfEdge
{
public:

	HalfEdge()
	{
		oriVertex = -1;
		pairEdge = nil_pair;
		nextHalfEdge = nil_pair;
		prevHalfEdge = nil_pair;
	}

	HalfEdge(int orivertex) {

	}

	~HalfEdge()
	{
	}
	int ori, dest;
	pair<int, int> pairEdge;
	pair<int, int> nextHalfEdge;
	pair<int, int> prevHalfEdge;
	int oriVertex;
	int face;

};

