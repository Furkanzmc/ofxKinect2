// @author sadmb
// @date 3,Jan,2014
// modified from ofxNI2.cpp of ofxNI2 by @satoruhiga
#ifndef OFX_KINECT2_H
#define OFX_KINECT2_H
#include "ofMain.h"
#include "ofxKinect2Types.h"
#include "utils/DoubleBuffer.h"
#include <array>
#include <assert.h>

namespace ofxKinect2
{
static const int DEPTH_WIDTH = 512;
static const int DEPTH_HEIGHT = 424;

void init();
class Device;
class Stream;

class IrStream;
class ColorStream;
class DepthStream;
class BodyIndexStream;

class Body;
class BodyStream;

class Recorder;

template<class Interface>
inline void safeRelease(Interface *&interfaceToRelease)
{
    if (interfaceToRelease) {
        interfaceToRelease->Release();
        interfaceToRelease = nullptr;
    }
}
} // namespace ofxKinect2

// device
class ofxKinect2::Device
{
    friend class ofxKinect2::Stream;

public:
    ofEvent<ofEventArgs> m_UpdateDeviceEvent;

    Device();
    ~Device();

    bool setup();
    bool setup(string kinect2FilePath);
    void exit();
    void update();

    /*
     bool startRecording(string filename = "");
     void stopRecording();
     bool isRecording() const { return recorder != nullptr; }
    */

    bool isOpen() const;
    void setDepthColorSyncEnabled(bool enabled = true);
    bool isDepthColorSyncEnabled() const;

    DeviceHandle &get();
    const DeviceHandle &get() const;

    ICoordinateMapper *getMapper();
    const ICoordinateMapper *getMapper() const;

protected:
    DeviceHandle m_Device;
    ICoordinateMapper *m_CoordinateMapper;
    std::vector<ofxKinect2::Stream *> m_Streams;
    bool m_IsDepthColorSyncEnabled;
    Recorder *m_Recorder;
};

class ofxKinect2::Recorder
{
};

// stream
class ofxKinect2::Stream : public ofThread
{
public:
    friend class ofxKinect2::Device;

    virtual ~Stream();
    virtual void exit();

    virtual bool open();
    virtual void close();

    virtual void update();
    virtual bool updateMode();

    bool isOpen() const;

    int getWidth() const;
    virtual bool setWidth(int width);

    int getHeight() const;
    virtual bool setHeight(int height);

    virtual bool setSize(int width, int height);

    ofTexture &getTextureReference();

    int getFps();
    bool setFps(int fps);

    void setMirror(bool mirrored = true);
    bool isMirror() const;

    float getHorizontalFieldOfView() const;
    float getVerticalFieldOfView() const;
    float getDiagonalFieldOfView() const;

    bool isFrameNew() const;

    void draw(float x = 0, float y = 0);
    virtual void draw(float x, float y, float w, float h);

    operator StreamHandle &()
    {
        return m_StreamHandle;
    }

    operator const StreamHandle &() const
    {
        return m_StreamHandle;
    }

    StreamHandle &get();
    const StreamHandle &get() const;
    CameraSettingsHandle &getCameraSettings();
    const CameraSettingsHandle &getCameraSettings() const;

protected:
    Frame m_Frame;
    StreamHandle m_StreamHandle;
    CameraSettingsHandle m_CameraSettings;
    uint64_t m_Kinect2Timestamp, m_OpenGLTimestamp;

    bool m_IsFrameNew,
         m_IsTextureNeedUpdate,
         m_IsMirror;

    ofTexture m_Texture;
    Device *m_Device;

protected:
    Stream();
    void threadedFunction();
    bool setup(Device &device, SensorType sensorType);
    virtual bool readFrame(IMultiSourceFrame *multiFrame = nullptr);
    virtual void setPixels(Frame &frame);
};

class ofxKinect2::ColorStream : public ofxKinect2::Stream
{
public:
    bool setup(ofxKinect2::Device &device);
    void exit();
    bool open();
    void close();

    void update();
    bool updateMode();

    bool setWidth(int v);
    bool setHeight(int v);
    bool setSize(int width, int height);

    ofPixels &getPixelsRef();

    int getExposureTime() const;
    int getFrameInterval() const;
    float getGain() const;
    float getGamma() const;

    /*
    void setAutoExposureEnabled(bool yn = true) {  }
    bool getAutoExposureEnabled() {  }

    void setAutoWhiteBalanceEnabled(bool yn = true) {  }
    bool getAutoWhiteBalanceEnabled() {  }
    */

protected:
    DoubleBuffer<ofPixels> m_DoubleBuffer;
    unsigned char *m_Buffer;

protected:
    bool readFrame(IMultiSourceFrame *multiFrame = nullptr);
    void setPixels(Frame &frame);
};

class ofxKinect2::DepthStream : public ofxKinect2::Stream
{
public:
    bool setup(ofxKinect2::Device &device);
    bool open();
    void close();

    void getColorSpacePoints(ColorSpacePoint *colorSpacePointsFromDepth);
    int getNumberColorSpacePoints() const;

    void getCameraSpacePoints(CameraSpacePoint *cameraSpacePointsFromDepth);
    int getNumberCameraSpacePoints() const;

    void update();
    bool updateMode();

    ofShortPixels &getPixelsRef();
    ofShortPixels getPixelsRef(int nearValue, int farValue, bool invert = false);

    void setNear(float nearValue);
    float getNear() const;

    void setFar(float farValue);
    float getFar() const;

    void setInvert(float invert);
    bool getInvert() const;

//    ofVec3f getWorldCoordinateAt(int x, int y);

protected:
    DoubleBuffer<ofShortPixels> m_DoubleBuffer;

    float m_NearValue, m_FarValue;
    bool m_IsInvert;

protected:
    bool readFrame(IMultiSourceFrame *multiFrame = nullptr);
    void setPixels(Frame &frame);
};

//----------------------------------------------------------
#pragma mark BodyIndexStream
//----------------------------------------------------------

class ofxKinect2::BodyIndexStream : public ofxKinect2::Stream
{
public:
    bool open();
    void close();

    void update();
    bool updateMode();

    bool setup(ofxKinect2::Device &device);
    const ofShortPixels &getPixelsRef() const;
    void setInvert(float invert);
    bool getInvert() const;

protected:
    DoubleBuffer<ofShortPixels> m_DoubleBuffer;
    bool m_IsInvert;

protected:
    bool readFrame(IMultiSourceFrame *multiFrame = nullptr);
    void setPixels(Frame &frame);
};

class ofxKinect2::IrStream : public ofxKinect2::Stream
{
public:
    bool setup(ofxKinect2::Device &device);
    bool open();
    void close();

    void update();
    bool updateMode();

    ofShortPixels &getPixelsRef();

protected:
    DoubleBuffer<ofShortPixels> m_DoubleBuffer;

protected:
    bool readFrame(IMultiSourceFrame *multiFrame = nullptr);
    void setPixels(Frame &frame);

};

class ofxKinect2::Body
{
public:
    friend class BodyStream;
    typedef ofPtr<Body> Ref;

    Body();
    void setup(ofxKinect2::Device &m_Device, IBody *body);
    void close();
    void update();
    void drawBody(bool draw3D = false);
    void drawBone(JointType joint0, JointType joint1, bool draw3D = false);
    void drawHand(JointType handType, bool draw3D = false);
    void drawHandLeft(bool draw3D = false);
    void drawHandRight(bool draw3D = false);
    void drawHands(bool draw3D = false);

    UINT64 getId() const;

    HandState getLeftHandState() const;
    HandState getRightHandState() const;

    size_t getNumJoints();
    const Joint &getJoint(size_t idx);

    const ofPoint &getJointPoint(size_t idx);
    const std::array<ofPoint, JointType_Count> &getJointPoints();

    bool IsInitialized();

private:
    bool m_IsInitialized;
    Device *m_Device;
    UINT64 m_TrackingID;
    Joint m_Joints[JointType_Count];
    std::array<ofPoint, JointType_Count> m_JointPoints;

    HandState m_LeftHandState;
    HandState m_RightHandState;

private:
    ofPoint bodyToScreen(const CameraSpacePoint &bodyPoint, int width, int height);
};

class ofxKinect2::BodyStream : public Stream
{
public:
    bool setup(ofxKinect2::Device &device);
    bool open();
    void close();

    void update();
    bool updateMode();

    void draw();
    void drawHands();
    void drawHandLeft();
    void drawHandRight();

    void draw(int x, int y, int w, int h);

    size_t getNumBodies();
    const Body *getBodyUsingIdx(int idx);
    const Body *getBody(UINT64 id);
    ofShortPixels &getPixelsRef();
    ofShortPixels getPixelsRef(int _near, int _far, bool invert = false);

protected:
    DoubleBuffer<ofShortPixels> m_DoubleBuffer;
    std::vector<Body *> m_Bodies;

protected:
    bool readFrame(IMultiSourceFrame *multiFrame = nullptr);
    void setPixels(Frame &frame);

};

#endif //OFX_KINECT2_H
