#pragma once

#include "ofMain.h"
#include <ofxGui.h>
#include "Reader.h"
#include <math.h>
class ofApp : public ofBaseApp{
	ofxPanel panel;
	ofxButton openFileButton;
	ofxToggle drawModel;
	ofxFloatSlider modelScale;
	ofxVec3Slider lightPos;
	ofxButton loopSubdivisionButton;
	ofxButton modifiedButterflySubdivisionButton;
	Reader reader;

	map<pair<int, int>, HalfEdge> halfEdge_map;
	map<int, Vertex> vertex;
	map<int, Face> face;
	ofVboMesh mesh;
	ofEasyCam cam;
	ofLight light;

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
		void calcNormals();
		tuple<double, double, double> calcFaceNormal(int i);
		ofVec3f calcPointNormal(int i);
};
