#ifndef IMAGE_H_
#define IMAGE_H_

#include <filesystem>
#include <fstream>
#include <libraw/libraw.h>
#include <limits>
#include <optional>
#include <spdlog/spdlog.h>

#include "frame.h"
#include "image_path.h"
#include "libraw/libraw_const.h"

/// Loads a raw image file (e.g. Canon CR2) at the given file path using a LibRaw processor
auto load_raw_image(fs::path const &file, std::unique_ptr<LibRaw> const &processor) -> std::optional<Frame<RGBFloat>>;

/// Loads a raw image file (e.g. Canon CR2) at the given file path.
///
/// Note: This will create a LibRaw processor. If you are batch loading images, please consider
/// creating a LibRaw processor and reusing it using `load_raw_image`.
auto load_raw_image_without_processor(fs::path const &file) -> std::optional<Frame<RGBFloat>>;

/// Loads a binary dump file
auto load_binary(fs::path const &filename) -> std::optional<Frame<RGBFloat>>;

/// Dumps the pixel data to a binary file (this will create a rather large output file)
void write_to_binary(fs::path const &filename, Frame<RGBFloat> const &frame);

class ImageFile {

    fs::path m_path         = {};
    Frame<RGBFloat> m_frame = {};

  public:
    ImageFile(){};
    explicit ImageFile(Frame<RGBFloat> const &frame, fs::path const &path) : m_frame(frame), m_path(path) {}

  protected:
    bool image_from_image_path(ImagePath const &path, std::unique_ptr<LibRaw> const &processor) {
        switch (path.type) {
        case ImagePath::RawImage: {
            auto im = load_raw_image(path.raw_path, processor);
            if (im) {
                // if the image isn't a binary yet, convert it
                write_to_binary(path.converted_path, *im);
                // and set its path directly to the new one
                m_frame = *im;
                m_path  = path.converted_path;
                return true;
            } else {
                spdlog::error("couldn't create Light from image (see above for error message)");
                return false;
            }
        }
        case ImagePath::Converted: {
            auto im = load_binary(path.converted_path);
            if (im) {
                m_frame = *im;
                m_path  = path.converted_path;
                return true;
            } else {
                spdlog::error("couldn't create Light from image (see above for error message)");
                return false;
            }
        }
        case ImagePath::None:
            return false;
        }
    }

    void save_to_binary() const { write_to_binary(m_path, m_frame); }

  public:
    auto frame() const -> Frame<RGBFloat> const & { return m_frame; }

    auto frame() -> Frame<RGBFloat> & { return const_cast<Frame<RGBFloat> &>(std::as_const(*this).frame()); }

    void change_path(fs::path const &path) { m_path = path; }

    auto path() const -> fs::path const & { return m_path; }

    auto filename() const -> fs::path { return m_path.filename(); }
};

#endif // IMAGE_H_
