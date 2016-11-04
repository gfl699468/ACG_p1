#pragma once
#include "Vertex.h"
#include "HalfEdge.h"

class Vertex;
class HalfEdge;

class Face
{
public:

	Face() {

	}

	Face(pair<int, int> halfedge)
	{
		corHalfEdge = halfedge;
	}

	~Face()
	{
	}
	tuple<double, double, double> normal;
	pair<int, int> corHalfEdge;

};

