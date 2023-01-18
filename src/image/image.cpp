#include "image.h"
#include "libraw/libraw.h"
#include <memory>
void write_to_binary(fs::path const &filename, Frame<RGBFloat> const &frame) {
    // TODO: add checks
    std::ofstream ofile{filename, std::ios::binary};

    size_t n_bytes = sizeof(RGBFloat) * frame.width() * frame.height();
    auto w         = frame.width();
    auto h         = frame.height();
    ofile.write(reinterpret_cast<const char *>(&w), sizeof(size_t));
    ofile.write(reinterpret_cast<const char *>(&h), sizeof(size_t));
    ofile.write(reinterpret_cast<char const *>(frame.data()), n_bytes);
}
auto load_raw_image_without_processor(fs::path const &file) -> std::optional<Frame<RGBFloat>> {
    auto processor = std::make_unique<LibRaw>();
    return load_raw_image(file, processor);
}
auto load_binary(fs::path const &filename) -> std::optional<Frame<RGBFloat>> {
    std::ifstream ifile{filename, std::ios::binary};
    size_t w;
    size_t h;

    if (!ifile.is_open()) {
        spdlog::error("error opening binary image file");
        return {};
    }

    ifile.read(reinterpret_cast<char *>(&w), sizeof(size_t));
    ifile.read(reinterpret_cast<char *>(&h), sizeof(size_t));

    auto frame     = Frame<RGBFloat>::empty(w, h);
    RGBFloat *data = frame.data();

    ifile.read(reinterpret_cast<char *>(data), sizeof(RGBFloat) * w * h);

    return frame;
}
auto load_raw_image(fs::path const &file, std::unique_ptr<LibRaw> const &processor) -> std::optional<Frame<RGBFloat>> {

    spdlog::info("opening raw image \"{}\"", file.string());
    if (processor->open_file(file.c_str()) != LIBRAW_SUCCESS) {
        spdlog::error("opening raw image failed");
        return {};
    }
    auto const &sizes = processor->imgdata.sizes;
    spdlog::info("Image size: {} {}", sizes.width, sizes.height);

    processor->unpack();
    processor->raw2image();

    auto frame         = Frame<RGBFloat>{(size_t)sizes.width - 1, (size_t)sizes.height - 1};
    auto const &imdata = processor->imgdata.image;

    for (int j = 0; j < sizes.height - 1; ++j) {
        for (int i = 0; i < sizes.width - 1; ++i) {
            // debayer
            // [R] G R G
            //  G  B G B
            auto idx11 = j * sizes.width + i;
            auto idx12 = idx11 + 1;
            auto idx21 = idx11 + sizes.width;
            auto idx22 = idx21 + 1;

            auto get_channel = [&imdata, idx11, idx12, idx21, idx22](int ch) {
                return imdata[idx11][ch] + imdata[idx12][ch] + imdata[idx21][ch] + imdata[idx22][ch];
            };
            auto to_float = [](ushort value) { return static_cast<float>(value) / std::numeric_limits<ushort>::max(); };

            ushort r = get_channel(0);
            ushort g = get_channel(1) + get_channel(3);
            ushort b = get_channel(2);

            frame.push_back({to_float(r), to_float(g) * 0.5f, to_float(b)});
        }
    }

    processor->recycle();
    return frame;
}
