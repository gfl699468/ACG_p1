#include "ofApp.h"

#define nil_pair make_pair(-1, -1)

using namespace std;
//--------------------------------------------------------------
void ofApp::setup(){
	panel.setup();
	panel.add(openFileButton.setup("open Model file"));
	panel.add(drawModel.setup("draw Model", false));
	panel.add(modelScale.setup("Model Scale", 100, 1, 1000));
	panel.add(lightPos.setup("Light Position", ofVec3f(0, 0, 100), ofVec3f(-100, -100, -100), ofVec3f(100, 100, 100)));
	openFileButton.addListener(this, &ofApp::openFileButtonPressed);
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	ofEnableLighting();
	ofEnableSmoothing();
	light.enable();
	light.setPointLight();
	light.setPosition(lightPos);

}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	light.setPosition(lightPos);
	ofBackgroundGradient(ofColor(64), ofColor(0));
	cam.begin();
	ofEnableDepthTest();

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
	ofDisableDepthTest();
	cam.end();
	panel.draw();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

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

void ofApp::loopSubdivisionButtonPressed()
{
	map<int, Vertex> tmp_vertex = vertex;
	map<pair<int, int>, HalfEdge> tmp_halfEdge_map = halfEdge_map;
	map<pair<int, int>, HalfEdge> new_halfEdge_map;
	map<int, Face> tmp_face = face;
	map<int, Face> new_face;
	//split the half edge (and add new vertices)
	while (tmp_halfEdge_map.size() != 0)
	{
		auto he1 = tmp_halfEdge_map.begin()->second;
		auto he2 = tmp_halfEdge_map[he1.pairEdge];
		tmp_halfEdge_map.erase(tmp_halfEdge_map.begin());
		tmp_halfEdge_map.erase(he1.pairEdge);
		auto v1 = he1.oriVertex;
		auto v2 = he2.oriVertex;
		auto vn = tmp_vertex.size();
		tmp_vertex.insert(make_pair(vn, Vertex()));
		auto ne1 = make_pair(v1, vn);
		auto ne2 = make_pair(vn, v2);
		auto ne3 = make_pair(v2, vn);
		auto ne4 = make_pair(vn, v1);
		//add new edge to the new map
		new_halfEdge_map.insert(make_pair(ne1, HalfEdge()));
		new_halfEdge_map.insert(make_pair(ne2, HalfEdge()));
		new_halfEdge_map.insert(make_pair(ne3, HalfEdge()));
		new_halfEdge_map.insert(make_pair(ne4, HalfEdge()));
		//add the prev&next edge pointer of the new edges
		new_halfEdge_map[ne1].nextHalfEdge = ne2;
		new_halfEdge_map[ne2].nextHalfEdge = he1.nextHalfEdge;
		new_halfEdge_map[ne3].nextHalfEdge = ne4;
		new_halfEdge_map[ne4].nextHalfEdge = he2.nextHalfEdge;
		new_halfEdge_map[ne1].prevHalfEdge = he1.prevHalfEdge;
		new_halfEdge_map[ne2].prevHalfEdge = ne1;
		new_halfEdge_map[ne3].prevHalfEdge = he2.prevHalfEdge;
		new_halfEdge_map[ne4].prevHalfEdge = ne3;
		//change the prev&next edge pointer of the adjust edges of new edges(in the old map)
		tmp_halfEdge_map[he1.prevHalfEdge].nextHalfEdge = ne1;
		tmp_halfEdge_map[he2.prevHalfEdge].nextHalfEdge = ne3;
		tmp_halfEdge_map[he1.nextHalfEdge].prevHalfEdge = ne2;
		tmp_halfEdge_map[he2.nextHalfEdge].prevHalfEdge = ne4;
		//add the face pointer to the new halfedge & change the face's halfedge pointer to the new halfedge(temporary)
		new_halfEdge_map[ne1].face = he1.face; 
		new_halfEdge_map[ne2].face = he1.face;
		new_halfEdge_map[ne3].face = he2.face;
		new_halfEdge_map[ne4].face = he2.face;
		face[he1.face].corHalfEdge = ne1;
		face[he2.face].corHalfEdge = ne3;
		//add the halfedge pointer to the new vertices
		tmp_vertex[vn].nextHalfEdge = nil_pair;
		tmp_vertex[v1].nextHalfEdge = ne1;
		tmp_vertex[v2].nextHalfEdge = ne3;
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
		auto cfp3 = new_halfEdge_map[new_halfEdge_map[new_halfEdge_map[tmp_vertex[cfp1].nextHalfEdge].nextHalfEdge].nextHalfEdge].oriVertex;
		new_face[cfi].corHalfEdge = make_pair(cfp2, cfp1);
		for (size_t j = 0; j < 3; i++)
		{
			auto fi = new_face.size();
			new_face.insert(make_pair(fi, Face()));
			
			new_face[fi].corHalfEdge = tmp_face[i].corHalfEdge;
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
			tmp_vertex[p1].nextHalfEdge = make_pair(p1, p2);

			new_halfEdge_map[make_pair(p2, p1)] = HalfEdge();
			new_halfEdge_map[make_pair(p2, p1)].face = cfi;
			new_halfEdge_map[make_pair(p2, p1)].pairEdge = make_pair(p1, p2);

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

	//Finished construct the halfedge, begin to adjust the vertices.


}

void ofApp::modifiedButterflySubdivisionButtonPressed()
{
}

void ofApp::updateModelvbo()
{
	mesh.clear();
	for (size_t i = 0; i < vertex.size(); i++)
	{
		mesh.addVertex(ofVec3f(vertex[i].x, vertex[i].y, vertex[i].z));
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
	for (size_t i = 0; i < vertex.size(); i++)
	{
		mesh.addNormal(calcPointNormal(i));
	}
}

tuple<double, double, double> ofApp::calcFaceNormal(int i) {
	auto v1 = halfEdge_map[face[i].corHalfEdge].oriVertex;
	auto v2 = halfEdge_map[halfEdge_map[face[i].corHalfEdge].nextHalfEdge].oriVertex;
	auto v3 = halfEdge_map[halfEdge_map[face[i].corHalfEdge].prevHalfEdge].oriVertex;
	auto p1 = ofVec3f(vertex[v1].x, vertex[v1].y, vertex[v1].z);
	auto p2 = ofVec3f(vertex[v2].x, vertex[v2].y, vertex[v2].z);
	auto p3 = ofVec3f(vertex[v3].x, vertex[v3].y, vertex[v3].z);
	auto e1 = p1 - p2;
	auto e2 = p1 - p3;
	auto normal = e1.getCrossed(e2);
	normal = normal.getNormalized();
	return make_tuple(normal.x, normal.y, normal.z);
}

ofVec3f ofApp::calcPointNormal(int i) {
	auto o = vertex[i].nextHalfEdge;
	auto n = halfEdge_map[halfEdge_map[o].pairEdge].nextHalfEdge;
	auto f = halfEdge_map[o].face;
	ofVec3f pn(get<0>(face[f].normal), get<1>(face[f].normal), get<2>(face[f].normal));
	while (o != n) {
		f = halfEdge_map[n].face;
		pn = pn + ofVec3f(get<0>(face[f].normal), get<1>(face[f].normal), get<2>(face[f].normal));
		n = halfEdge_map[halfEdge_map[n].pairEdge].nextHalfEdge;
	}
	pn = pn.getNormalized();
	return pn;
}

void