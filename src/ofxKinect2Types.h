// @author sadmb
// @date 10,Jan,2014
#ifndef _OFX_KINECT2_TYPES_H_
#define _OFX_KINECT2_TYPES_H_
#include "ofxKinect2Enums.h"
#include "Kinect.h"
#include "D2d1.h"

namespace ofxKinect2
{
#define MAX_STR 256
#define MAX_SENSORS 10
typedef union {
    IColorFrameReader *colorFrameReader;
    IDepthFrameReader *depthFrameReader;
    IBodyFrameReader *bodyFrameReader;
    IBodyIndexFrameReader *bodyIndexFrameReader;
    IAudioBeamFrameReader *audioBeamFrameReader;
    IInfraredFrameReader *infraredFrameReader;
    ILongExposureInfraredFrameReader *longExposureInfraredFrameReader;
} StreamHandle;

typedef union {
    IColorCameraSettings *colorCameraSettings;
} CameraSettingsHandle;

typedef union {
    IKinectSensor *kinect2;
} DeviceHandle;

typedef struct {
    PixelFormat pixelFormat;
    int resolutionX;
    int resolutionY;
    float fps;
} Mode;

typedef struct {
    int dataSize;
    void *data;

    SensorType sensorType;
    UINT64 timestamp;
    int frameIndex;

    int width;
    int height;

    float horizontalFieldOfView;
    float verticalFieldOfView;
    float diagonalFieldOfView;

    Mode mode;
    int stride;
} Frame;

typedef struct {
    SensorType sensorType;
} SensorInfo;

typedef struct {
    char uri[MAX_STR];
    char vendor[MAX_STR];
    char name[MAX_STR];
} DeviceInfo;

} // namespace ofxKinect2

#endif // _OFX_KINECT2_TYPES_H_
