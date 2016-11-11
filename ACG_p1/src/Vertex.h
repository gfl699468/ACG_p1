#pragma once
#include "Face.h"
#include "HalfEdge.h"

class HalfEdge;

class Vertex
{
public:

	Vertex() {

	}

	Vertex(double x, double y, double z)
	{
		pos = make_tuple(x, y, z);
	}

	~Vertex()
	{
	}

	inline double x() {
		return get<0>(pos);
	}

	inline double y() {
		return get<1>(pos);
	}
	
	inline double z() {
		return get<2>(pos);
	}
	std::tuple<double, double, double> pos;
	pair<int, int> nextHalfEdge;

};

