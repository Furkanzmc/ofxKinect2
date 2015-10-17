#pragma once
#include "ofMain.h"
#include "ofxKinect2.h"

class ofApp : public ofBaseApp
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
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void exit();

private:
    ofxKinect2::Device m_Device;
    ofxKinect2::ColorStream m_ColorStream;
    ofxKinect2::DepthStream m_DepthStream;
    ofxKinect2::BodyIndexStream m_BodyIndexStream;
    ofxKinect2::BodyStream m_BodyStream;
    ofxKinect2::IrStream m_IrStream;
};
