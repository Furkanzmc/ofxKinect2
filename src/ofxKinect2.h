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
typedef unsigned int BodyIndex;

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

//----------------------------------------------------------
#pragma mark - Device
//----------------------------------------------------------
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

//----------------------------------------------------------
#pragma mark - Stream
//----------------------------------------------------------
class ofxKinect2::Stream : public ofThread
{
public:
    friend class ofxKinect2::Device;

    virtual ~Stream();
    virtual bool open();
    virtual void close();
    virtual void exit();

    virtual void update();
    virtual bool updateMode();

    bool isOpen() const;

    virtual bool setSize(int width, int height);
    ofTexture &getTextureReference();

    int getWidth() const;
    virtual bool setWidth(int width);

    int getHeight() const;
    virtual bool setHeight(int height);

    int getFps() const;
    bool setFps(int fps);

    float getHorizontalFieldOfView() const;
    float getVerticalFieldOfView() const;
    float getDiagonalFieldOfView() const;

    bool isFrameNew() const;

    void setMirror(bool mirrored = true);
    bool isMirror() const;

    void draw(float x = 0, float y = 0);
    virtual void draw(float x, float y, float w, float h);

    StreamHandle &get();
    const StreamHandle &get() const;
    CameraSettingsHandle &getCameraSettings();
    const CameraSettingsHandle &getCameraSettings() const;

    operator StreamHandle &()
    {
        return m_StreamHandle;
    }

    operator const StreamHandle &() const
    {
        return m_StreamHandle;
    }

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

//----------------------------------------------------------
#pragma mark - ColorStream
//----------------------------------------------------------
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

protected:
    DoubleBuffer<ofPixels> m_DoubleBuffer;
    unsigned char *m_Buffer;

protected:
    bool readFrame(IMultiSourceFrame *multiFrame = nullptr);
    void setPixels(Frame &frame);
};

//----------------------------------------------------------
#pragma mark - DepthStream
//----------------------------------------------------------
class ofxKinect2::DepthStream : public ofxKinect2::Stream
{
public:
    bool setup(ofxKinect2::Device &device);
    bool open();
    void close();
    void update();
    bool updateMode();

    void getColorSpacePoints(ColorSpacePoint *colorSpacePointsFromDepth);
    int getNumberColorSpacePoints() const;

    int getNumberCameraSpacePoints() const;
    void getCameraSpacePoints(CameraSpacePoint *cameraSpacePointsFromDepth);

    ofShortPixels &getPixelsRef();
    ofShortPixels getPixelsRef(int nearValue, int farValue, bool invert = false);

    void setNear(float nearValue);
    float getNear() const;

    void setFar(float farValue);
    float getFar() const;

    void setInvert(float invert);
    bool getInvert() const;

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

//----------------------------------------------------------
#pragma mark - IrStream
//----------------------------------------------------------
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

//----------------------------------------------------------
#pragma mark - Body
//----------------------------------------------------------
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

    size_t getNumJoints() const;
    const Joint &getJoint(size_t idx) const;

    const ofPoint &getJointPoint(size_t idx) const;
    const std::array<ofPoint, JointType_Count> &getJointPoints() const;

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

//----------------------------------------------------------
#pragma mark - BodyStream
//----------------------------------------------------------
class ofxKinect2::BodyStream : public Stream
{
public:
    bool setup(ofxKinect2::Device &device);
    bool open();
    void close();

    void update();
    bool updateMode();

    void draw(bool draw3D = false);
    void drawHands();
    void drawHandLeft();
    void drawHandRight();

    size_t getNumBodies();
    /**
     * @brief Index 0 is the left most player, 5 is the right-most player
     * @param bodyIndex
     * @return
     */
    const Body *getBodyWithIndex(BodyIndex bodyIndex);
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
