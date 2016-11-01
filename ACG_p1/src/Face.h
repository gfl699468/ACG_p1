#pragma once
#include "Vertex.h"
#include "HalfEdge.h"

class Vertex;
class HalfEdge;

class Face
{
public:

	Face(HalfEdge* halfedge)
	{
	}

	~Face()
	{
	}

	HalfEdge* corHalfEdge;

};

