#pragma once
#include "Face.h"
#include "HalfEdge.h"

class HalfEdge;

class Vertex
{
public:

	Vertex(double x, double y, double z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	~Vertex()
	{
	}

	double x, y, z;
	HalfEdge* nextHalfEdge;

};

