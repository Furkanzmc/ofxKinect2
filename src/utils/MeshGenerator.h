// @author sadmb
// @date 10th,Feb,2014.
// modified from utils/DepthRemapToRange.h of ofxNI2 by @satoruhiga

#pragma once
#include "ofxKinect2.h"

namespace ofxKinect2
{
class MeshGenerator;
} // namespace ofxKinect2

class ofxKinect2::MeshGenerator
{
public:
    MeshGenerator()
        : m_DownsamplingLevel(1)
    {

    }

    void setup(DepthStream &depthStream)
    {
        const float fovH = depthStream.getHorizontalFieldOfView();
        const float fovV = depthStream.getVerticalFieldOfView();

        m_xzFactor = tan(fovH * 0.5) * 2;
        m_yzFactor = tan(fovV * 0.5) * -2;
    }

    const ofMesh &update(const ofShortPixels &depth, const ofPixels &color = ofPixels())
    {
        assert(depth.getNumChannels() == 1);

        const int depthWidth = depth.getWidth();
        const int depthHeight = depth.getHeight();
        const float invW = 1. / depthWidth;
        const float invH = 1. / depthHeight;
        const unsigned short *depthPixels = depth.getPixels();

        const bool hasColor = color.isAllocated();
        const float invByte = 1. / 255.;
        m_Mesh.setMode(OF_PRIMITIVE_POINTS);

        vector<ofVec3f> &verts = m_Mesh.getVertices();
        verts.resize((depthWidth / m_DownsamplingLevel) * (depthHeight / m_DownsamplingLevel));
        int vertIndex = 0;

        if (hasColor) {
            const unsigned char *colorPixel = color.getPixels();
            vector<ofFloatColor> &colors = m_Mesh.getColors();
            colors.resize((depthWidth / m_DownsamplingLevel) * (depthHeight / m_DownsamplingLevel));

            if (color.getNumChannels() == 1) {
                for (int y = 0; y < depthHeight; y += m_DownsamplingLevel) {
                    for (int x = 0; x < depthWidth; x += m_DownsamplingLevel) {
                        const int idx = y * depthWidth + x;
                        const float Z = depthPixels[idx];
                        const float normX = x * invW - 0.5;
                        const float normY = y * invH - 0.5;
                        const float X = normX * m_xzFactor * Z;
                        const float Y = normY * m_yzFactor * Z;
                        verts[vertIndex].set(X, Y, -Z);

                        const unsigned char *C = &colorPixel[idx];
                        colors[vertIndex].set(C[0] * invByte);
                        vertIndex++;
                    }
                }
            }
            else if (color.getNumChannels() == 3) {
                for (int y = 0; y < depthHeight; y += m_DownsamplingLevel) {
                    for (int x = 0; x < depthWidth; x += m_DownsamplingLevel) {
                        const int idx = y * depthWidth + x;
                        const float Z = depthPixels[idx];
                        const float normX = x * invW - 0.5;
                        const float normY = y * invH - 0.5;
                        const float X = normX * m_xzFactor * Z;
                        const float Y = normY * m_yzFactor * Z;
                        verts[vertIndex].set(X, Y, -Z);

                        const unsigned char *C = &colorPixel[idx * 3];
                        colors[vertIndex].set(C[0] * invByte, C[1] * invByte, C[2] * invByte);
                        vertIndex++;
                    }
                }
            }
            else {
                throw;
            }

            m_Mesh.addColors(colors);
        }
        else {
            for (int y = 0; y < depthHeight; y += m_DownsamplingLevel) {
                for (int x = 0; x < depthWidth; x += m_DownsamplingLevel) {
                    int idx = y * depthWidth + x;

                    float Z = depthPixels[idx];
                    float X = (x * invW - 0.5) * m_xzFactor * Z;
                    float Y = (y * invH - 0.5) * m_yzFactor * Z;
                    verts[vertIndex].set(X, Y, -Z);
                    vertIndex++;
                }
            }
        }
        return m_Mesh;
    }

    void draw()
    {
        m_Mesh.draw();
    }

    void setDownsamplingLevel(int level)
    {
        m_DownsamplingLevel = level;
    }
    int getDownsamplingLevel() const
    {
        return m_DownsamplingLevel;
    }

    ofMesh &getMesh()
    {
        return m_Mesh;
    }

protected:
    int m_DownsamplingLevel;
    ofMesh m_Mesh;
    float m_xzFactor, m_yzFactor;

};
