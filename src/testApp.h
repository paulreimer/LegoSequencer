#pragma once

#include "ofMain.h"

#include "json.h"

class testApp
: public ofBaseApp
{
public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	
	ofTrueTypeFont font;
	
	map<short,ofxCvImage*>	imgs;
	map<short,cv::Mat>		mats;
	map<short,string>		titles;
	
	int iterations;
	
	enum cv_stages
	{
		IMAGE_INPUT,
		IMAGE_FOREGROUND,
		IMAGE_SKIN,
		IMAGE_MEDIAN,
		IMAGE_BINARY,
		IMAGE_DILATED,
	};
	
	Json::Value params;
	Json::Value desc;
};

