#ifndef FRAME_H_
#define FRAME_H_

#include <cstddef>
#include <stdint.h>
#include <utility>
#include <vector>

constexpr float THIRD = 1.0f / 3.0f;

struct RGBFloat {
    float r;
    float g;
    float b;

    float mag() const { return THIRD * (r + g + b); }
};

struct RGBu16 {
    uint16_t r;
    uint16_t g;
    uint16_t b;

    uint16_t mag() const { return (r + g + b) / 3; }
};

struct CFloat {
    float ch;

    float mag() const { return ch; }
};

struct Cu16 {
    uint16_t ch;

    uint16_t mag() const { return ch; }
};

/// The `Frame` class represents image data.
/// This can be a single exposure or already partially processed image data
/// up to the final output image.
template <typename Color> class Frame {
  public:
    /// the actual pixel data
    std::vector<Color> m_data = {};

  private:
    size_t m_width  = 0;
    size_t m_height = 0;

  public:
    Frame() {}
    Frame(size_t width, size_t height) : m_width(width), m_height(height) { m_data.reserve(width * height); }

    static Frame empty(size_t width, size_t height) {
        Frame frame{};
        frame.resize(width, height);
        return frame;
    }

    bool is_loaded() const { return m_width == 0 && m_height == 0; }

    /// resets the sizes
    /// Attention: this does not modify the internal vector in any way.
    /// Please make sure to apropriately resize the vector by e.g. pushing enough values;
    void set_size(size_t new_width, size_t new_height) {
        m_width  = new_width;
        m_height = new_height;
    }

    // ---- STL-like access ------------------------------------------------------------------------
    //
    void resize(size_t new_width, size_t new_height) {
        m_width  = new_width;
        m_height = new_height;
        m_data.resize(new_height * new_width);
    }

    void reserve(size_t new_size) { m_data.reserve(new_size * 3); }

    void push_back(Color c) { m_data.push_back(c); }

    constexpr auto begin() noexcept { return m_data.begin(); }

    constexpr auto end() noexcept { return m_data.begin(); }

    auto data() const -> const Color * { return m_data.data(); }
    auto data() -> Color * { return const_cast<Color *>(std::as_const(*this).data()); }

    // ---- Accessors ------------------------------------------------------------------------------
    /// Returns the pixel at position (x,y)
    Color at(size_t x, size_t y) const { return m_data[y * m_width + x]; }

    /// Accesses the pixel at position (x,y) mutably
    Color &at_mut(size_t x, size_t y) { return m_data[y * m_width + x]; }

    // ---- Properties -----------------------------------------------------------------------------

    size_t width() const { return m_width; };

    size_t height() const { return m_height; };
};

#endif // FRAME_H_
