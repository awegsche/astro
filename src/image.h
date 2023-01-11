#ifndef IMAGE_H_
#define IMAGE_H_

#include "frame.h"
#include <filesystem>
#include <libraw/libraw.h>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

class Image {
    std::filesystem::path m_path;
    Frame m_frame;

  public:
    Image(fs::path const &file, LibRaw *processor) : m_path(file) {
        spdlog::info("opening image \"{}\"", m_path.string());
        processor->open_file(m_path.c_str());
        auto const &sizes = processor->imgdata.sizes;
        spdlog::info("Image size: {} {}", sizes.width, sizes.height);

        processor->recycle();
    }

    static auto load_without_processor(fs::path const &file) {
        LibRaw processor;
        return Image{file, &processor};
    }
};
#endif // IMAGE_H_
