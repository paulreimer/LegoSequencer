#include "testApp.h"

#define PARAM(name) params.get(name, desc[name]["default"])

//--------------------------------------------------------------
void
testApp::setup()
{
	ofBackground(0,0,0);
	
	font.loadFont("fonts/FF_DIN_Medium.ttf",12);
	
#ifdef _USE_LIVE_VIDEO
	vidGrabber.setVerbose(true);
	vidGrabber.initGrabber(VIDEO_SIZE);
#else
	vidPlayer.loadMovie("fingers.mov");
	vidPlayer.play();
#endif
	
	imgs[IMAGE_INPUT]		= new ofxCvColorImage();
	imgs[IMAGE_FOREGROUND]	= new ofxCvColorImage();
	imgs[IMAGE_SKIN]		= new ofxCvGrayscaleImage();
	imgs[IMAGE_MEDIAN]		= new ofxCvGrayscaleImage();
	imgs[IMAGE_BINARY]		= new ofxCvGrayscaleImage();
	imgs[IMAGE_DILATED]		= new ofxCvGrayscaleImage();
	
	titles[IMAGE_INPUT]		= "Input";
	titles[IMAGE_FOREGROUND]= "Foreground";
	titles[IMAGE_SKIN]		= "Skin RCA";
	titles[IMAGE_MEDIAN]	= "Median Filter";
	titles[IMAGE_BINARY]	= "Binary";
	titles[IMAGE_DILATED]	= "Dilated";
	
	map<short,ofxCvImage*>::iterator img_iter;
	for (img_iter=imgs.begin(); img_iter!=imgs.end(); img_iter++)
	{
		img_iter->second->allocate(VIDEO_SIZE);
		mats[img_iter->first] = cv::Mat(img_iter->second->getCvImage());
	}

	Json::Reader reader;
	Json::Value guiJson;
	
	string guiFile(ofToDataPath("gui.json"));
	std::ifstream guiFileContents(guiFile.c_str());
	
	if (!reader.parse(guiFileContents, guiJson))
		std::cout
		<< "Failed to parse configuration\n"
		<< reader.getFormatedErrorMessages()
		<< std::endl;
	
	params = guiJson["params"];
	desc = guiJson["desc"];
}

//--------------------------------------------------------------
void testApp::exit()
{
	map<short,ofxCvImage*>::iterator img_iter;
	for (img_iter=imgs.begin(); img_iter!=imgs.end(); img_iter++)
	{
		img_iter->second->clear();
		delete img_iter->second;
	}
	mats.clear();
	imgs.clear();
}

//--------------------------------------------------------------
void
testApp::update()
{
	bool bNewFrame = false;
	
#ifdef _USE_LIVE_VIDEO
	vidGrabber.grabFrame();
	bNewFrame = vidGrabber.isFrameNew();
#else
	vidPlayer.idleMovie();
	bNewFrame = vidPlayer.isFrameNew();
#endif
	
	if (bNewFrame)
	{
#ifdef _USE_LIVE_VIDEO
		imgs[IMAGE_INPUT]->setFromPixels(vidGrabber.getPixels(), VIDEO_SIZE);
#else
		imgs[IMAGE_INPUT]->setFromPixels(vidPlayer.getPixels(), VIDEO_SIZE);
#endif
		skinRCA(mats[IMAGE_INPUT], mats[IMAGE_SKIN]);
		
		cv::medianBlur(mats[IMAGE_SKIN], mats[IMAGE_MEDIAN], PARAM("medianKernelSize").asInt()|1);
		cv::threshold(mats[IMAGE_MEDIAN], mats[IMAGE_BINARY], 1, 255, cv::THRESH_BINARY);
		
		cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS,
													cv::Size(PARAM("elementSize").asInt(),
															 PARAM("elementSize").asInt()));
		
		//		cv::dilate(mats[IMAGE_BINARY], mats[IMAGE_DILATED],
		//				   element, cv::Point(-1,-1), PARAM("iterations"));
		
		cv::morphologyEx(mats[IMAGE_BINARY], mats[IMAGE_DILATED],
						 cv::MORPH_CLOSE, element, cv::Point(-1,-1), PARAM("iterations").asInt());
		
		mats[IMAGE_FOREGROUND] = cv::Scalar(0);
		mats[IMAGE_INPUT].copyTo(mats[IMAGE_FOREGROUND], mats[IMAGE_BINARY]);
		
		contours.findContours(*(ofxCvGrayscaleImage*)imgs[IMAGE_BINARY],
							  PARAM("minContourArea").asInt(),	PARAM("maxContourArea").asInt(),
							  PARAM("nContours").asInt(),		PARAM("findHoles").asBool());
		
		map<short,ofxCvImage*>::iterator img_iter;
		for (img_iter=imgs.begin(); img_iter!=imgs.end(); img_iter++)
		{
			ofxCvImage*	img = imgs[img_iter->first];
			ofTexture&	tex = img->getTextureReference();
			int glchannels = (img->getCvImage()->nChannels==1)? GL_LUMINANCE:GL_RGB;
			
			img->flagImageChanged();
			tex.loadData(img->getPixels(), img->width, img->height, glchannels);
		}		
	}
}

//--------------------------------------------------------------
void
testApp::draw()
{
	ofPoint padding(8, 8);
	
	ofSetColor(0xffffff);
	
	ofRectangle frame;
	frame.x			= -320 - 2*padding.x;
	frame.y			= 0;
	frame.width		= 320;
	frame.height	= 240;
	
	glPushMatrix();
	ofTranslate(padding.x, padding.y, 0.);
	
	map<short,ofxCvImage*>::iterator img_iter;
	for (img_iter=imgs.begin(); img_iter!=imgs.end(); img_iter++)
	{
		advanceFrame(frame, padding);
		renderImage(img_iter, frame);
	}
	
	advanceFrame(frame, padding);
	ofTranslate(frame.x, frame.y, 0.);
	ofScale(frame.width/contours.getWidth(), frame.height/contours.getHeight(), 1.);
	for (int i=0; i<contours.nBlobs; ++i)
        contours.blobs[i].draw(0,0);
	
	glPopMatrix();
}


//--------------------------------------------------------------
void testApp::advanceFrame(ofRectangle& frame, ofPoint& padding)
{
	if (frame.x+2*(frame.width+2*padding.x) > ofGetWidth())
	{
		frame.y +=	padding.y + frame.height + padding.y;
		frame.x =	0;
	}
	else {
		frame.x +=	padding.x + frame.width + padding.x;
	}
}

//--------------------------------------------------------------
void testApp::renderImage(map<short,ofxCvImage*>::iterator img_iter,
						  ofRectangle frame)
{
	map<short,string>::iterator title_iter;
	
	img_iter->second->draw(frame.x, frame.y, frame.width, frame.height);
	title_iter = titles.find(img_iter->first);
	if (title_iter != titles.end())
	{
		ofSetColor(0xffffff);
		font.drawString(title_iter->second,
						frame.x + frame.width	- font.stringWidth(title_iter->second),
						frame.y + frame.height	- font.stringHeight(title_iter->second));
	}
}

//--------------------------------------------------------------
void
testApp::keyPressed(int key)
{}

//--------------------------------------------------------------
void
testApp::keyReleased(int key)
{}

//--------------------------------------------------------------
void
testApp::mouseMoved(int x, int y)
{}

//--------------------------------------------------------------
void
testApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void
testApp::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void
testApp::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void
testApp::windowResized(int w, int h)
{}

