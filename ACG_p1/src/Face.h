#pragma once
#include "Vertex.h"
#include "HalfEdge.h"

class Vertex;
class HalfEdge;

class Face
{
public:

	Face(pair<int, int> halfedge)
	{
	}

	~Face()
	{
	}

	pair<int, int> corHalfEdge;

};

