#include "ofApp.h"

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
	}
}

void ofApp::loopSubdivisionPressed()
{
}

void ofApp::modifiedButterflySubdivisionPressed()
{
}
