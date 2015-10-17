// @author sadmb
// @date 10th,Feb,2014.
// modified from utils/DepthRemapToRange.h of ofxNI2 by @satoruhiga

#pragma once
#include "ofMain.h"

namespace ofxKinect2
{
template <typename PixelType>
struct DoubleBuffer;
} // namespace ofxKinect2

template <typename PixelType>
struct ofxKinect2::DoubleBuffer {

public:
    DoubleBuffer()
        : m_FrontBufferIndex(0)
        , m_BackBufferIndex(1)
        , m_IsAllocated(false)
    {

    }

    void allocate(int w, int h, int channels)
    {
        if (m_IsAllocated) {
            return;
        }

        m_IsAllocated = true;
        m_Pixels[0].allocate(w, h, channels);
        m_Pixels[1].allocate(w, h, channels);
    }

    void deallocate()
    {
        if (!m_IsAllocated) {
            return;
        }

        m_IsAllocated = false;
        m_Pixels[0].clear();
        m_Pixels[1].clear();
    }

    PixelType &getFrontBuffer()
    {
        return m_Pixels[m_FrontBufferIndex];
    }

    const PixelType &getFrontBuffer() const
    {
        return m_Pixels[m_FrontBufferIndex];
    }

    PixelType &getBackBuffer()
    {
        return m_Pixels[m_BackBufferIndex];
    }

    const PixelType &getBackBuffer() const
    {
        return m_Pixels[m_BackBufferIndex];
    }

    void swap()
    {
        std::swap(m_FrontBufferIndex, m_BackBufferIndex);
    }

private:
    PixelType m_Pixels[2];
    int m_FrontBufferIndex, m_BackBufferIndex;
    bool m_IsAllocated;

};
