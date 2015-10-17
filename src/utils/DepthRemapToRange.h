// @author sadmb
// @date 10th,Feb,2014.
// modified from utils/DepthRemapToRange.h of ofxNI2 by @satoruhiga

#pragma once
#include "ofMain.h"

namespace ofxKinect2
{
inline void depthRemapToRange(const ofShortPixels &src, ofShortPixels &dst, int nearValue, int farValue, int invert)
{
    const int dataSize = src.getWidth() * src.getHeight();
    dst.allocate(src.getWidth(), src.getHeight(), 1);
    const unsigned short *srcPtr = src.getPixels();
    unsigned short *dstPtr = dst.getPixels();

    if (invert) {
        std::swap(nearValue, farValue);
    }

    for (int i = 0; i < dataSize; i++) {
        unsigned short C = *srcPtr;
        *dstPtr = ofMap(C, nearValue, farValue, 0, 65535, true);
        srcPtr++;
        dstPtr++;
    }
}
} // namespace ofxKinect2
