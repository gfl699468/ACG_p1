#pragma once
#include "HalfEdge.h"
#include "Vertex.h"
#include "Face.h"
#include <string>
#include <fstream>
#include <vector>
#include <regex>
#include <map>
#include <list>
#include <iostream>
using namespace std;

class Reader
{

private:
	map<pair<int, int>, HalfEdge> halfEdgePairs_map;
	vector<tuple<int, int, int>> facet;

	regex comment = regex("^#.+|#");
	regex shape_begin = regex("^[[:space:]]*Shape[[:space:]]*[{]", regex_constants::extended);
	regex geometry_indexedFaceSet_begin = regex("^[[:space:]]*geometry[[:space:]]+IndexedFaceSet[[:space:]]*[{]", regex_constants::extended);
	regex coord_coordinate_begin = regex("^[[:space:]]*coord[[:space:]]+Coordinate[[:space:]]*[{]", regex_constants::extended);
	regex point_begin = regex("^[[:space:]]*point[[:space:]]*\[[]", regex_constants::extended);
	regex coordIndex_begin = regex("^[[:space:]]*coordIndex[[:space:]]*[[]", regex_constants::extended);
	regex data_end = regex("[[:space:]]*[]][[:space:]]*", regex_constants::extended);
	regex end = regex("[[:space:]]*[}][[:space:]]*", regex_constants::extended);

	inline void checkPoint(ifstream* fs) {
		regex pattern = regex("^[[:space:]]*(([-]?[0-9]+(\.[0-9]+)?)|([-]?[0-9]+(\.[0-9]+)?e[-]?[0-9]+))[[:space:]]+(([-]?[0-9]+(\.[0-9]+)?)|([-]?[0-9]+(\.[0-9]+)?e[-]?[0-9]+))[[:space:]]+(([-]?[0-9]+(\.[0-9]+)?)|([-]?[0-9]+(\.[0-9]+)?e[-]?[0-9]+))[[:space:]]*$", regex_constants::extended);
		string tmp;
		int pos;
		while (!fs->eof())
		{
			pos = fs->tellg();
			getline(*fs, tmp);
			if (regex_match(tmp, this->comment)) {
				continue;
			}
			else
			{
				if (regex_match(tmp, this->point_begin)) {
					while (!fs->eof())
					{
						int p1 = fs->tellg();
						getline(*fs, tmp);
						if (regex_match(tmp, this->comment)) {
							continue;
						}
						if (regex_match(tmp, this->data_end)) {
							break;
						}
						fs->seekg(p1);
						if (regex_match(tmp, pattern)) {
							getline(*fs, tmp);
							string::size_type sz;
							double x = stod(tmp, &sz);
							tmp = tmp.substr(sz);
							double y = stod(tmp, &sz);
							tmp = tmp.substr(sz);
							double z = stod(tmp);
							vertex.insert(make_pair(vertex.size(),Vertex(x, y, z)));
						}
						else {
							cout << "Unkown data in point field!" << endl;
							exit;
						}
					}
				}
				else {
					fs->seekg(pos);
					break;
				};
			};
		}
	}
	
	inline void checkCoordIndex(ifstream* fs) {
		regex pattern = regex("^[[:space:]]*[0-9]+[[:space:]]+[0-9]+[[:space:]]+[0-9]+[[:space:]]+-1$", regex_constants::extended);
		string tmp;
		int pos;
		while (!fs->eof())
		{
			pos = fs->tellg();
			getline(*fs, tmp);
			if (regex_match(tmp, this->comment)) {
				continue;
			}
			else
			{
				if (regex_match(tmp, this->coordIndex_begin)) {
					while (!fs->eof())
					{
						int p1 = fs->tellg();
						getline(*fs, tmp);
						if (regex_match(tmp, this->comment)) {
							continue;
						}
						if (regex_match(tmp, this->data_end)) {
							break;
						}
						fs->seekg(p1);
						if (regex_match(tmp, pattern)) {
							getline(*fs, tmp);
							string::size_type sz;
							int a = stoi(tmp, &sz);
							tmp = tmp.substr(sz);
							int b = stoi(tmp, &sz);
							tmp = tmp.substr(sz);
							int c = stoi(tmp);
							facet.push_back(tuple<int, int, int>(a, b, c));
						}
						else {
							cout << "Unkown data in coordIndex field!" << endl;
							exit;
						}
					}
				}
				else {
					fs->seekg(pos);
					break;
				};
			};
		}
	}

	inline void checkCoord(ifstream* fs) {
		string tmp;
		int pos;
		while (!fs->eof())
		{
			pos = fs->tellg();
			getline(*fs, tmp);
			if (regex_match(tmp, this->comment)) {
				continue;
			}
			else
			{
				if (regex_match(tmp, this->coord_coordinate_begin)) {
					while (!fs->eof())
					{
						int p1 = fs->tellg();
						getline(*fs, tmp);
						if (regex_match(tmp, this->comment)) {
							continue;
						}
						if (regex_match(tmp, this->end)) {
							break;
						}
						fs->seekg(p1);
						checkPoint(fs);
					}
				}
				else {
					fs->seekg(pos);
					break;
				};
			};
		}
	}

	inline void checkGeometry(ifstream* fs) {
		string tmp;
		int pos;
		while (!fs->eof())
		{
			pos = fs->tellg();
			getline(*fs, tmp);
			if (regex_match(tmp, this->comment)) {
				continue;
			}
			else
			{
				if (regex_match(tmp, this->geometry_indexedFaceSet_begin)) {
					while (!fs->eof())
					{
						int p1 = fs->tellg();
						getline(*fs, tmp);
						if (regex_match(tmp, this->comment)) {
							continue;
						}
						if (regex_match(tmp, this->end)) {
							break;
						}
						fs->seekg(p1);
						checkCoord(fs);
						checkCoordIndex(fs);
					}
				}
				else {
					fs->seekg(pos);
					break;
				};
			};
		}
	}

	inline void checkShape(ifstream* fs) {
		string tmp;
		int pos;
		while (!fs->eof())
		{
			pos = fs->tellg();
			getline(*fs, tmp);
			if (regex_match(tmp, this->comment)) {
				continue;
			}
			else
			{
				if (regex_match(tmp, this->shape_begin)) {
					while (!fs->eof())
					{
						int p1 = fs->tellg();
						getline(*fs, tmp);
						if (regex_match(tmp, this->comment)) {
							continue;
						}
						if (regex_match(tmp, this->end)) {
							break;
						}
						fs->seekg(p1);
						checkGeometry(fs);
					}
				}
				else {
					fs->seekg(pos);
					break;
				};
			};
		}
	}

	inline void loadFacet(int v1, int v2, int v3) {
		auto edge1 = HalfEdge();
		auto edge2 = HalfEdge();
		auto edge3 = HalfEdge();
		edge1.ori = v1;
		edge1.dest = v2;
		edge2.ori = v2;
		edge2.dest = v3;
		edge3.ori = v3;
		edge3.dest = v1;
		halfEdge_map[make_pair(v1, v2)] = edge1;
		halfEdge_map[make_pair(v2, v3)] = edge2;
		halfEdge_map[make_pair(v3, v1)] = edge3;
		halfEdge_map[make_pair(v1, v2)].nextHalfEdge = make_pair(v2, v3);
		halfEdge_map[make_pair(v2, v3)].nextHalfEdge = make_pair(v3, v1);
		halfEdge_map[make_pair(v3, v1)].nextHalfEdge = make_pair(v1, v2);
		halfEdge_map[make_pair(v1, v2)].prevHalfEdge = make_pair(v3, v1);
		halfEdge_map[make_pair(v2, v3)].prevHalfEdge = make_pair(v1, v2);
		halfEdge_map[make_pair(v3, v1)].prevHalfEdge = make_pair(v2, v3);
		halfEdge_map[make_pair(v1, v2)].oriVertex = v1;
		vertex[v1].nextHalfEdge = make_pair(v1, v2);
		halfEdge_map[make_pair(v2, v3)].oriVertex = v2;
		vertex[v2].nextHalfEdge = make_pair(v2, v3);
		halfEdge_map[make_pair(v3, v1)].oriVertex = v3;
		vertex[v3].nextHalfEdge = make_pair(v3, v1);
		face.insert(make_pair(face.size(), Face(make_pair(v1, v2))));
		halfEdge_map[make_pair(v1, v2)].face = face.size() - 1;
		halfEdge_map[make_pair(v2, v3)].face = face.size() - 1;
		halfEdge_map[make_pair(v3, v1)].face = face.size() - 1;
		halfEdgePairs_map[make_pair((v1 + v2), abs(v1 - v2))] = edge1;
		halfEdgePairs_map[make_pair((v2 + v3), abs(v2 - v3))] = edge2;
		halfEdgePairs_map[make_pair((v3 + v1), abs(v3 - v1))] = edge3;
	}

	inline bool checkFacet(int v1, int v2, int v3) {
		auto edge1 = make_pair((v1 + v2), abs(v1 - v2));
		auto edge2 = make_pair((v2 + v3), abs(v2 - v3));
		auto edge3 = make_pair((v3 + v1), abs(v3 - v1));
		bool flag = false; //check weather the facet is added to the vector or not
		if (halfEdgePairs_map.find(edge1) != halfEdgePairs_map.end()) {
			HalfEdge& e1 = halfEdgePairs_map.find(edge1)->second;
			auto a1 = e1.ori;
			auto a2 = e1.dest;
			if (e1.ori == v2) {
				if (!flag) {
					loadFacet(v1, v2, v3);
					flag = true;
				}
				halfEdge_map[make_pair(v1, v2)].pairEdge = make_pair(a1, a2);
				halfEdge_map[make_pair(a1, a2)].pairEdge = make_pair(v1, v2);
			}
			else {
				if (!flag) {
					loadFacet(v2, v1, v3);
					flag = true;
				}
				halfEdge_map[make_pair(v2, v1)].pairEdge = make_pair(a1, a2);
				halfEdge_map[make_pair(a1, a2)].pairEdge = make_pair(v2, v1);
			}
		}
		if (halfEdgePairs_map.find(edge2) != halfEdgePairs_map.end()) {
			HalfEdge& e1 = halfEdgePairs_map.find(edge2)->second;
			auto a1 = e1.ori;
			auto a2 = e1.dest;
			if (e1.ori == v3) {
				if (!flag) {
					loadFacet(v1, v2, v3);
					flag = true;
				}
				halfEdge_map[make_pair(v2, v3)].pairEdge = make_pair(a1, a2);
				halfEdge_map[make_pair(a1, a2)].pairEdge = make_pair(v2, v3);
			}
			else {
				if (!flag) {
					loadFacet(v2, v1, v3);
					flag = true;
				}
				halfEdge_map[make_pair(v3, v2)].pairEdge = make_pair(a1, a2);
				halfEdge_map[make_pair(a1, a2)].pairEdge = make_pair(v3, v2);
			}
		}
		if (halfEdgePairs_map.find(edge3) != halfEdgePairs_map.end()) {
			HalfEdge& e1 = halfEdgePairs_map.find(edge3)->second;
			auto a1 = e1.ori;
			auto a2 = e1.dest;
			if (e1.ori == v1) {
				if (!flag) {
					loadFacet(v1, v2, v3);
					flag = true;
				}
				halfEdge_map[make_pair(v3, v1)].pairEdge = make_pair(a1, a2);
				halfEdge_map[make_pair(a1, a2)].pairEdge = make_pair(v3, v1);
			}
			else {
				if (!flag) {
					loadFacet(v2, v1, v3);
					flag = true;
				}
				halfEdge_map[make_pair(v1, v3)].pairEdge = make_pair(a1, a2);
				halfEdge_map[make_pair(a1, a2)].pairEdge = make_pair(v1, v3);
			}
		}
		return flag;
	}


public:

	map<pair<int, int>, HalfEdge> halfEdge_map;
	map<int, Vertex> vertex;
	map<int, Face> face;

	inline void loadVRMLFile(string fileName) {
		ifstream fs(fileName);
		vector<string> context;
		checkShape(&fs);
		auto first_facet = facet[0];
		facet.erase(facet.begin());
		
		loadFacet(get<0>(first_facet), get<1>(first_facet), get<2>(first_facet));

		while (facet.size() != 0)
		{
			for (size_t i = 0; i < facet.size(); i++)
			{
				if (checkFacet(get<0>(facet[i]), get<1>(facet[i]), get<2>(facet[i]))) {
					facet.erase(facet.begin() + i);
				}
			}
		}
	}
};

