// @author sadmb
// @date 3,Jan,2014
// modified from ofxNI2.cpp of ofxNI2 by @satoruhiga

#include "ofxKinect2.h"
#include "utils\DepthRemapToRange.h"
#include <cmath>

namespace ofxKinect2
{
void init()
{
    static bool inited = false;
    if (inited) {
        return;
    }
    inited = true;
}
} // namespace ofxKinect2

using namespace ofxKinect2;

//----------------------------------------------------------
#pragma mark - Device
//----------------------------------------------------------

Device::Device()
    : m_Recorder(nullptr)
    , m_IsDepthColorSyncEnabled(false)
    , m_CoordinateMapper(nullptr)
{
    m_Device.kinect2 = nullptr;
}

Device::~Device()
{
    exit();
}

bool Device::setup()
{
    ofxKinect2::init();
    HRESULT hr = GetDefaultKinectSensor(&m_Device.kinect2);;
    if (SUCCEEDED(hr)) {
        m_Device.kinect2->Open();
        return true;
    }
    return false;
}

bool Device::setup(string kinect2FilePath)
{
    ofxKinect2::init();
    kinect2FilePath = ofToDataPath(kinect2FilePath);
    ofLogWarning("ofxKinect2::Device") << " Open from path is not supported yet.";
    return setup();
}

void Device::exit()
{
    if (m_Device.kinect2) {
        m_Device.kinect2->Close();
    }
    safeRelease(m_Device.kinect2);

    vector<Stream *>::iterator it;
    int counter = 0;
    while (!m_Streams.empty()) {
        it = m_Streams.begin();
        (*it)->exit();
        counter++;

        // just in case streams array mismatch .
        if (counter > 1000) {
            ofLogNotice("ofxKinect2::Device") << "streams array is incorrect.";
            break;
        }
    }
    m_Streams.clear();
    if (m_CoordinateMapper) {
        safeRelease(m_CoordinateMapper);
    }
}

void Device::update()
{
    if (!isOpen()) {
        return;
    }

    for (int i = 0; i < m_Streams.size(); i++) {
        Stream *stream = m_Streams[i];
        stream->m_IsFrameNew = stream->m_Kinect2Timestamp != stream->m_OpenGLTimestamp;
        stream->m_OpenGLTimestamp = stream->m_Kinect2Timestamp;
    }

    static ofEventArgs e;
    ofNotifyEvent(m_UpdateDeviceEvent, e, this);
}

bool Device::isOpen() const
{
    if (m_Device.kinect2 == nullptr) {
        return false;
    }

    bool open = false;
    m_Device.kinect2->get_IsOpen((BOOLEAN *)&open);
    return open;
}

void Device::setDepthColorSyncEnabled(bool enabled)
{
    m_IsDepthColorSyncEnabled = enabled;
    if (enabled) {
        HRESULT hr = m_Device.kinect2->get_CoordinateMapper(&m_CoordinateMapper);
        if (FAILED(hr)) {
            ofLogWarning("ofxKinect2::Device") << "Cannot start depth color sync";
        }
    }
    else {
        if (m_CoordinateMapper) {
            safeRelease(m_CoordinateMapper);
        }
    }
}

bool Device::isDepthColorSyncEnabled() const
{
    return m_IsDepthColorSyncEnabled;
}

DeviceHandle &Device::get()
{
    return m_Device;
}

const DeviceHandle &Device::get() const
{
    return m_Device;
}

ICoordinateMapper *Device::getMapper()
{
    return m_CoordinateMapper;
}

const ICoordinateMapper *Device::getMapper() const
{
    return m_CoordinateMapper;
}

//----------------------------------------------------------
#pragma mark - Stream
//----------------------------------------------------------

Stream::Stream()
{

}

Stream::~Stream()
{

}

bool Stream::open()
{
    startThread();
    return true;
}

void Stream::close()
{
    m_Frame.frameIndex = 0;
    m_Frame.stride = 0;
    m_Frame.data = nullptr;
    m_Frame.dataSize = 0;
    stopThread();
}

void Stream::exit()
{
    for (vector<Stream *>::iterator it = m_Device->m_Streams.begin(); it != m_Device->m_Streams.end();) {
        Stream *stream = *it;
        if (stream == this) {
            it = m_Device->m_Streams.erase(it);
            close();
        }
        else {
            ++it;
        }
    }
}

void Stream::update()
{
    m_IsTextureNeedUpdate = false;
}

bool Stream::updateMode()
{
    return true;
}

bool Stream::isOpen() const
{
    bool open = (m_StreamHandle.audioBeamFrameReader != nullptr) || (m_StreamHandle.bodyFrameReader != nullptr) ||
                (m_StreamHandle.bodyIndexFrameReader != nullptr) || (m_StreamHandle.colorFrameReader != nullptr) || (m_StreamHandle.depthFrameReader != nullptr)
                || (m_StreamHandle.infraredFrameReader != nullptr) || (m_StreamHandle.longExposureInfraredFrameReader != nullptr);
    return open;
}

bool Stream::setSize(int width, int height)
{
    m_Frame.mode.resolutionX = width;
    m_Frame.mode.resolutionY = height;

    return updateMode();
}

ofTexture &Stream::getTextureReference()
{
    return m_Texture;
}

int Stream::getWidth() const
{
    return m_Frame.width;
}

bool Stream::setWidth(int width)
{
    return setSize(width, getHeight());
}

int Stream::getHeight() const
{
    return m_Frame.height;
}

bool Stream::setHeight(int height)
{
    return setSize(getWidth(), height);
}

int Stream::getFps() const
{
    return m_Frame.mode.fps;
}

bool Stream::setFps(int fps)
{
    m_Frame.mode.fps = fps;
    return updateMode();
}

float Stream::getHorizontalFieldOfView() const
{
    return m_Frame.horizontalFieldOfView;
}

float Stream::getVerticalFieldOfView() const
{
    return m_Frame.verticalFieldOfView;
}

float Stream::getDiagonalFieldOfView() const
{
    return m_Frame.diagonalFieldOfView;
}

bool Stream::isFrameNew() const
{
    return m_IsFrameNew;
}

void Stream::setMirror(bool mirrored)
{
    m_IsMirror = false;
}

bool Stream::isMirror() const
{
    return m_IsMirror;
}

void Stream::draw(float x, float y)
{
    draw(x, y, getWidth(), getHeight());
}

void Stream::draw(float x, float y, float w, float h)
{
    if (m_IsTextureNeedUpdate) {
        update();
    }

    if (m_Texture.isAllocated()) {
        m_Texture.draw(x, y, w, h);
    }
}

StreamHandle &Stream::get()
{
    return m_StreamHandle;
}

const StreamHandle &Stream::get() const
{
    return m_StreamHandle;
}

CameraSettingsHandle &Stream::getCameraSettings()
{
    return m_CameraSettings;
}

const CameraSettingsHandle &Stream::getCameraSettings() const
{
    return m_CameraSettings;
}

bool Stream::setup(Device &device, SensorType sensorType)
{
    if (!device.isOpen()) {
        return false;
    }

    m_Kinect2Timestamp = 0;
    m_OpenGLTimestamp = 0;
    m_Frame.sensorType = sensorType;
    m_Frame.mode.fps = 0;
    m_Frame.frameIndex = 0;
    m_Frame.stride = 0;
    m_Frame.data = nullptr;
    m_Frame.dataSize = 0;
    m_IsFrameNew = false;
    m_IsTextureNeedUpdate = false;

    device.m_Streams.push_back(this);
    this->m_Device = &device;

    setMirror(false);
    return true;
}

void Stream::threadedFunction()
{
    while (isThreadRunning() != 0) {
        if (lock()) {
            if (readFrame()) {
                m_Kinect2Timestamp = m_Frame.timestamp;
                m_IsTextureNeedUpdate = true;
            }
            unlock();
        }
        if (m_Frame.mode.fps != 0) {
            ofSleepMillis(1000.f / m_Frame.mode.fps);
        }
    }
}

bool Stream::readFrame(IMultiSourceFrame *multiFrame)
{
    return false;
}

void Stream::setPixels(Frame &frame)
{
    m_Kinect2Timestamp = frame.timestamp;
}

//----------------------------------------------------------
#pragma mark - ColorStream
//----------------------------------------------------------

bool ColorStream::readFrame(IMultiSourceFrame *multiFrame)
{
    bool readed = false;
    if (!m_StreamHandle.colorFrameReader) {
        ofLogWarning("ofxKinect2::ColorStream") << "Stream is not open.";
        return readed;
    }
    Stream::readFrame(multiFrame);
    IColorFrame *colorFrame = nullptr;

    HRESULT hr = E_FAIL;
    if (!multiFrame) {
        hr = m_StreamHandle.colorFrameReader->AcquireLatestFrame(&colorFrame);
    }
    else {
        IColorFrameReference *colorFrameFeference = nullptr;
        hr = multiFrame->get_ColorFrameReference(&colorFrameFeference);
        if (SUCCEEDED(hr)) {
            hr = colorFrameFeference->AcquireFrame(&colorFrame);
        }
        safeRelease(colorFrameFeference);
    }

    if (SUCCEEDED(hr)) {
        IFrameDescription *colorFrameDescription = nullptr;
        ColorImageFormat imageFormat = ColorImageFormat_None;

        hr = colorFrame->get_RelativeTime((INT64 *)&m_Frame.timestamp);

        if (SUCCEEDED(hr)) {
            hr = colorFrame->get_FrameDescription(&colorFrameDescription);
        }

        if (SUCCEEDED(hr)) {
            hr = colorFrameDescription->get_Width(&m_Frame.width);
        }

        if (SUCCEEDED(hr)) {
            hr = colorFrameDescription->get_Height(&m_Frame.height);
        }

        if (SUCCEEDED(hr)) {
            hr = colorFrameDescription->get_HorizontalFieldOfView(&m_Frame.horizontalFieldOfView);
        }
        if (SUCCEEDED(hr)) {
            hr = colorFrameDescription->get_VerticalFieldOfView(&m_Frame.verticalFieldOfView);
        }

        if (SUCCEEDED(hr)) {
            hr = colorFrameDescription->get_DiagonalFieldOfView(&m_Frame.diagonalFieldOfView);
        }

        if (SUCCEEDED(hr)) {
            hr = colorFrame->get_RawColorImageFormat(&imageFormat);
        }

        if (SUCCEEDED(hr)) {
            if (m_Buffer == nullptr) {
                m_Buffer = new unsigned char[m_Frame.width * m_Frame.height * 4];
            }
            if (imageFormat == ColorImageFormat_Rgba) {
                hr = colorFrame->AccessRawUnderlyingBuffer((UINT *)&m_Frame.dataSize, reinterpret_cast<BYTE **>(&m_Frame.data));
            }
            else {
                m_Frame.data = m_Buffer;
                m_Frame.dataSize = m_Frame.width * m_Frame.height * 4 * sizeof(unsigned char);
                hr = colorFrame->CopyConvertedFrameDataToArray((UINT)m_Frame.dataSize, reinterpret_cast<BYTE *>(m_Frame.data),  ColorImageFormat_Rgba);
            }
        }

        if (SUCCEEDED(hr)) {
            readed = true;
            setPixels(m_Frame);
        }
        safeRelease(colorFrameDescription);
    }

    safeRelease(colorFrame);

    return readed;
}

void ColorStream::setPixels(Frame &frame)
{
    Stream::setPixels(frame);
    const unsigned char *src = (const unsigned char *)frame.data;

    m_DoubleBuffer.getBackBuffer().setFromPixels(src, frame.width, frame.height, OF_IMAGE_COLOR_ALPHA);
    m_DoubleBuffer.swap();
}

bool ColorStream::setup(ofxKinect2::Device &device)
{
    m_Buffer = nullptr;
    return Stream::setup(device, SENSOR_COLOR);
}

void ColorStream::exit()
{
    Stream::exit();
    if (m_Buffer) {
        delete[] m_Buffer;
        m_Buffer = nullptr;
    }
}

bool ColorStream::open()
{
    if (!m_Device->isOpen()) {
        ofLogWarning("ofxKinect2::ColorStream") << "No ready Kinect2 found.";
        return false;
    }

    IColorFrameSource *colorFrameSource = nullptr;
    HRESULT hr = m_Device->get().kinect2->get_ColorFrameSource(&colorFrameSource);

    if (SUCCEEDED(hr)) {
        hr = colorFrameSource->OpenReader(&m_StreamHandle.colorFrameReader);
    }

    IFrameDescription *colorFrameDescription = nullptr;
    colorFrameSource->get_FrameDescription(&colorFrameDescription);

    if (SUCCEEDED(hr)) {
        int resolutionX, resolutionY = 0;
        hr = colorFrameDescription->get_Width(&resolutionX);
        hr = colorFrameDescription->get_Width(&resolutionY);
        m_Frame.mode.resolutionX = resolutionX;
        m_Frame.mode.resolutionY = resolutionY;
        m_Frame.width = resolutionX;
        m_Frame.height = resolutionY;
        m_DoubleBuffer.allocate(resolutionX, resolutionY, 4);

    }

    safeRelease(colorFrameDescription);
    safeRelease(colorFrameSource);
    if (FAILED(hr)) {
        ofLogWarning("ofxKinect2::ColorStream") << "Can't open stream.";
        return false;
    }

    return Stream::open();
}

void ColorStream::close()
{
    while (!lock()) {
        printf("ColorStream waiting to close\n");
    }
    unlock();

    Stream::close();
    safeRelease(m_StreamHandle.colorFrameReader);
}

void ColorStream::update()
{
    if (!m_Texture.isAllocated() || m_Texture.getWidth() != getWidth() || m_Texture.getHeight() != getHeight()) {
        m_Texture.allocate(getWidth(), getHeight(), GL_RGB);
    }

    if (lock()) {
        m_Texture.loadData(m_DoubleBuffer.getFrontBuffer());
        Stream::update();
        unlock();
    }
}

bool ColorStream::updateMode()
{
    ofLogWarning("ofxKinect2::ColorStream") << "Not supported yet.";
    return false;
}

bool ColorStream::setWidth(int width)
{
    bool ret = Stream::setWidth(width);
    m_DoubleBuffer.deallocate();
    m_DoubleBuffer.allocate(m_Frame.width, m_Frame.height, 4);
    return ret;
}

bool ColorStream::setHeight(int height)
{
    bool ret = Stream::setHeight(height);
    m_DoubleBuffer.allocate(m_Frame.width, m_Frame.height, 4);
    return ret;
}

bool ColorStream::setSize(int width, int height)
{
    bool ret = Stream::setSize(width, height);
    m_DoubleBuffer.allocate(m_Frame.width, m_Frame.height, 4);
    return ret;
}

ofPixels &ColorStream::getPixelsRef()
{
    return m_DoubleBuffer.getFrontBuffer();
}

int ColorStream::getExposureTime() const
{
    TIMESPAN exposureTime = 0;
    m_CameraSettings.colorCameraSettings->get_ExposureTime(&exposureTime);
    return (int)exposureTime;
}

int ColorStream::getFrameInterval() const
{
    TIMESPAN frameInterval = 0;
    m_CameraSettings.colorCameraSettings->get_FrameInterval(&frameInterval);
    return (int)frameInterval;
}

float ColorStream::getGain() const
{
    float gain = 0;
    m_CameraSettings.colorCameraSettings->get_Gain(&gain);
    return gain;
}

float ColorStream::getGamma() const
{
    float gamma = 0;
    m_CameraSettings.colorCameraSettings->get_Gamma(&gamma);
    return gamma;
}

//----------------------------------------------------------
#pragma mark - DepthStream
//----------------------------------------------------------

bool DepthStream::readFrame(IMultiSourceFrame *multiFrame)
{
    bool readed = false;
    if (!m_StreamHandle.depthFrameReader) {
        ofLogWarning("ofxKinect2::DepthStream") << "Stream is not open.";
        return readed;
    }

    Stream::readFrame(multiFrame);
    IDepthFrame *depthFrame = nullptr;

    HRESULT hr = E_FAIL;
    if (!multiFrame) {
        hr = m_StreamHandle.depthFrameReader->AcquireLatestFrame(&depthFrame);
    }
    else {
        IDepthFrameReference *depthFrameReference = nullptr;
        hr = multiFrame->get_DepthFrameReference(&depthFrameReference);

        if (SUCCEEDED(hr)) {
            hr = depthFrameReference->AcquireFrame(&depthFrame);
        }

        safeRelease(depthFrameReference);
    }

    if (SUCCEEDED(hr)) {
        IFrameDescription *depthFrameDescription = nullptr;

        hr = depthFrame->get_RelativeTime((INT64 *)&m_Frame.timestamp);

        if (SUCCEEDED(hr)) {
            hr = depthFrame->get_FrameDescription(&depthFrameDescription);
        }

        if (SUCCEEDED(hr)) {
            hr = depthFrameDescription->get_Width(&m_Frame.width);
        }

        if (SUCCEEDED(hr)) {
            hr = depthFrameDescription->get_Height(&m_Frame.height);
        }

        if (SUCCEEDED(hr)) {
            hr = depthFrameDescription->get_HorizontalFieldOfView(&m_Frame.horizontalFieldOfView);
        }

        if (SUCCEEDED(hr)) {
            hr = depthFrameDescription->get_VerticalFieldOfView(&m_Frame.verticalFieldOfView);
        }

        if (SUCCEEDED(hr)) {
            hr = depthFrameDescription->get_DiagonalFieldOfView(&m_Frame.diagonalFieldOfView);
        }

        if (SUCCEEDED(hr)) {
            hr = depthFrame->get_DepthMinReliableDistance((USHORT *)&m_NearValue);
        }

        if (SUCCEEDED(hr)) {
            hr = depthFrame->get_DepthMaxReliableDistance((USHORT *)&m_FarValue);
        }

        if (SUCCEEDED(hr)) {
            hr = depthFrame->get_DepthMinReliableDistance((USHORT *)&m_NearValue);
        }

        if (SUCCEEDED(hr)) {
            if (m_Frame.dataSize == 0) {
                m_Frame.dataSize = m_Frame.width * m_Frame.height;
                m_Frame.data = new UINT16[m_Frame.width * m_Frame.height];
            }
            hr = depthFrame->CopyFrameDataToArray(m_Frame.width * m_Frame.height, reinterpret_cast<UINT16 *>(m_Frame.data));
        }

        if (SUCCEEDED(hr)) {
            readed = true;
            setPixels(m_Frame);
        }
        safeRelease(depthFrameDescription);
    }

    safeRelease(depthFrame);

    return readed;
}

void DepthStream::setPixels(Frame &frame)
{
    Stream::setPixels(frame);
    const unsigned short *pixels = (const unsigned short *)frame.data;

    m_DoubleBuffer.allocate(frame.width, frame.height, 1);
    m_DoubleBuffer.getBackBuffer().setFromPixels(pixels, frame.width, frame.height, OF_IMAGE_GRAYSCALE);
    m_DoubleBuffer.swap();
}

bool DepthStream::setup(ofxKinect2::Device &device)
{
    m_NearValue = 50;
    m_FarValue = 10000;
    return Stream::setup(device, SENSOR_DEPTH);
}

bool DepthStream::open()
{
    if (!m_Device->isOpen()) {
        ofLogWarning("ofxKinect2::DepthStream") << "No ready Kinect2 found.";
        return false;
    }

    m_IsInvert = true;
    m_NearValue = 0;
    m_FarValue = 10000;
    IDepthFrameSource *depthFrameSource = nullptr;
    HRESULT hr = m_Device->get().kinect2->get_DepthFrameSource(&depthFrameSource);

    if (SUCCEEDED(hr)) {
        hr = depthFrameSource->OpenReader(&m_StreamHandle.depthFrameReader);
    }

    safeRelease(depthFrameSource);
    if (FAILED(hr)) {
        ofLogWarning("ofxKinect2::DepthStream") << "Can't open stream.";
        return false;
    }

    return Stream::open();
}

void DepthStream::close()
{
    while (!lock()) {
        printf("DepthStream waiting to close\n");
    }
    unlock();

    delete m_Frame.data;
    m_Frame.data = nullptr;

    Stream::close();
    safeRelease(m_StreamHandle.depthFrameReader);
}

void DepthStream::update()
{
    if (!m_IsTextureNeedUpdate) {
        return;
    }

    if (!m_Texture.isAllocated() || m_Texture.getWidth() != getWidth() || m_Texture.getHeight() != getHeight()) {
#if OF_VERSION_MINOR <= 7
        static ofTextureData data;
        data.pixelType = GL_UNSIGNED_SHORT;
        data.glTypeInternal = GL_LUMINANCE16;
        data.width = getWidth();
        data.height = getHeight();

        m_Texture.allocate(data);
#elif OF_VERSION_MINOR > 7
        m_Texture.allocate(getWidth(), getHeight(), GL_RGBA, true, GL_LUMINANCE, GL_UNSIGNED_SHORT);
#endif
    }

    if (lock()) {
        // Update the image information
        ofShortPixels pixels;
        depthRemapToRange(m_DoubleBuffer.getFrontBuffer(), pixels, m_NearValue, m_FarValue, m_IsInvert);
        m_Texture.loadData(pixels);
        Stream::update();
        unlock();
    }
}

bool DepthStream::updateMode()
{
    ofLogWarning("ofxKinect2::DepthStream") << "Not supported yet.";
    return false;
}

void DepthStream::getColorSpacePoints(ColorSpacePoint *colorSpacePointsFromDepth)
{
    if (lock()) {
        // Calculate the body's position on the screen
        const UINT depthPointCount = DEPTH_WIDTH * DEPTH_HEIGHT;
        ICoordinateMapper *coordinateMapper = nullptr;
        HRESULT hr = m_Device->get().kinect2->get_CoordinateMapper(&coordinateMapper);

        if (SUCCEEDED(hr)) {
            coordinateMapper->MapDepthFrameToColorSpace(depthPointCount, (UINT16 *)m_Frame.data, depthPointCount, colorSpacePointsFromDepth);
        }

        safeRelease(coordinateMapper);
        unlock();
    }
}

int DepthStream::getNumberColorSpacePoints() const
{
    if (m_Frame.data != nullptr) {
        return m_Frame.width * m_Frame.height;
    }

    return 0;
}

void DepthStream::getCameraSpacePoints(CameraSpacePoint *cameraSpacePointsFromDepth)
{
    if (lock()) {
        // Calculate the body's position on the screen
        const UINT depthPointCount = DEPTH_WIDTH * DEPTH_HEIGHT;

        ICoordinateMapper *m_pCoordinateMapper = nullptr;
        HRESULT hr = m_Device->get().kinect2->get_CoordinateMapper(&m_pCoordinateMapper);

        if (SUCCEEDED(hr)) {
            m_pCoordinateMapper->MapDepthFrameToCameraSpace(depthPointCount, (UINT16 *)m_Frame.data, depthPointCount, cameraSpacePointsFromDepth);
        }

        safeRelease(m_pCoordinateMapper);
        unlock();
    }
}

int DepthStream::getNumberCameraSpacePoints() const
{
    if (m_Frame.data != nullptr) {
        return m_Frame.width * m_Frame.height;
    }

    return 0;
}

ofShortPixels &DepthStream::getPixelsRef()
{
    return m_DoubleBuffer.getFrontBuffer();
}

ofShortPixels DepthStream::getPixelsRef(int nearValue, int farValue, bool invert)
{
    ofShortPixels pixels;
    depthRemapToRange(getPixelsRef(), pixels, nearValue, farValue, invert);
    return pixels;
}

void DepthStream::setNear(float nearValue)
{
    m_NearValue = nearValue;
}

float DepthStream::getNear() const
{
    return m_NearValue;
}

void DepthStream::setFar(float farValue)
{
    m_FarValue = farValue;
}

float DepthStream::getFar() const
{
    return m_FarValue;
}

void DepthStream::setInvert(float invert)
{
    m_IsInvert = invert;
}

bool DepthStream::getInvert() const
{
    return m_IsInvert;
}

//----------------------------------------------------------
#pragma mark - BodyIndexStream
//----------------------------------------------------------

bool BodyIndexStream::readFrame(IMultiSourceFrame *multiFrame)
{
    bool isSuccess = false;
    if (!m_StreamHandle.depthFrameReader) {
        ofLogWarning("ofxKinect2::BodyIndexStream") << "Stream is not open.";
        return isSuccess;
    }

    Stream::readFrame(multiFrame);
    IBodyIndexFrame *bodyIndexFrame = nullptr;

    HRESULT hr = E_FAIL;
    if (!multiFrame) {
        hr = m_StreamHandle.bodyIndexFrameReader->AcquireLatestFrame(&bodyIndexFrame);
    }
    else {
        IBodyIndexFrameReference *frameReference = nullptr;
        hr = multiFrame->get_BodyIndexFrameReference(&frameReference);

        if (SUCCEEDED(hr)) {
            hr = frameReference->AcquireFrame(&bodyIndexFrame);
        }

        safeRelease(frameReference);
    }

    if (SUCCEEDED(hr)) {
        IFrameDescription *frameDescription = nullptr;

        hr = bodyIndexFrame->get_RelativeTime((INT64 *)&m_Frame.timestamp);
        if (SUCCEEDED(hr)) {
            hr = bodyIndexFrame->get_FrameDescription(&frameDescription);
        }

        if (SUCCEEDED(hr)) {
            hr = frameDescription->get_Width(&m_Frame.width);
        }

        if (SUCCEEDED(hr)) {
            hr = frameDescription->get_Height(&m_Frame.height);
        }

        if (SUCCEEDED(hr)) {
            hr = frameDescription->get_HorizontalFieldOfView(&m_Frame.horizontalFieldOfView);
        }

        if (SUCCEEDED(hr)) {
            hr = frameDescription->get_VerticalFieldOfView(&m_Frame.verticalFieldOfView);
        }

        if (SUCCEEDED(hr)) {
            hr = frameDescription->get_DiagonalFieldOfView(&m_Frame.diagonalFieldOfView);
        }

        if (SUCCEEDED(hr)) {
            hr = bodyIndexFrame->AccessUnderlyingBuffer((UINT *)&m_Frame.dataSize, reinterpret_cast<BYTE **>(&m_Frame.data));
        }

        if (SUCCEEDED(hr)) {
            isSuccess = true;
            setPixels(m_Frame);
        }
        safeRelease(frameDescription);
    }

    safeRelease(bodyIndexFrame);

    return isSuccess;
}

void BodyIndexStream::setPixels(Frame &frame)
{
    Stream::setPixels(frame);

    const BYTE *pixels = reinterpret_cast<const BYTE *>(frame.data);
    ofShortPixels pxs;
    pxs.allocate(frame.width, frame.height, OF_IMAGE_GRAYSCALE);
    for (int i = 0; i < frame.dataSize; i++) {
        if (pixels[i] < 6) {
            //This pixel belongs to a player
            pxs.setColor(i, ofColor::black);
        }
        else {
            pxs.setColor(i, ofColor::white);
        }
    }

    m_DoubleBuffer.getBackBuffer().setFromPixels(pxs.getPixels(), frame.width, frame.height, OF_IMAGE_GRAYSCALE);
    m_DoubleBuffer.swap();
}

bool BodyIndexStream::setup(ofxKinect2::Device &device)
{
    return Stream::setup(device, SensorType::SENSOR_BODY_INDEX);
}

const ofShortPixels &BodyIndexStream::getPixelsRef() const
{
    return m_DoubleBuffer.getFrontBuffer();
}

void BodyIndexStream::setInvert(float invert)
{
    m_IsInvert = invert;
}

bool BodyIndexStream::getInvert() const
{
    return m_IsInvert;
}

bool BodyIndexStream::open()
{
    if (!m_Device->isOpen()) {
        ofLogWarning("ofxKinect2::BodyIndexStream") << "No ready Kinect2 found.";
        return false;
    }

    m_IsInvert = true;
    IBodyIndexFrameSource *frameSource = nullptr;
    HRESULT hr = E_FAIL;

    hr = m_Device->get().kinect2->get_BodyIndexFrameSource(&frameSource);
    if (SUCCEEDED(hr)) {
        hr = frameSource->OpenReader(&m_StreamHandle.bodyIndexFrameReader);

        if (SUCCEEDED(hr)) {
            IFrameDescription *frameDescription = nullptr;
            frameSource->get_FrameDescription(&frameDescription);
            if (SUCCEEDED(hr)) {
                int resX, resY = 0;
                hr = frameDescription->get_Width(&resX);
                hr = frameDescription->get_Width(&resY);
                m_Frame.mode.resolutionX = resX;
                m_Frame.mode.resolutionY = resY;
                m_Frame.width = resX;
                m_Frame.height = resY;
                m_DoubleBuffer.allocate(resX, resY, 4);

            }
            safeRelease(frameDescription);
        }
    }

    safeRelease(frameSource);
    if (FAILED(hr)) {
        ofLogWarning("ofxKinect2::BodyIndexStream") << "Can't open stream.";
        return false;
    }

    return Stream::open();
}

void BodyIndexStream::close()
{
    while (!lock()) {
        printf("DepthStream waiting to close\n");
    }
    unlock();

    safeRelease(m_StreamHandle.bodyIndexFrameReader);
    Stream::close();
}

void BodyIndexStream::update()
{
    if (!m_IsTextureNeedUpdate) {
        return;
    }

    if (!m_Texture.isAllocated() || m_Texture.getWidth() != getWidth() || m_Texture.getHeight() != getHeight()) {
#if OF_VERSION_MINOR <= 7
        static ofTextureData data;

        data.pixelType = GL_UNSIGNED_SHORT;
        data.glTypeInternal = GL_LUMINANCE16;
        data.width = getWidth();
        data.height = getHeight();

        m_Texture.allocate(data);
#elif OF_VERSION_MINOR > 7
        m_Texture.allocate(getWidth(), getHeight(), GL_RGBA, true, GL_LUMINANCE, GL_UNSIGNED_SHORT);
#endif
    }

    if (lock()) {
        m_Texture.loadData(m_DoubleBuffer.getFrontBuffer());
        unlock();
    }
    Stream::update();
}

bool BodyIndexStream::updateMode()
{
    ofLogWarning("ofxKinect2::BodyIndexStream") << "Not supported yet.";
    return false;
}

//----------------------------------------------------------
#pragma mark - IrStream
//----------------------------------------------------------

bool IrStream::readFrame(IMultiSourceFrame *multiFrame)
{
    bool readed = false;
    if (!m_StreamHandle.infraredFrameReader) {
        ofLogWarning("ofxKinect2::IrStream") << "Stream is not open.";
        return readed;
    }
    Stream::readFrame();

    IInfraredFrame *irFrame = nullptr;

    HRESULT hr = E_FAIL;
    if (!multiFrame) {
        hr = m_StreamHandle.infraredFrameReader->AcquireLatestFrame(&irFrame);
    }
    else {
        IInfraredFrameReference *irFrameReference = nullptr;
        hr = multiFrame->get_InfraredFrameReference(&irFrameReference);

        if (SUCCEEDED(hr)) {
            hr = irFrameReference->AcquireFrame(&irFrame);
        }

        safeRelease(irFrameReference);
    }

    if (SUCCEEDED(hr)) {
        IFrameDescription *irFrameDescription = nullptr;

        hr = irFrame->get_RelativeTime((INT64 *)&m_Frame.timestamp);

        if (SUCCEEDED(hr)) {
            hr = irFrame->get_FrameDescription(&irFrameDescription);
        }

        if (SUCCEEDED(hr)) {
            hr = irFrameDescription->get_Width(&m_Frame.width);
        }

        if (SUCCEEDED(hr)) {
            hr = irFrameDescription->get_Height(&m_Frame.height);
        }

        if (SUCCEEDED(hr)) {
            hr = irFrameDescription->get_HorizontalFieldOfView(&m_Frame.horizontalFieldOfView);
        }

        if (SUCCEEDED(hr)) {
            hr = irFrameDescription->get_VerticalFieldOfView(&m_Frame.verticalFieldOfView);
        }

        if (SUCCEEDED(hr)) {
            hr = irFrameDescription->get_DiagonalFieldOfView(&m_Frame.diagonalFieldOfView);
        }

        if (SUCCEEDED(hr)) {
            hr = irFrame->AccessUnderlyingBuffer((UINT *)&m_Frame.dataSize, reinterpret_cast<UINT16 **>(&m_Frame.data));
        }

        if (SUCCEEDED(hr)) {
            readed = true;
            setPixels(m_Frame);
        }
        safeRelease(irFrameDescription);
    }

    safeRelease(irFrame);

    return readed;
}

void IrStream::setPixels(Frame &frame)
{
    Stream::setPixels(frame);
    const unsigned short *pixels = (const unsigned short *)frame.data;

    m_DoubleBuffer.allocate(frame.width, frame.height, 1);
    m_DoubleBuffer.getBackBuffer().setFromPixels(pixels, frame.width, frame.height, OF_IMAGE_GRAYSCALE);
    m_DoubleBuffer.swap();
}

bool IrStream::setup(ofxKinect2::Device &device)
{
    return Stream::setup(device, SENSOR_IR);
}

bool IrStream::open()
{
    if (!m_Device->isOpen()) {
        ofLogWarning("ofxKinect2::IrStream") << "No ready Kinect2 found.";
        return false;
    }

    IInfraredFrameSource *irFrameSource = nullptr;
    HRESULT hr = E_FAIL;

    hr = m_Device->get().kinect2->get_InfraredFrameSource(&irFrameSource);

    if (SUCCEEDED(hr)) {
        hr = irFrameSource->OpenReader(&m_StreamHandle.infraredFrameReader);
    }

    safeRelease(irFrameSource);
    if (FAILED(hr)) {
        ofLogWarning("ofxKinect2::IrStream") << "Can't open stream.";
        return false;
    }

    return Stream::open();
}

void IrStream::close()
{
    while (!lock()) {
        printf("IrStream waiting to close\n");
    }
    unlock();

    Stream::close();
    safeRelease(m_StreamHandle.infraredFrameReader);
}

void IrStream::update()
{
    if (!m_Texture.isAllocated() || m_Texture.getWidth() != getWidth() || m_Texture.getHeight() != getHeight()) {
        m_Texture.allocate(getWidth(), getHeight(), GL_LUMINANCE);
    }

    if (lock()) {
        m_Texture.loadData(m_DoubleBuffer.getFrontBuffer());
        Stream::update();
        unlock();
    }
}

bool IrStream::updateMode()
{
    ofLogWarning("ofxKinect2::IrStream") << "Not supported yet.";
    return false;
}

ofShortPixels &IrStream::getPixelsRef()
{
    return m_DoubleBuffer.getFrontBuffer();
}

//----------------------------------------------------------
#pragma mark - Body
//----------------------------------------------------------

Body::Body()
    : m_IsInitialized(false)
{
    std::fill(m_JointPoints.begin(), m_JointPoints.end(), ofPoint::zero());
}

void Body::setup(ofxKinect2::Device &device, IBody *body)
{
    this->m_Device = &device;

    body->get_HandLeftState(&m_LeftHandState);
    body->get_HandRightState(&m_RightHandState);

    // TODO: Correct tracking ID management
    body->get_TrackingId(&m_TrackingID);

    body->GetJoints(JointType_Count, m_Joints);

    m_IsInitialized = true;
}


void Body::close()
{

}

void Body::update()
{
    // Update the 2D joints from the 3D skeleton
    for (int jointIndex = 0; jointIndex < JointType_Count; ++jointIndex) {
        m_JointPoints[jointIndex] = bodyToScreen(m_Joints[jointIndex].Position, ofGetWidth(), ofGetHeight());
    }
}

void Body::drawBody(bool draw3D)
{
    drawBone(JointType_Head, JointType_Neck, draw3D);
    drawBone(JointType_Neck, JointType_SpineShoulder, draw3D);
    drawBone(JointType_SpineShoulder, JointType_SpineMid, draw3D);
    drawBone(JointType_SpineMid, JointType_SpineBase, draw3D);
    drawBone(JointType_SpineShoulder, JointType_ShoulderLeft, draw3D);
    drawBone(JointType_SpineShoulder, JointType_ShoulderRight, draw3D);
    drawBone(JointType_SpineBase, JointType_HipLeft, draw3D);
    drawBone(JointType_SpineBase, JointType_HipRight, draw3D);

    drawBone(JointType_ShoulderLeft, JointType_ElbowLeft, draw3D);
    drawBone(JointType_ElbowLeft, JointType_WristLeft, draw3D);
    drawBone(JointType_WristLeft, JointType_HandLeft, draw3D);
    drawBone(JointType_HandLeft, JointType_HandTipLeft, draw3D);
    drawBone(JointType_WristLeft, JointType_ThumbLeft, draw3D);

    drawBone(JointType_ShoulderRight, JointType_ElbowRight, draw3D);
    drawBone(JointType_ElbowRight, JointType_WristRight, draw3D);
    drawBone(JointType_WristRight, JointType_HandRight, draw3D);
    drawBone(JointType_HandRight, JointType_HandTipRight, draw3D);
    drawBone(JointType_WristRight, JointType_ThumbRight, draw3D);

    drawBone(JointType_HipLeft, JointType_KneeLeft, draw3D);
    drawBone(JointType_KneeLeft, JointType_AnkleLeft, draw3D);
    drawBone(JointType_AnkleLeft, JointType_FootLeft, draw3D);

    drawBone(JointType_HipRight, JointType_KneeRight, draw3D);
    drawBone(JointType_KneeRight, JointType_AnkleRight, draw3D);
    drawBone(JointType_AnkleRight, JointType_FootRight, draw3D);

    ofPushStyle();
    for (int i = 0; i < JointType_Count; ++i) {
        if (m_Joints[i].TrackingState == TrackingState_Inferred) {
            ofSetColor(ofColor::yellow);
            if (draw3D) {
                ofSphere(m_Joints[i].Position.X, m_Joints[i].Position.Y, m_Joints[i].Position.Z, 0.01);
            }
            else {
                ofEllipse(m_JointPoints[i], 3, 3);
            }
        }
        else if (m_Joints[i].TrackingState == TrackingState_Tracked) {
            ofSetColor(50, 200, 50);
            if (draw3D) {
                ofSphere(m_Joints[i].Position.X, m_Joints[i].Position.Y, m_Joints[i].Position.Z, 0.01);
            }
            else {
                ofEllipse(m_JointPoints[i], 3, 3);
            }
        }
    }
    ofPopStyle();
}

void Body::drawBone(JointType joint0, JointType joint1, bool draw3D)
{
    ofPushStyle();
    TrackingState state0 = m_Joints[joint0].TrackingState;
    TrackingState state1 = m_Joints[joint1].TrackingState;

    if ((state0 == TrackingState_NotTracked) || (state1 == TrackingState_NotTracked)) {
        return;
    }

    if ((state0 == TrackingState_Inferred) && (state1 == TrackingState_Inferred)) {
        return;
    }

    if ((state0 == TrackingState_Tracked) && (state1 == TrackingState_Tracked)) {
        ofSetColor(ofColor::green);
    }
    else {
        ofSetColor(ofColor::gray);
    }
    if (draw3D) {
        ofLine(m_Joints[joint0].Position.X, m_Joints[joint0].Position.Y, m_Joints[joint0].Position.Z,
               m_Joints[joint1].Position.X, m_Joints[joint1].Position.Y, m_Joints[joint1].Position.Z);
    }
    else {
        ofLine(m_JointPoints[joint0], m_JointPoints[joint1]);
    }
    ofPopStyle();
}

void Body::drawHands(bool draw3D)
{
    drawHandLeft(draw3D);
    drawHandRight(draw3D);
}

void Body::drawHand(JointType handType, bool draw3D)
{
    if (handType == JointType_HandLeft || handType == JointType_HandRight) {
        ofPushStyle();
        switch (m_LeftHandState) {
        case HandState_Closed:
            ofSetColor(ofColor::red);
            if (draw3D) {
                ofSphere(m_Joints[handType].Position.X, m_Joints[handType].Position.Y, m_Joints[handType].Position.Z, 0.01);
            }
            else {
                ofEllipse(m_JointPoints[handType], 30, 30);
            }
            break;
        case HandState_Open:
            ofSetColor(ofColor::green);
            if (draw3D) {
                ofSphere(m_Joints[handType].Position.X, m_Joints[handType].Position.Y, m_Joints[handType].Position.Z, 0.01);
            }
            else {
                ofEllipse(m_JointPoints[handType], 30, 30);
            }
            break;
        case HandState_Lasso:
            ofSetColor(ofColor::blue);
            if (draw3D) {
                ofSphere(m_Joints[handType].Position.X, m_Joints[handType].Position.Y, m_Joints[handType].Position.Z, 0.01);
            }
            else {
                ofEllipse(m_JointPoints[handType], 30, 30);
            }
            break;
        }
        ofPopStyle();
    }
}

void Body::drawHandLeft(bool draw3D)
{
    drawHand(JointType_HandLeft, draw3D);
}

void Body::drawHandRight(bool draw3D)
{
    drawHand(JointType_HandRight, draw3D);
}

UINT64 Body::getId() const
{
    // TODO: Correct tracking ID management
    return m_TrackingID;
}

HandState Body::getLeftHandState() const
{
    return m_LeftHandState;
}

HandState Body::getRightHandState() const
{
    return m_RightHandState;
}

size_t Body::getNumJoints()
{
    return JointType_Count;
}

const Joint &Body::getJoint(size_t idx)
{
    return m_Joints[idx];
}

const ofPoint &Body::getJointPoint(size_t idx)
{
    return m_JointPoints[idx];
}

const std::array<ofPoint, JointType_Count> &Body::getJointPoints()
{
    return m_JointPoints;
}

bool Body::IsInitialized()
{
    return m_IsInitialized;
}

ofPoint Body::bodyToScreen(const CameraSpacePoint &bodyPoint, int width, int height)
{
    // Calculate the body's position on the screen
    ColorSpacePoint rgbPoint = {0, 0};
    ICoordinateMapper *m_pCoordinateMapper = nullptr;
    HRESULT hr = m_Device->get().kinect2->get_CoordinateMapper(&m_pCoordinateMapper);

    if (SUCCEEDED(hr)) {
        m_pCoordinateMapper->MapCameraPointToColorSpace(bodyPoint, &rgbPoint);
    }

    safeRelease(m_pCoordinateMapper);
    return ofPoint(rgbPoint.X, rgbPoint.Y);
}

//----------------------------------------------------------
#pragma mark - BodyStream
//----------------------------------------------------------

bool BodyStream::readFrame(IMultiSourceFrame *multiFrame)
{
    bool readed = false;
    if (!m_StreamHandle.bodyFrameReader) {
        ofLogWarning("ofxKinect2::BodyStream") << "Stream is not open.";
        return readed;
    }

    IBodyFrame *bodyFrame = nullptr;

    HRESULT hr = E_FAIL;
    if (!multiFrame) {
        hr = m_StreamHandle.bodyFrameReader->AcquireLatestFrame(&bodyFrame);
    }
    else {
        IBodyFrameReference *bodyFrameReference = nullptr;
        hr = multiFrame->get_BodyFrameReference(&bodyFrameReference);

        if (SUCCEEDED(hr)) {
            hr = bodyFrameReference->AcquireFrame(&bodyFrame);
        }

        safeRelease(bodyFrameReference);
    }

    if (SUCCEEDED(hr)) {
        hr = bodyFrame->get_RelativeTime((INT64 *)&m_Frame.timestamp);

        IBody *ppBodies[BODY_COUNT] = {0};

        if (SUCCEEDED(hr)) {
            hr = bodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
        }

        if (lock()) {
            if (SUCCEEDED(hr)) {
                readed = true;

                // Clears the body list
                for (int b = 0 ; b < m_Bodies.size() ; b++) {
                    delete m_Bodies[b];
                }
                m_Bodies.clear();

                for (int i = 0; i < _countof(ppBodies); ++i) {
                    BOOLEAN isTracked = false;
                    if (ppBodies[i]) {
                        ppBodies[i]->get_IsTracked(&isTracked);
                        if (isTracked) {
                            UINT64 id = -1;
                            ppBodies[i]->get_TrackingId(&id);

                            // Add the tracked body to the list
                            Body *body = new Body();
                            body->setup(*m_Device, ppBodies[i]);
                            body->update();
                            m_Bodies.push_back(body);
                            // TODO: Use the body tracked id to re-use the Body objects
                            // TODO: Clarify the relationship between Body and IBody.
                        }
                    }
                }
            }

            unlock();
        }

        for (int i = 0; i < _countof(ppBodies); ++i) {
            safeRelease(ppBodies[i]);
        }
    }

    safeRelease(bodyFrame);

    return readed;
}

void BodyStream::setPixels(Frame &frame)
{
    Stream::setPixels(frame);
}

bool BodyStream::setup(ofxKinect2::Device &device)
{
    return Stream::setup(device, SENSOR_BODY);
}

bool BodyStream::open()
{
    if (!m_Device->isOpen()) {
        ofLogWarning("ofxKinect2::BodyStream") << "No ready Kinect2 found.";
        return false;
    }

    IBodyFrameSource *bodyFrameSource = nullptr;
    HRESULT hr = m_Device->get().kinect2->get_BodyFrameSource(&bodyFrameSource);

    if (SUCCEEDED(hr)) {
        hr = bodyFrameSource->OpenReader(&m_StreamHandle.bodyFrameReader);
    }

    safeRelease(bodyFrameSource);
    if (FAILED(hr)) {
        ofLogWarning("ofxKinect2::BodyStream") << "Can't open stream.";
        return false;
    }

    return Stream::open();
}

void BodyStream::close()
{
    while (!lock()) {
        printf("BodyStream waiting to close\n");
    }
    unlock();

    Stream::close();
    safeRelease(m_StreamHandle.bodyFrameReader);
    for (int i = 0; i < m_Bodies.size(); i++) {
        m_Bodies[i]->close();
    }
}

void BodyStream::update()
{
    if (lock()) {
        for (int i = 0; i < m_Bodies.size(); i++) {
            m_Bodies[i]->update();
        }
        Stream::update();
        unlock();
    }
}

bool BodyStream::updateMode()
{
    ofLogWarning("ofxKinect2::BodyStream") << "Not supported yet.";
    return false;
}

void BodyStream::draw(bool draw3D)
{
    if (lock()) {
        for (int i = 0; i < m_Bodies.size(); i++) {
            if (m_Bodies[i]->getJointPoints().size() > 0) {
                m_Bodies[i]->drawBody(draw3D);
                m_Bodies[i]->drawHands(draw3D);
            }
        }
        unlock();
    }
}


void BodyStream::drawHands()
{
    if (lock()) {
        for (int i = 0; i < m_Bodies.size(); i++) {
            if (m_Bodies[i]->getJointPoints().size() > 0) {
                m_Bodies[i]->drawHands();
            }
        }
        unlock();
    }
}

void BodyStream::drawHandLeft()
{
    if (lock()) {
        for (int i = 0; i < m_Bodies.size(); i++) {
            if (m_Bodies[i]->getJointPoints().size() > 0) {
                m_Bodies[i]->drawHandLeft();
            }
        }
        unlock();
    }
}

void BodyStream::drawHandRight()
{
    if (lock()) {
        for (int i = 0; i < m_Bodies.size(); i++) {
            if (m_Bodies[i]->getJointPoints().size() > 0) {
                m_Bodies[i]->drawHandRight();
            }
        }
        unlock();
    }
}

size_t BodyStream::getNumBodies()
{
    return m_Bodies.size();
}

const Body *BodyStream::getBodyUsingIdx(int idx)
{
    if (lock()) {
        if (idx < m_Bodies.size()) {
            unlock();
            return m_Bodies[idx];
        }
        unlock();
    }

    return nullptr;
}

const Body *BodyStream::getBody(UINT64 id)
{
    for (int i = 0; i < m_Bodies.size(); i++) {
        if (m_Bodies[i]->getId() == id) {
            return m_Bodies[id];
        }
    }
    return m_Bodies[0];
}

ofShortPixels &BodyStream::getPixelsRef()
{
    return m_DoubleBuffer.getFrontBuffer();
}
