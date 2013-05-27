#include "testApp.h"

//David Chatting - davidchatting.com - 16th June 2011

void testApp::setup(){
	debug=false;
	
	ofSetVerticalSync(true);
	
	ofBackground(0,0,0);
	ofHideCursor();
    ofSetFrameRate(50);
	
	font.loadFont("DIN.otf",64);
	
    if(!settings.loadFile("settings.xml") ){
	    printf("settings.xml not found - defaults will be used");
    }
	
	arduino.connect(settings.getValue("SETTINGS:ARDUINO-DEV", ""), settings.getValue("SETTINGS:ARDUINO-SPEED", 57600));
	bSetupArduino	= false;

	movie.loadMovie(settings.getValue("SETTINGS:MOVIE", ""));
	movieFps=25;
	repositionMovie();
	//movie.setPaused(true);
	movie.setLoopState(OF_LOOP_NONE);
	
    minDistance=settings.getValue("SETTINGS:MIN-DISTANCE", 15.0f);
    maxDistance=settings.getValue("SETTINGS:MAX-DISTANCE", 75.0f);
	lastDistance=maxDistance;
	interPersonalSpace=settings.getValue("SETTINGS:INTERPERSONAL-SPACE", 50.0f);
	
	startLoopTime=settings.getValue("SETTINGS:START-LOOP-TIME", 0.0f);
	endLoopTime=settings.getValue("SETTINGS:END-LOOP-TIME", 0.0f);
	
    sampleInterval=settings.getValue("SETTINGS:SAMPLE-INTERVAL", 200);   //ms
	nextSampleTime=0;		//ms
	sensorResetInactivityInterval=settings.getValue("SETTINGS:SENSOR-RESET-INACTIVITY-INTERVAL", 60000); //ms
	
	wasActive=false;
	currentState=START_LOOP_STATE;
	movie.play();
}

void testApp::repositionMovie(){
	float screenAspectRatio=ofGetWidth()/ofGetHeight();
	float movieAspectRatio=movie.width/(float)movie.height;
	if(movieAspectRatio<screenAspectRatio){
		movieWidth=ofGetWidth();
		movieHeight=movieWidth/movieAspectRatio;
	}
	else {
		movieHeight=ofGetHeight();
		movieWidth=movieHeight*movieAspectRatio;
	}
	
	movieX=(ofGetWidth()-movieWidth)/2;
	movieY=(ofGetHeight()-movieHeight)/2;
}

//--------------------------------------------------------------
void testApp::update(){
	//cout << currentState << endl;
	
	if(sensorHasFailed()) {
		currentState=SENSOR_FAIL_STATE;
		movie.setLoopState(OF_LOOP_PALINDROME);
	}
	else {
		if(currentState==SENSOR_FAIL_STATE)	currentState=END_LOOP_STATE;
		movie.setLoopState(OF_LOOP_NONE);	//doesn't work if you put this in setup()!
	}
	
	int time=ofGetElapsedTimeMillis();
	if(nextSensorResetTime>0 && nextSensorResetTime<time){
		resetSensor();
		nextSensorResetTime=ofGetElapsedTimeMillis()+sensorResetInactivityInterval;
	}
	
	if(nextSampleTime<time){
		nextSampleTime=time+sampleInterval;
		
		int distance=readDistance();
		addValueToWindow(distance);
		lastReading=distance;
		
		if(distance>=0){
			isActive(distance>minDistance && distance<maxDistance);
			
			distance=max(distance,(int)minDistance);
			distance=min(distance,(int)maxDistance);
			
			if(distance>interPersonalSpace){
				if(currentState==END_LOOP_STATE || currentState==PLAY_FORWARD_STATE){
					currentState=PLAY_BACKWARD_STATE;
				}
			}
			else{
				if(currentState==START_LOOP_STATE || currentState==PLAY_BACKWARD_STATE){
					currentState=PLAY_FORWARD_STATE;
				}
			}
			
			/*
			float d=lastDistance-distance;
			if(abs((int)d)>movementThreshold){
				if(d>0)	currentState=PLAY_FORWARD_STATE;
				else	currentState=PLAY_BACKWARD_STATE;
			}
			*/
			lastSampleTime=time;
			lastDistance=distance;
		}
		else {
		}
	}
	else {
	}
	
	if(movie.getPosition()==0.0f) {
		movie.setSpeed(1.0f);
	}
	if(movie.getPosition()==1.0f) {
		movie.setSpeed(-1.0f);
	}
	
	float t=(movie.getPosition()*(movie.getDuration()*1000));
	if(currentState==START_LOOP_STATE){
		if((movie.getSpeed()==1.0f) && t>startLoopTime){
			movie.setSpeed(-1);
		}
	}
	
	if(currentState==END_LOOP_STATE){
		if((movie.getSpeed()==-1.0f) && t<endLoopTime){
			movie.setSpeed(1);
		}
	}
	
	if(currentState==PLAY_FORWARD_STATE) {
		movie.setSpeed(1);
		if(t>endLoopTime){
			currentState=END_LOOP_STATE;
		}
	}
	
	if(currentState==PLAY_BACKWARD_STATE){
		movie.setSpeed(-1);
		if(t<startLoopTime){
			currentState=START_LOOP_STATE;
		}
	}
	
	movie.idleMovie();
}

int testApp::readDistance(){
    int distance=-1;
	
    if (arduino.isArduinoReady()){
        if (!bSetupArduino) setupArduino();
        arduino.update();
		
		distance=arduino.getAnalog(0);
		if(distance==0) distance=-1;
	}
	//distance=minDistance+((mouseX*(maxDistance-minDistance))/ofGetWidth());
	
    return(distance);
}

void testApp::setupArduino(){
    if (!bSetupArduino){
        arduino.sendAnalogPinReporting(0, ARD_ANALOG);
        arduino.sendDigitalPinMode(2,ARD_OUTPUT);
        arduino.sendDigital(2,ARD_HIGH,true);
		arduino.sendDigital(13,ARD_HIGH,true);
        bSetupArduino = true;
    }
}

//--------------------------------------------------------------
void testApp::draw(){
	movie.draw(movieX,movieY+movieHeight,movieWidth,movieHeight*-1);
	
	if(debug){
		ofSetColor(0xffffff);
		char *tempString=new char[16];
		sprintf(tempString,"%i", lastReading);
		font.drawString(tempString,10,80);
		
		switch (currentState) {
			case START_LOOP_STATE: font.drawString("START_LOOP_STATE",10,160);break;
			case PLAY_FORWARD_STATE: font.drawString("PLAY_FORWARD_STATE",10,160);break;
			case PLAY_BACKWARD_STATE: font.drawString("PLAY_BACKWARD_STATE",10,160);break;
			case END_LOOP_STATE: font.drawString("END_LOOP_STATE",10,160);break;
			case SENSOR_FAIL_STATE: font.drawString("SENSOR_FAIL_STATE",10,160);break;
		}
		
		delete(tempString);
	}
}

bool testApp::sensorHasFailed(){
	bool failed=true;
	
    for(int i=0;i<5;++i){
		failed&=(window[i]==-1);
	}
	
	return(failed);
}

void testApp::addValueToWindow(int v){
    for(int i=0;i<5-1;++i){
		window[i]=window[i+1];
	}
	window[4]=v;
}

void testApp::resetWindow(){
    for(int i=0;i<5;++i){
		window[i]=0;
	}
}

bool testApp::isActive(bool now){
    bool isActive=false;
	
    for(int n=ACTIVITY_WINDOW_SIZE;n>0;--n){
        activityWindow[n]=activityWindow[n-1];
        isActive|=activityWindow[n];
    }
    activityWindow[0]=now;
    isActive|=activityWindow[0];
	
    if(wasActive && !isActive){
        printf("now idle\n");
        nextSensorResetTime=ofGetElapsedTimeMillis()+sensorResetInactivityInterval;
    }
    else if(!wasActive && isActive){
        printf("now active\n");
        nextSensorResetTime=-1;
    }
    wasActive=isActive;
	
    return(isActive);
}

void testApp::resetSensor(){
    printf("reset sensor\n");
	arduino.sendDigital(2,ARD_LOW,true);
	arduino.sendDigital(13,ARD_LOW,true);
	ofSleepMillis(300);
	arduino.sendDigital(2,ARD_HIGH,true);
	arduino.sendDigital(13,ARD_HIGH,true);
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){
	switch (key)
	{
		case ' ':debug=!debug;break;
		default:break;
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){	
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	
}

