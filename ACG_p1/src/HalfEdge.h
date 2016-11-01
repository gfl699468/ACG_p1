#pragma once
#include "Vertex.h"
#include "Face.h"

class HalfEdge;

class HalfEdge
{
public:

	HalfEdge()
	{
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

