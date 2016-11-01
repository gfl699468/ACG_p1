#include "ofApp.h"
using namespace std;
//--------------------------------------------------------------
void ofApp::setup(){
	panel.setup();
	panel.add(openFileButton.setup("open Model file"));
	panel.add(drawModel.setup("draw Model", false));
	openFileButton.addListener(this, &ofApp::openFileButtonPressed);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	panel.draw();
	if (drawModel) {
		if (halfEdge_map.size() != 0) {

		}
		else {
			ofSystemAlertDialog("Please load model file first!");
			drawModel = false;
		}
	}
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
	ofFileDialogResult result = ofSystemLoadDialog("Load VRML file");
	if (result.bSuccess) {
		string path = result.getPath();
		reader.loadVRMLFile(path);
		this->halfEdge_map = reader.halfEdge_map;
		this->vertex = reader.vertex;
		this->face = reader.face;

	}
}

void ofApp::loopSubdivisionButtonPressed()
{
}

void ofApp::modifiedButterflySubdivisionButtonPressed()
{
}

void ofApp::updateModelvbo()
{
	for (size_t i = 0; i < vertex.size(); i++)
	{
		mesh.addVertex(ofVec3f(vertex[i].x, vertex[i].y, vertex[i].z));
	}
	for (size_t i = 0; i < face.size(); i++)
	{

		mesh.addIndex(halfEdge_map[face[i].corHalfEdge].oriVertex);
		mesh.addIndex(halfEdge_map[halfEdge_map[face[i].corHalfEdge].nextHalfEdge].oriVertex);
		mesh.addIndex(halfEdge_map[halfEdge_map[face[i].corHalfEdge].prevHalfEdge].oriVertex);
	}
}
