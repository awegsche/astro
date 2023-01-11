#ifndef FRAME_H_
#define FRAME_H_

#include <cstddef>
#include <vector>

/// The `Frame` class represents image data.
/// This can be a single exposure or already partially processed image data
/// up to the final output image.
class Frame {
    /// the actual pixel data
    std::vector<float> m_data = {};

    size_t m_width  = 0;
    size_t m_height = 0;

  public:
    Frame() {}
    Frame(size_t width, size_t height) : m_width(width), m_height(height) {}
};

#endif // FRAME_H_
