#include "image_path.h"
auto get_image_path(fs::directory_entry const &f) -> ImagePath {
    if (f.is_regular_file()) {
        // check if tiff already exists
        auto path_converted = f.path().parent_path().parent_path() / "converted" / f.path().filename();
        path_converted.replace_extension("dump");
        if (fs::exists(path_converted)) {
            spdlog::info("loading converted file");
            return {ImagePath::Converted, path_converted, f.path()};
        } else {
            return {ImagePath::RawImage, path_converted, f.path()};
        }
    }
    return {};
}
