#pragma once

#include "ofMain.h"
#include <ofxGui.h>
#include "Reader.h"
class ofApp : public ofBaseApp{
	ofxPanel panel;
	ofxButton openFileButton;
	ofxToggle drawModel;
	ofxButton loopSubdivisionButton;
	ofxButton modifiedButterflySubdivisionButton;
	Reader reader = Reader();

	map<pair<int, int>, HalfEdge> halfEdge_map;
	map<int, Vertex> vertex;
	map<int, Face> face;
	ofVboMesh mesh;

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void openFileButtonPressed();
		void loopSubdivisionButtonPressed();
		void modifiedButterflySubdivisionButtonPressed();
		void updateModelvbo();
};
