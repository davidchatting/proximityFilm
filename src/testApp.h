#ifndef _TEST_APP
#define _TEST_APP

//David Chatting - davidchatting.com - 16th June 2011

#include "ofMain.h"
#include "ofxXmlSettings.h"

#define ACTIVITY_WINDOW_SIZE 100

#define	START_LOOP_STATE	0
#define	PLAY_FORWARD_STATE	1
#define	PLAY_BACKWARD_STATE	2
#define	END_LOOP_STATE		3
#define	SENSOR_FAIL_STATE	4

class testApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	
	int readDistance();
	
	void repositionMovie();
	
	void addValueToWindow(int v);
	void resetWindow();
	
	bool isActive(bool now);
	void resetSensor();
	bool sensorHasFailed();
	
	void keyPressed(int key);
	void keyReleased(int key);
	
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	
	void setupArduino();
	
	ofArduino		arduino;
	bool			bSetupArduino;			// flag variable for setting up arduino once
	ofVideoPlayer	movie;
	
	ofTrueTypeFont	font;
	
	ofxXmlSettings settings;
	
	int sampleInterval;
	int lastSampleTime;
	int nextSampleTime;
	long nextSensorResetTime;
	int sensorResetInactivityInterval;
	bool wasActive;
	
	int window[5];
	
	float minDistance;
	float maxDistance;
	float lastDistance;
	float interPersonalSpace;
	
	float startLoopTime;
	float endLoopTime;
	
	bool activityWindow[ACTIVITY_WINDOW_SIZE];
	
	float startPosition;
	float targetPosition;
	
	int movieFps;
	int movieX;
	int movieY;
	int movieWidth;
	int movieHeight;
	
	int currentState;
	
	bool debug;
	int lastReading;
};

#endif

