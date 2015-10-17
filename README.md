ofxKinect2
==========
This addon is based on [Lucie Bélanger](https://github.com/lucieb)'s work which is based on [sadmb](https://github.com/sadmb) work. :D

I added BodyIndexStream and re-factored the code based on my preferences.
It requires Kinect2 device and KinectSDK-v2.0 installed and it only works on Windows 8 and above.

## Usage
Make sure to add the include path and the library for Kinect v2
Just set up the streams you want to use and then draw them or do whatever you like.
```C++
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
```

## Screenshot
![Screenshot][1]

[1]: http://i.imgur.com/DsZMS0n.png
