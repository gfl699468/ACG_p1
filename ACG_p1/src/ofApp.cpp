#include "ofApp.h"

#define nil_pair make_pair(-1, -1)

std::tuple<double, double, double> operator+(std::tuple<double, double, double>& lhs, std::tuple<double, double, double>& rhs) {
	return make_tuple(get<0>(lhs) + get<0>(rhs), get<1>(lhs) + get<1>(rhs), get<2>(lhs) + get<2>(rhs));
};

std::tuple<double, double, double> operator-(std::tuple<double, double, double>& lhs, std::tuple<double, double, double>& rhs) {
	return make_tuple(get<0>(lhs) - get<0>(rhs), get<1>(lhs) - get<1>(rhs), get<2>(lhs) - get<2>(rhs));
};

std::tuple<double, double, double>& operator+=(std::tuple<double, double, double>& lhs, const std::tuple<double, double, double>& rhs) {
	get<0>(lhs) += get<0>(rhs);
	get<1>(lhs) += get<1>(rhs);
	get<2>(lhs) += get<2>(rhs);
	return lhs;
};

std::tuple<double, double, double> operator*(const double& lhs, std::tuple<double, double, double>& rhs) {
	return make_tuple(lhs * get<0>(rhs), lhs * get<1>(rhs), lhs * get<2>(rhs));
};

std::tuple<double, double, double> operator/(std::tuple<double, double, double>& lhs, const double& rhs) {
	return make_tuple(get<0>(lhs) / rhs, get<1>(lhs) / rhs, get<2>(lhs) / rhs);
};

using namespace std;
//--------------------------------------------------------------
void ofApp::setup() {
	panel.setup();
	panel.add(openFileButton.setup("open Model file"));
	panel.add(saveFileButton.setup("save Model file"));
	panel.add(drawModel.setup("draw Model", false));
	panel.add(drawWireFrame.setup("draw WireFrame", false));
	panel.add(loopSubdivisionButton.setup("Loop Subdivision"));
	panel.add(modifiedButterflySubdivisionButton.setup("MB Subdivision"));
	panel.add(modelScale.setup("Model Scale", 100, 1, 1000));
	panel.add(lightPos.setup("Light Position", ofVec3f(-100, -100, 100), ofVec3f(-100, -100, -100), ofVec3f(100, 100, 100)));

	openFileButton.addListener(this, &ofApp::openFileButtonPressed);
	saveFileButton.addListener(this, &ofApp::saveFileButtonPressed);

	loopSubdivisionButton.addListener(this, &ofApp::loopSubdivisionButtonPressed);

	modifiedButterflySubdivisionButton.addListener(this, &ofApp::modifiedButterflySubdivisionButtonPressed);

	drawModel.addListener(this, &ofApp::drawModelToggled);

	drawWireFrame.addListener(this, &ofApp::drawWireFrameToggled);

	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	ofEnableLighting();
	ofEnableSmoothing();
	light.enable();
	light.setPointLight();	
	ambientLight.enable();
	ambientLight.setAmbientColor(ofFloatColor(0.2, 0.2, 0.2, 1));
	light.setPosition(lightPos);

}

//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {
	light.setPosition(lightPos);

	ofBackgroundGradient(ofColor(64), ofColor(0));
	cam.begin();
	ofEnableDepthTest();
	light.draw();
	//ofRotateY(ofGetElapsedTimef() * 30);

	ofScale(1, -1, 1);
	ofScale(modelScale, modelScale, modelScale);
	if (drawModel) {
		if (halfEdge_map.size() != 0) {
			mesh.draw();

		}
		else {
			ofSystemAlertDialog("Please load model file first!");
			drawModel = false;
		}
	}
	else if (drawWireFrame) {
		if (halfEdge_map.size() != 0) {
			mesh.drawWireframe();
		}
		else {
			ofSystemAlertDialog("Please load model file first!");
			drawModel = false;
		}
	}
	ofDisableDepthTest();
	cam.end();
	panel.draw();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}

void ofApp::openFileButtonPressed() {
	reader = Reader();
	ofFileDialogResult result = ofSystemLoadDialog("Load VRML file");
	if (result.bSuccess) {
		string path = result.getPath();
		reader.loadVRMLFile(path);
		this->halfEdge_map = reader.halfEdge_map;
		this->vertex = reader.vertex;
		this->face = reader.face;
		updateModelvbo();
	}
}

void ofApp::saveFileButtonPressed()
{
	writer = Writer();
	ofFileDialogResult result = ofSystemSaveDialog("Untitled.wrl", "Save VRML file");
	if (result.bSuccess) {
		string path = result.getPath();
		writer.writeVRMLFile(path, halfEdge_map, vertex, face);
	}
}

void ofApp::loopSubdivisionButtonPressed()
{
	map<int, Vertex> new_vertex;
	map<pair<int, int>, HalfEdge> new_halfEdge_map;
	map<int, Face> new_face;
	reconstructHalfEdge(halfEdge_map, vertex, face, new_vertex, new_halfEdge_map, new_face);

	//Finished construct the halfedge, begin to adjust the vertices.

	auto valence = map<int, int>();

	for (auto i = new_halfEdge_map.begin(); i != new_halfEdge_map.end(); i++)
	{
		if (i->second.pairEdge == nil_pair) {
			valence[new_halfEdge_map[i->second.nextHalfEdge].oriVertex] = -(new_halfEdge_map.size() + 1);
			valence[i->second.oriVertex] = -(new_halfEdge_map.size() + 1);
		}
		else {
			valence[i->second.oriVertex]++;
		}
	}
	auto new_vertex_size = new_vertex.size() - vertex.size();

	//adjust the pos of new vertex
	//       /\ P2
	//      /__\
	//     /\  /\
	//  P1/__\/__\ P3
	//    \  /\  /
	//     \/__\/
	//      \  /
	//       \/ P4
	for (size_t i = new_vertex.size() - new_vertex_size; i < new_vertex.size(); i++)
	{
		auto pos = make_tuple(0.0, 0.0, 0.0);
		auto o = new_vertex[i].nextHalfEdge;
		if (valence[i] < 0) {  //boundary edge
			//find the beginning edge if the vertex is on an boundary edge
			pair<int, int> n;
			if (new_halfEdge_map[o].pairEdge != nil_pair) {
				n = new_halfEdge_map[new_halfEdge_map[o].pairEdge].nextHalfEdge;
			}
			else {
				n = o;
			}
			auto p_n = o;
			while ((o != n) && (n != nil_pair) && (new_halfEdge_map[n].pairEdge != nil_pair)) {
				p_n = n;
				n = new_halfEdge_map[new_halfEdge_map[n].pairEdge].nextHalfEdge;
			}
			if (n != nil_pair) { o = n; }
			else { o = p_n; n = o; }
			do {
				p_n = n;
				auto p = new_halfEdge_map[new_halfEdge_map[n].nextHalfEdge].oriVertex;
				if (p < new_vertex.size() - new_vertex_size) {
					pos += (1.0 / 2.0)*new_vertex[p].pos;
				}
				n = new_halfEdge_map[new_halfEdge_map[n].prevHalfEdge].pairEdge;
			} while ((o != n) && (n != nil_pair) && (new_halfEdge_map[n].pairEdge != nil_pair));
			//     <--
			//   p2 -- i -- p1
			//
			//point i has no direct connection to point p2
			auto p = new_halfEdge_map[new_halfEdge_map[new_halfEdge_map[p_n].nextHalfEdge].nextHalfEdge].oriVertex;
			if (p < new_vertex.size() - new_vertex_size) {
				pos += (1.0 / 2.0)*new_vertex[p].pos;
			}
		}
		else {  // normal case
			auto n = o;
			do {
				auto p = new_halfEdge_map[new_halfEdge_map[n].nextHalfEdge].oriVertex;
				if (p < new_vertex.size() - new_vertex_size) {
					pos += (3.0 / 8.0)*new_vertex[p].pos;
				}
				else {
					auto next_p = new_halfEdge_map[new_halfEdge_map[new_halfEdge_map[n].nextHalfEdge].nextHalfEdge].oriVertex;
					auto edge = new_halfEdge_map[new_halfEdge_map[n].nextHalfEdge].pairEdge;
					if (next_p >= new_vertex.size() - new_vertex_size) {
						auto np = new_halfEdge_map[new_halfEdge_map[edge].prevHalfEdge].oriVertex;
						pos += (1.0 / 8.0)*new_vertex[np].pos;
					}
				}
				n = new_halfEdge_map[new_halfEdge_map[n].prevHalfEdge].pairEdge;
			} while (o != n);
		}
		new_vertex[i].pos = pos;
	}


	//adjust the pos of old vertex
	//        ________
	//    P1 /\      /\ P2
	//      /  \    /  \
	//     /    \  /    \
	// P0 /______\/______\ P3
	//           /\      /
	//      ... /  \    /
	//         /    \  /
	//      Pn/______\/ P4

	for (size_t i = 0; i < vertex.size(); i++)
	{
		if (valence[i] < 0) {
			auto pos = make_tuple(0.0, 0.0, 0.0);
			pos += (3. / 4.) * vertex[i].pos;
			auto o = vertex[i].nextHalfEdge;
			pair<int, int> n;
			if (halfEdge_map[o].pairEdge != nil_pair) {
				n = halfEdge_map[halfEdge_map[o].pairEdge].nextHalfEdge;
			}
			else {
				n = o;
			}
			//find the beginning edge if the vertex is on an boundary edge
			auto p_n = o;
			while ((o != n) && (n != nil_pair) && (halfEdge_map[n].pairEdge != nil_pair)) {
				p_n = n;
				n = halfEdge_map[halfEdge_map[n].pairEdge].nextHalfEdge;
			}
			if (n != nil_pair) { o = n; }
			else { o = p_n; n = o; }
			do {
				p_n = n;
				auto p = halfEdge_map[halfEdge_map[n].nextHalfEdge].oriVertex;
				if (valence[p] < 0) {
					pos += (1.0 / 8.0)*new_vertex[p].pos;
				}
				n = halfEdge_map[halfEdge_map[n].prevHalfEdge].pairEdge;
			} while ((o != n) && (n != nil_pair) && (halfEdge_map[n].pairEdge != nil_pair));
			//     <--
			//   p2 -- i -- p1
			//
			//point i has no direct connection to point p2
			auto p = halfEdge_map[halfEdge_map[halfEdge_map[p_n].nextHalfEdge].nextHalfEdge].oriVertex;
			if (p < new_vertex.size() - new_vertex_size) {
				pos += (1.0 / 8.0)*new_vertex[p].pos;
			}
			new_vertex[i].pos = pos;
		}
		else {
			auto pos = make_tuple(0.0, 0.0, 0.0);
			auto o = vertex[i].nextHalfEdge;
			auto n = o;
			auto counter = 0;
			do {
				auto p = halfEdge_map[halfEdge_map[n].nextHalfEdge].oriVertex;
				pos += vertex[p].pos;
				counter++;
				n = halfEdge_map[halfEdge_map[n].prevHalfEdge].pairEdge;
			} while (o != n);
			auto beta = 0.0;
			if (counter > 3) {
				beta = (3.0 / (8.0 * counter));
			}
			else {
				beta = (3.0 / 16.0);
			}
			new_vertex[i].pos = (1.0 - counter * beta) * new_vertex[i].pos + beta * pos;
		}

	}


	//replace the original data structure
	vertex = new_vertex;
	halfEdge_map = new_halfEdge_map;
	face = new_face;
	updateModelvbo();
}

void ofApp::modifiedButterflySubdivisionButtonPressed()
{
	map<int, Vertex> new_vertex;
	map<pair<int, int>, HalfEdge> new_halfEdge_map;
	map<int, Face> new_face;
	reconstructHalfEdge(halfEdge_map, vertex, face, new_vertex, new_halfEdge_map, new_face);

	//Finished construct the halfedge, begin to adjust the vertices.

	auto new_vertex_size = new_vertex.size() - vertex.size();

	//calc all vertex's valence

	auto valence = map<int, int>();

	for (auto i = new_halfEdge_map.begin(); i != new_halfEdge_map.end(); i++)
	{
		if (i->second.pairEdge == nil_pair) {
			valence[new_halfEdge_map[i->second.nextHalfEdge].oriVertex] += -(new_halfEdge_map.size() + 1);
			valence[i->second.oriVertex] += -(new_halfEdge_map.size() + 1);
		}
		else {
			valence[i->second.oriVertex]++;
		}
	}


	//adjust the pos of new vertex

	for (size_t i = new_vertex.size() - new_vertex_size; i < new_vertex.size(); i++)
	{
		if (valence[i] < 0) {
			auto o = new_vertex[i].nextHalfEdge;
			pair<int, int> n;
			if (new_halfEdge_map[o].pairEdge != nil_pair) {
				n = new_halfEdge_map[new_halfEdge_map[o].pairEdge].nextHalfEdge;
			}
			else {
				n = o;
			}
			auto pos = make_tuple(0., 0., 0.);
			int p1, p2;
			//find the beginning edge if the vertex is on an boundary edge
			auto p_n = o;
			while ((o != n) && (n != nil_pair) && (new_halfEdge_map[n].pairEdge != nil_pair)) {
				p_n = n;
				n = new_halfEdge_map[new_halfEdge_map[n].pairEdge].nextHalfEdge;
			}
			if (n != nil_pair) { o = n; }
			else { o = p_n; n = o; }
			do {
				p_n = n;
				auto p = new_halfEdge_map[new_halfEdge_map[n].nextHalfEdge].oriVertex;
				if (p < new_vertex.size() - new_vertex_size) {
					p1 = p;
					pos += (9.0 / 16.0)*new_vertex[p].pos;
				}
				n = new_halfEdge_map[new_halfEdge_map[n].prevHalfEdge].pairEdge;
			} while ((o != n) && (n != nil_pair) && (new_halfEdge_map[n].pairEdge != nil_pair));
			//     <--
			//   p2 -- i -- p1
			//
			//point i has no direct connection to point p2
			auto p = new_halfEdge_map[new_halfEdge_map[new_halfEdge_map[p_n].nextHalfEdge].nextHalfEdge].oriVertex;
			if (p < new_vertex.size() - new_vertex_size) {
				p2 = p;
				pos += (9.0 / 16.0)*new_vertex[p].pos;
			}
			//                        ---->
			//    p4-----p2 -- i -- p1-----p3
			//
			o = vertex[p1].nextHalfEdge;
			n = halfEdge_map[halfEdge_map[o].pairEdge].nextHalfEdge;
			p_n = o;
			while ((o != n) && (n != nil_pair) && (halfEdge_map[n].pairEdge != nil_pair)) {
				p_n = n;
				n = halfEdge_map[halfEdge_map[n].pairEdge].nextHalfEdge;
			}
			if (n != nil_pair) { o = n; }
			else { o = p_n; n = o; }
			p = halfEdge_map[halfEdge_map[n].nextHalfEdge].oriVertex;
			pos += -(1.0 / 16.0)*new_vertex[p].pos;
			//      <----
			//    p4-----p2 -- i -- p1-----p3
			//
			o = vertex[p2].nextHalfEdge;
			n = o;
			do {
				p_n = n;
				n = halfEdge_map[halfEdge_map[n].prevHalfEdge].pairEdge;
			} while ((o != n) && (n != nil_pair) && (halfEdge_map[n].pairEdge != nil_pair));
			p = halfEdge_map[halfEdge_map[halfEdge_map[p_n].nextHalfEdge].nextHalfEdge].oriVertex;
			if (p < new_vertex.size() - new_vertex_size) {
				pos += -(1.0 / 16.0)*new_vertex[p].pos;
			}
			new_vertex[i].pos = pos;
		}
		else {
			auto pa = vector<int>();
			auto o = new_vertex[i].nextHalfEdge;
			auto n = o;
			auto counter = 0;
			do {
				auto p = new_halfEdge_map[new_halfEdge_map[n].nextHalfEdge].oriVertex;
				if (p < new_vertex.size() - new_vertex_size) {
					pa.push_back(p);
				}
				n = new_halfEdge_map[new_halfEdge_map[n].prevHalfEdge].pairEdge;
			} while (o != n);
			//op_p3  op_p1 op_p4
			//   ____________
			//   \ e3 /\ e4 /
			//    \  /e1\  /
			//     \/____\/
			//pa[0]/\ e2 /\pa[1]    
			//    /e5\  /e6\
			//   /____\/____\
			// op_p5 op_p2 op_p6
			//
			//pa[0] and pa[1] may be the vertex on the boundary edge
			//
			//case 1
			if ((valence[pa[0]] + valence[pa[1]] == 12)&(valence[pa[0]] == 6)) {
				auto e1 = make_pair(pa[0], pa[1]);
				auto e2 = make_pair(pa[1], pa[0]);
				auto op_p1 = halfEdge_map[halfEdge_map[e1].prevHalfEdge].oriVertex;
				auto op_p2 = halfEdge_map[halfEdge_map[e2].prevHalfEdge].oriVertex;
				auto e3 = make_pair(pa[0], op_p1);
				auto e4 = make_pair(op_p1, pa[1]);
				auto op_p3 = halfEdge_map[halfEdge_map[e3].prevHalfEdge].oriVertex;
				auto op_p4 = halfEdge_map[halfEdge_map[e4].prevHalfEdge].oriVertex;
				auto e5 = make_pair(op_p2, pa[0]);
				auto e6 = make_pair(pa[1], op_p2);
				auto op_p5 = halfEdge_map[halfEdge_map[e5].prevHalfEdge].oriVertex;
				auto op_p6 = halfEdge_map[halfEdge_map[e6].prevHalfEdge].oriVertex;

				auto case_1_pos = (1. / 2.) * (new_vertex[pa[0]].pos + new_vertex[pa[1]].pos);
				auto case_2_pos = (1. / 8.) * (new_vertex[op_p1].pos + new_vertex[op_p2].pos);
				auto case_3_pos = (1. / 16.) * (new_vertex[op_p3].pos + new_vertex[op_p4].pos + new_vertex[op_p5].pos + new_vertex[op_p6].pos);
				new_vertex[i].pos = case_1_pos + case_2_pos - case_3_pos;
			}
			//case 2
			if ((valence[pa[0]] + valence[pa[1]] != 12)&((valence[pa[0]] == 6)^((valence[pa[1]] == 6)))) {
				if (valence[pa[1]] == 6) {
					iter_swap(pa.begin(), pa.end()-1);
				}
				auto o = make_pair(pa[1], pa[0]);
				auto n = o;
				auto pos = (3. / 4.) * new_vertex[pa[1]].pos;

				if (valence[pa[1]] >= 5)
				{
					auto counter = 0.;
					do
					{
						auto p = halfEdge_map[halfEdge_map[n].nextHalfEdge].oriVertex;
						auto coeff = (1. / valence[pa[1]])*((1. / 4.) + cos(2. * counter*PI / valence[pa[1]]) + (1. / 2.)*(cos(4. * counter*PI / valence[pa[1]])));
						pos += coeff*new_vertex[p].pos;
						n = halfEdge_map[halfEdge_map[n].prevHalfEdge].pairEdge;
						counter++;
					} while (o != n);
				}
				if (valence[pa[1]] == 4) {
					double coeffs[4] = { 3. / 8., 0, -1. / 8., 0 };
					auto counter = 0;
					do
					{
						auto p = halfEdge_map[halfEdge_map[n].nextHalfEdge].oriVertex;
						auto coeff = coeffs[counter];
						pos += coeff*new_vertex[p].pos;
						n = halfEdge_map[halfEdge_map[n].prevHalfEdge].pairEdge;
						counter++;
					} while (o != n);
				}
				if (valence[pa[1]] == 3) {
					double coeffs[3] = { 5. / 12., -1. / 12., -1. / 12. };
					auto counter = 0;
					do
					{
						auto p = halfEdge_map[halfEdge_map[n].nextHalfEdge].oriVertex;
						auto coeff = coeffs[counter];
						pos += coeff*new_vertex[p].pos;
						n = halfEdge_map[halfEdge_map[n].prevHalfEdge].pairEdge;
						counter++;
					} while (o != n);
				}
				if (valence[pa[1]] < 0) {
					pair<int, int> n;
					if (halfEdge_map[o].pairEdge != nil_pair) {
						n = halfEdge_map[halfEdge_map[o].pairEdge].nextHalfEdge;
					}
					else {
						n = o;
					}
					vector<int> ps,ps1;
					//find one vertex on the boundary edge
					auto p_n = o;
					while ((o != n) && (n != nil_pair) && (halfEdge_map[n].pairEdge != nil_pair)) {
						p_n = n;
						n = halfEdge_map[halfEdge_map[n].pairEdge].nextHalfEdge;
					}
					if (n != nil_pair) { o = n; }
					else { o = p_n; n = o; }
					//traverse to another vertex on boundary edge. save all the encountered vertices
					auto counter = 0.;
					do
					{
						auto p = halfEdge_map[halfEdge_map[n].nextHalfEdge].oriVertex;
						ps.push_back(p);
						n = halfEdge_map[halfEdge_map[n].prevHalfEdge].pairEdge;
						counter++;
					} while ((o != n) && (n != nil_pair) && (halfEdge_map[n].pairEdge != nil_pair));
					//use the formula from the case that the valence is larger than 4 and not equal to 6
					auto k = (new_halfEdge_map.size() + 1) * 2 + valence[pa[1]] + 1; // add 1 cause when we calc the valence, two adjust boundary edges will only give one valence to the vertex
					auto p0 = find(ps.begin(), ps.end(), pa[0]);
					ps1.insert(ps1.begin(), p0, ps.end());
					ps1.insert(ps1.end(), ps.begin(), p0);
					if (k == 3) {
						double coeffs[3] = { 5. / 12., -1. / 12., -1. / 12. };
						for (size_t j = 0; j < ps1.size(); j++)
						{
							auto coeff = coeffs[j];
							pos += coeff*vertex[ps1[j]].pos;
						}
					}
					if (k == 4) {
						double coeffs[4] = { 3. / 8., 0, -1. / 8., 0 };
						for (size_t j = 0; j < ps1.size(); j++)
						{
							auto coeff = coeffs[j];
							pos += coeff*vertex[ps1[j]].pos;
						}
					}
					if (k >= 5) {
						for (size_t j = 0; j < ps1.size(); j++)
						{
							auto coeff = (1. / k)*((1. / 4.) + cos(2. * j*PI / k) + (1. / 2.)*(cos(4. * j*PI / k)));
							pos += coeff*vertex[ps1[j]].pos;
						}
					}

				}
				new_vertex[i].pos = pos;
			}
			//case 3
			if ((valence[pa[0]] != 6)&(valence[pa[1]] != 6)) {
				auto o = make_pair(pa[0], pa[1]);
				auto n = o;
				auto pos = (3. / 4.) * new_vertex[pa[0]].pos + (3. / 4.) * new_vertex[pa[1]].pos;

				if (valence[pa[0]] >= 5)
				{
					auto counter = 0.;
					do
					{
						auto p = halfEdge_map[halfEdge_map[n].nextHalfEdge].oriVertex;
						auto coeff = (1. / valence[pa[0]])*((1. / 4.) + cos(2. * counter*PI / valence[pa[0]]) + (1. / 2.)*(cos(4. * counter*PI / valence[pa[0]])));
						auto t = cos(PI);
						pos += coeff*new_vertex[p].pos;
						n = halfEdge_map[halfEdge_map[n].prevHalfEdge].pairEdge;
						counter++;
					} while (o != n);
				}
				if (valence[pa[0]] == 4) {
					double coeffs[4] = { 3. / 8., 0, -1. / 8., 0 };
					auto counter = 0;
					do
					{
						auto p = halfEdge_map[halfEdge_map[n].nextHalfEdge].oriVertex;
						auto coeff = coeffs[counter];
						pos += coeff*new_vertex[p].pos;
						n = halfEdge_map[halfEdge_map[n].prevHalfEdge].pairEdge;
						counter++;
					} while (o != n);
				}
				if (valence[pa[0]] == 3) {
					double coeffs[3] = { 5. / 12., -1. / 12., -1. / 12. };
					auto counter = 0;
					do
					{
						auto p = halfEdge_map[halfEdge_map[n].nextHalfEdge].oriVertex;
						auto coeff = coeffs[counter];
						pos += coeff*new_vertex[p].pos;
						n = halfEdge_map[halfEdge_map[n].prevHalfEdge].pairEdge;
						counter++;
					} while (o != n);
				}
				if (valence[pa[0]] < 0) {
					pair<int, int> n;
					if (halfEdge_map[o].pairEdge != nil_pair) {
						n = halfEdge_map[halfEdge_map[o].pairEdge].nextHalfEdge;
					}
					else {
						n = o;
					}
					vector<int> ps, ps1;
					//find one vertex on the boundary edge
					auto p_n = o;
					while ((o != n) && (n != nil_pair) && (halfEdge_map[n].pairEdge != nil_pair)) {
						p_n = n;
						n = halfEdge_map[halfEdge_map[n].pairEdge].nextHalfEdge;
					}
					if (n != nil_pair) { o = n; }
					else { o = p_n; n = o; }
					//traverse to another vertex on boundary edge. save all the encountered vertices
					auto counter = 0.;
					do
					{
						auto p = halfEdge_map[halfEdge_map[n].nextHalfEdge].oriVertex;
						ps.push_back(p);
						n = halfEdge_map[halfEdge_map[n].prevHalfEdge].pairEdge;
						counter++;
					} while ((o != n) && (n != nil_pair) && (halfEdge_map[n].pairEdge != nil_pair));
					//use the formula from the case that the valence is larger than 4 and not equal to 6
					auto k = (new_halfEdge_map.size() + 1) * 2 + valence[pa[0]] + 1; // add 1 cause when we calc the valence, two adjust boundary edges will only give one valence to the vertex
					auto p0 = find(ps.begin(), ps.end(), pa[0]);
					ps1.insert(ps1.begin(), p0, ps.end());
					ps1.insert(ps1.end(), ps.begin(), p0);
					if (k == 3) {
						double coeffs[3] = { 5. / 12., -1. / 12., -1. / 12. };
						for (size_t j = 0; j < ps1.size(); j++)
						{
							auto coeff = coeffs[j];
							pos += coeff*vertex[ps1[j]].pos;
						}
					}
					if (k == 4) {
						double coeffs[4] = { 3. / 8., 0, -1. / 8., 0 };
						for (size_t j = 0; j < ps1.size(); j++)
						{
							auto coeff = coeffs[j];
							pos += coeff*vertex[ps1[j]].pos;
						}
					}
					if (k >= 5) {
						for (size_t j = 0; j < ps1.size(); j++)
						{
							auto coeff = (1. / k)*((1. / 4.) + cos(2. * j*PI / k) + (1. / 2.)*(cos(4. * j*PI / k)));
							pos += coeff*vertex[ps1[j]].pos;
						}
					}

				}
				o = make_pair(pa[1], pa[0]);
				n = o;
				if (valence[pa[1]] >= 5)
				{
					auto counter = 0.;
					do
					{
						auto p = halfEdge_map[halfEdge_map[n].nextHalfEdge].oriVertex;
						auto coeff = (1. / valence[pa[1]])*((1. / 4.) + cos(2. * counter*PI / valence[pa[1]]) + (1. / 2.)*(cos(4. * counter*PI / valence[pa[1]])));
						pos += coeff*new_vertex[p].pos;
						n = halfEdge_map[halfEdge_map[n].prevHalfEdge].pairEdge;
						counter++;
					} while (o != n);
				}
				if (valence[pa[1]] == 4) {
					double coeffs[4] = { 3. / 8., 0, -1. / 8., 0 };
					auto counter = 0;
					do
					{
						auto p = halfEdge_map[halfEdge_map[n].nextHalfEdge].oriVertex;
						auto coeff = coeffs[counter];
						pos += coeff*new_vertex[p].pos;
						n = halfEdge_map[halfEdge_map[n].prevHalfEdge].pairEdge;
						counter++;
					} while (o != n);
				}
				if (valence[pa[1]] == 3) {
					double coeffs[3] = { 5. / 12., -1. / 12., -1. / 12. };
					auto counter = 0;
					do
					{
						auto p = halfEdge_map[halfEdge_map[n].nextHalfEdge].oriVertex;
						auto coeff = coeffs[counter];
						pos += coeff*new_vertex[p].pos;
						n = halfEdge_map[halfEdge_map[n].prevHalfEdge].pairEdge;
						counter++;
					} while (o != n);
				}
				if (valence[pa[1]] < 0) {
					pair<int, int> n;
					if (halfEdge_map[o].pairEdge != nil_pair) {
						n = halfEdge_map[halfEdge_map[o].pairEdge].nextHalfEdge;
					}
					else {
						n = o;
					}
					vector<int> ps, ps1;
					//find one vertex on the boundary edge
					auto p_n = o;
					while ((o != n) && (n != nil_pair) && (halfEdge_map[n].pairEdge != nil_pair)) {
						p_n = n;
						n = halfEdge_map[halfEdge_map[n].pairEdge].nextHalfEdge;
					}
					if (n != nil_pair) { o = n; }
					else { o = p_n; n = o; }
					//traverse to another vertex on boundary edge. save all the encountered vertices
					auto counter = 0.;
					do
					{
						auto p = halfEdge_map[halfEdge_map[n].nextHalfEdge].oriVertex;
						ps.push_back(p);
						n = halfEdge_map[halfEdge_map[n].prevHalfEdge].pairEdge;
						counter++;
					} while ((o != n) && (n != nil_pair) && (halfEdge_map[n].pairEdge != nil_pair));
					//use the formula from the case that the valence is larger than 4 and not equal to 6
					auto k = (new_halfEdge_map.size() + 1) * 2 + valence[pa[1]] + 1; // add 1 cause when we calc the valence, two adjust boundary edges will only give one valence to the vertex
					auto p0 = find(ps.begin(), ps.end(), pa[0]);
					ps1.insert(ps1.begin(), p0, ps.end());
					ps1.insert(ps1.end(), ps.begin(), p0);
					if (k == 3) {
						double coeffs[3] = { 5. / 12., -1. / 12., -1. / 12. };
						for (size_t j = 0; j < ps1.size(); j++)
						{
							auto coeff = coeffs[j];
							pos += coeff*vertex[ps1[j]].pos;
						}
					}
					if (k == 4) {
						double coeffs[4] = { 3. / 8., 0, -1. / 8., 0 };
						for (size_t j = 0; j < ps1.size(); j++)
						{
							auto coeff = coeffs[j];
							pos += coeff*vertex[ps1[j]].pos;
						}
					}
					if (k >= 5) {
						for (size_t j = 0; j < ps1.size(); j++)
						{
							auto coeff = (1. / k)*((1. / 4.) + cos(2. * j*PI / k) + (1. / 2.)*(cos(4. * j*PI / k)));
							pos += coeff*vertex[ps1[j]].pos;
						}
					}

				}
				new_vertex[i].pos = pos / 2;
			}
		}
	}

	//replace the original data structure
	vertex = new_vertex;
	halfEdge_map = new_halfEdge_map;
	face = new_face;
	updateModelvbo();
}

void ofApp::updateModelvbo()
{
	mesh = ofVboMesh();
	for (size_t i = 0; i < vertex.size(); i++)
	{
		mesh.addVertex(ofVec3f(vertex[i].x(), vertex[i].y(), vertex[i].z()));
	}
	for (size_t i = 0; i < face.size(); i++)
	{
		auto a1 = halfEdge_map[face[i].corHalfEdge].oriVertex;
		auto a2 = halfEdge_map[halfEdge_map[face[i].corHalfEdge].nextHalfEdge].oriVertex;
		auto a3 = halfEdge_map[halfEdge_map[face[i].corHalfEdge].prevHalfEdge].oriVertex;
		mesh.addIndex(a1);
		mesh.addIndex(a2);
		mesh.addIndex(a3);
	}
	calcNormals();
	mesh.enableNormals();
}

void ofApp::calcNormals()
{
	for (size_t i = 0; i < face.size(); i++)
	{
		face[i].normal = calcFaceNormal(i);
	}
}

tuple<double, double, double> ofApp::calcFaceNormal(int i) {
	auto v1 = halfEdge_map[face[i].corHalfEdge].oriVertex;
	auto v2 = halfEdge_map[halfEdge_map[face[i].corHalfEdge].nextHalfEdge].oriVertex;
	auto v3 = halfEdge_map[halfEdge_map[face[i].corHalfEdge].prevHalfEdge].oriVertex;
	auto p1 = ofVec3f(vertex[v1].x(), vertex[v1].y(), vertex[v1].z());
	auto p2 = ofVec3f(vertex[v2].x(), vertex[v2].y(), vertex[v2].z());
	auto p3 = ofVec3f(vertex[v3].x(), vertex[v3].y(), vertex[v3].z());
	auto e1 = p1 - p2;
	auto e2 = p1 - p3;
	auto normal = e1.getCrossed(e2);
	normal = normal.getNormalized();
	return make_tuple(normal.x, normal.y, normal.z);
}

ofVec3f ofApp::calcPointNormal(int i) {
	auto o = vertex[i].nextHalfEdge;
	auto n = halfEdge_map[halfEdge_map[o].prevHalfEdge].pairEdge;
	//find the beginning edge if the vertex is on an boundary edge
	auto p_n = o;
	while ((o != n) & (n != nil_pair)) { p_n = n; n = halfEdge_map[halfEdge_map[n].prevHalfEdge].pairEdge; }
	o = p_n;
	n = o;
	auto f = halfEdge_map[o].face;
	ofVec3f pn(get<0>(face[f].normal), get<1>(face[f].normal), get<2>(face[f].normal));

	while ((o != n) & (n != nil_pair)) {
		f = halfEdge_map[n].face;
		pn += ofVec3f(get<0>(face[f].normal), get<1>(face[f].normal), get<2>(face[f].normal));
		n = halfEdge_map[halfEdge_map[n].pairEdge].nextHalfEdge;
	}
	pn = pn.getNormalized();
	return pn;
}

void ofApp::reconstructHalfEdge(map<pair<int, int>, HalfEdge>& halfEdge_map, map<int, Vertex>& vertex, map<int, Face>& face, map<int, Vertex>& new_vertex, map<pair<int, int>, HalfEdge>& new_halfEdge_map, map<int, Face>& new_face)
{
	new_vertex = vertex;
	map<pair<int, int>, HalfEdge> tmp_halfEdge_map = halfEdge_map;
	map<int, Face> tmp_face = face;
	//split the half edge (and add new vertices)
	while (tmp_halfEdge_map.size() != 0)
	{
		auto he1 = tmp_halfEdge_map.begin()->second;
		auto he2 = HalfEdge();
		if (he1.pairEdge != nil_pair) {
			he2 = tmp_halfEdge_map[he1.pairEdge];
		}
		auto v1 = he1.oriVertex;
		auto v2 = halfEdge_map[halfEdge_map[tmp_halfEdge_map.begin()->first].nextHalfEdge].oriVertex;
		tmp_halfEdge_map.erase(tmp_halfEdge_map.begin());
		if (he1.pairEdge != nil_pair) {
			tmp_halfEdge_map.erase(he1.pairEdge);
		}

		auto vn = new_vertex.size();
		new_vertex.insert(make_pair(vn, Vertex()));
		auto ne1 = make_pair(v1, vn);
		auto ne2 = make_pair(vn, v2);
		auto ne3 = nil_pair;
		auto ne4 = nil_pair;
		if (he1.pairEdge != nil_pair) {
			ne3 = make_pair(v2, vn);
			ne4 = make_pair(vn, v1);
		}

		//add new edge to the new map
		new_halfEdge_map.insert(make_pair(ne1, HalfEdge()));
		new_halfEdge_map.insert(make_pair(ne2, HalfEdge()));
		if (he1.pairEdge != nil_pair) {
			new_halfEdge_map.insert(make_pair(ne3, HalfEdge()));
			new_halfEdge_map.insert(make_pair(ne4, HalfEdge()));
		}

		//add the prev&next edge pointer of the new edges
		new_halfEdge_map[ne1].nextHalfEdge = ne2;
		new_halfEdge_map[ne2].nextHalfEdge = he1.nextHalfEdge;
		new_halfEdge_map[ne1].prevHalfEdge = he1.prevHalfEdge;
		new_halfEdge_map[ne2].prevHalfEdge = ne1;
		if (he1.pairEdge != nil_pair) {
			new_halfEdge_map[ne3].nextHalfEdge = ne4;
			new_halfEdge_map[ne4].nextHalfEdge = he2.nextHalfEdge;
			new_halfEdge_map[ne3].prevHalfEdge = he2.prevHalfEdge;
			new_halfEdge_map[ne4].prevHalfEdge = ne3;
		}
		//change the prev&next edge pointer of the adjust edges of new edges(in the old map)
		if (tmp_halfEdge_map.find(he1.prevHalfEdge) != tmp_halfEdge_map.end())
			tmp_halfEdge_map[he1.prevHalfEdge].nextHalfEdge = ne1;
		else
			new_halfEdge_map[he1.prevHalfEdge].nextHalfEdge = ne1;
		if (tmp_halfEdge_map.find(he1.nextHalfEdge) != tmp_halfEdge_map.end())
			tmp_halfEdge_map[he1.nextHalfEdge].prevHalfEdge = ne2;
		else
			new_halfEdge_map[he1.nextHalfEdge].prevHalfEdge = ne2;
		if (he1.pairEdge != nil_pair) {
			if (tmp_halfEdge_map.find(he2.prevHalfEdge) != tmp_halfEdge_map.end())
				tmp_halfEdge_map[he2.prevHalfEdge].nextHalfEdge = ne3;
			else
				new_halfEdge_map[he2.prevHalfEdge].nextHalfEdge = ne3;
			if (tmp_halfEdge_map.find(he2.nextHalfEdge) != tmp_halfEdge_map.end())
				tmp_halfEdge_map[he2.nextHalfEdge].prevHalfEdge = ne4;
			else
				new_halfEdge_map[he2.nextHalfEdge].prevHalfEdge = ne4;
		}


		//add the face pointer to the new halfedge & change the face's halfedge pointer to the new halfedge(temporary)
		new_halfEdge_map[ne1].face = he1.face;
		new_halfEdge_map[ne2].face = he1.face;
		tmp_face[he1.face].corHalfEdge = ne1;
		if (he1.pairEdge != nil_pair) {
			new_halfEdge_map[ne3].face = he2.face;
			new_halfEdge_map[ne4].face = he2.face;
			tmp_face[he2.face].corHalfEdge = ne3;
		}
		//add the halfedge pointer to the new vertices & the vertex pointer to the halfedge
		new_vertex[vn].nextHalfEdge = nil_pair;
		new_vertex[v1].nextHalfEdge = ne1;
		new_halfEdge_map[ne1].oriVertex = v1;
		new_halfEdge_map[ne2].oriVertex = vn;
		if (he1.pairEdge != nil_pair) {
			new_vertex[v2].nextHalfEdge = ne3;
			new_halfEdge_map[ne3].oriVertex = v2;
			new_halfEdge_map[ne4].oriVertex = vn;
		}
		//add the pair pointer to the new halfedge
		if (he1.pairEdge != nil_pair) {
			new_halfEdge_map[ne1].pairEdge = ne4;
			new_halfEdge_map[ne2].pairEdge = ne3;
			new_halfEdge_map[ne3].pairEdge = ne2;
			new_halfEdge_map[ne4].pairEdge = ne1;
		}
		else {
			new_halfEdge_map[ne1].pairEdge = nil_pair;
			new_halfEdge_map[ne2].pairEdge = nil_pair;
		}

	}
	//add new face & change the face pointer of all halfedge
	for (size_t i = 0; i < tmp_face.size(); i++)
	{
		//reserved center facet index
		auto cfi = new_face.size();
		new_face.insert(make_pair(cfi, Face()));

		auto he1 = tmp_face[i].corHalfEdge;
		auto he2 = new_halfEdge_map[he1].prevHalfEdge;
		auto next_he1 = new_halfEdge_map[new_halfEdge_map[he1].nextHalfEdge].nextHalfEdge;
		auto next_he2 = new_halfEdge_map[new_halfEdge_map[he2].nextHalfEdge].nextHalfEdge;

		auto cfp1 = new_halfEdge_map[new_halfEdge_map[he1].nextHalfEdge].oriVertex;
		auto cfp2 = new_halfEdge_map[he2].oriVertex;
		auto cfp3 = new_halfEdge_map[new_halfEdge_map[next_he1].nextHalfEdge].oriVertex;
		new_face[cfi].corHalfEdge = make_pair(cfp2, cfp1);
		for (size_t j = 0; j < 3; j++)
		{
			auto fi = new_face.size();
			new_face.insert(make_pair(fi, Face()));

			new_face[fi].corHalfEdge = he1;
			new_halfEdge_map[he1].face = fi;
			new_halfEdge_map[he2].face = fi;
			auto p1 = new_halfEdge_map[new_halfEdge_map[he1].nextHalfEdge].oriVertex;
			auto p2 = new_halfEdge_map[he2].oriVertex;
			new_halfEdge_map[make_pair(p1, p2)] = HalfEdge();
			new_halfEdge_map[make_pair(p1, p2)].face = fi;
			new_halfEdge_map[make_pair(p1, p2)].nextHalfEdge = he2;
			new_halfEdge_map[make_pair(p1, p2)].prevHalfEdge = he1;
			new_halfEdge_map[make_pair(p1, p2)].oriVertex = p1;
			new_halfEdge_map[he1].nextHalfEdge = make_pair(p1, p2);
			new_halfEdge_map[he2].prevHalfEdge = make_pair(p1, p2);
			new_vertex[p1].nextHalfEdge = make_pair(p1, p2);
			new_halfEdge_map[make_pair(p1, p2)].pairEdge = make_pair(p2, p1);

			new_halfEdge_map[make_pair(p2, p1)] = HalfEdge();
			new_halfEdge_map[make_pair(p2, p1)].face = cfi;
			new_halfEdge_map[make_pair(p2, p1)].pairEdge = make_pair(p1, p2);
			new_halfEdge_map[make_pair(p2, p1)].oriVertex = p2;

			he1 = next_he1;
			he2 = next_he2;
			next_he1 = new_halfEdge_map[new_halfEdge_map[he1].nextHalfEdge].nextHalfEdge;
			next_he2 = new_halfEdge_map[new_halfEdge_map[he2].nextHalfEdge].nextHalfEdge;
		}

		new_halfEdge_map[make_pair(cfp1, cfp3)].nextHalfEdge = make_pair(cfp3, cfp2);
		new_halfEdge_map[make_pair(cfp3, cfp2)].nextHalfEdge = make_pair(cfp2, cfp1);
		new_halfEdge_map[make_pair(cfp2, cfp1)].nextHalfEdge = make_pair(cfp1, cfp3);
		new_halfEdge_map[make_pair(cfp1, cfp3)].prevHalfEdge = make_pair(cfp2, cfp1);
		new_halfEdge_map[make_pair(cfp3, cfp2)].prevHalfEdge = make_pair(cfp1, cfp3);
		new_halfEdge_map[make_pair(cfp2, cfp1)].prevHalfEdge = make_pair(cfp3, cfp2);
	}
}

void ofApp::drawModelToggled(bool & inval)
{
	if (inval) {
		if (drawWireFrame) {
			drawWireFrame = false;
			drawModel = true;
		}
		else
		{
			drawModel = true;
		}
	}

}

void ofApp::drawWireFrameToggled(bool & inval)
{
	if (inval) {
		if (drawModel) {
			drawModel = false;
			drawWireFrame = true;
		}
		else
		{
			drawWireFrame = true;
		}
	}
}
