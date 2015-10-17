#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetFrameRate(60);
    m_Device.setup();
    if (m_ColorStream.setup(m_Device)) {
        m_ColorStream.open();
    }
    if (m_DepthStream.setup(m_Device)) {
        m_DepthStream.open();
    }
    if (m_BodyIndexStream.setup(m_Device)) {
        m_BodyIndexStream.open();
    }
    if (m_BodyStream.setup(m_Device)) {
        m_BodyStream.open();
    }
    if (m_IrStream.setup(m_Device)) {
        m_IrStream.open();
    }
}

//--------------------------------------------------------------
void ofApp::update()
{
    m_Device.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    m_ColorStream.draw(0, 0, 640, 480);
    m_DepthStream.draw(0, 480);
    m_BodyIndexStream.draw(ofxKinect2::DEPTH_WIDTH, 480);
    m_IrStream.draw(680, 0);
    m_BodyStream.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{

}

void ofApp::exit()
{
    m_ColorStream.close();
    m_DepthStream.close();
    m_BodyIndexStream.close();
    m_BodyStream.close();
    m_IrStream.close();
    m_Device.exit();
}
