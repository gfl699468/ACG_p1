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

	HalfEdge(Vertex* orivertex) {

	}

	~HalfEdge()
	{
	}
	int ori, dest;
	HalfEdge* pair;
	HalfEdge* nextHalfEdge;
	Vertex* oriVertex;
	Face* face;

};

