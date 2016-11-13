#pragma once

#include "ofMain.h"
#include <ofxGui.h>
#include "Reader.h"
#include "Writer.h"
#include <math.h>

class ofApp : public ofBaseApp{
	ofxPanel panel;
	ofxButton openFileButton;
	ofxButton saveFileButton;
	ofxToggle drawModel;
	ofxToggle drawWireFrame;
	ofxFloatSlider modelScale;
	ofxVec3Slider lightPos;
	ofxButton loopSubdivisionButton;
	ofxButton modifiedButterflySubdivisionButton;
	Reader reader;
	Writer writer;

	map<pair<int, int>, HalfEdge> halfEdge_map;
	map<int, Vertex> vertex;
	map<int, Face> face;
	ofVboMesh mesh;
	ofEasyCam cam;
	ofLight light;
	ofLight ambientLight;

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
		void saveFileButtonPressed();

		void loopSubdivisionButtonPressed();
		void modifiedButterflySubdivisionButtonPressed();
		void updateModelvbo();
		void calcNormals();
		tuple<double, double, double> calcFaceNormal(int i);
		ofVec3f calcPointNormal(int i);

		void drawModelToggled(bool & inval);
		void drawWireFrameToggled(bool & inval);
};
