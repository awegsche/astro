#ifndef IMAGE_PATH_H_
#define IMAGE_PATH_H_

#include <filesystem>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;
/// Utilities for image path resolution

struct ImagePath {
    enum ImageType { RawImage, Converted, None } type = None;

    fs::path converted_path = {};
    fs::path raw_path       = {};
};

auto get_image_path(fs::directory_entry const &f) -> ImagePath;

#endif // IMAGE_PATH_H_
