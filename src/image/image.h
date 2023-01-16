#ifndef IMAGE_H_
#define IMAGE_H_

#include "frame.h"
#include <filesystem>
#include <fstream>
#include <libraw/libraw.h>
#include <limits>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

class Image {
  public:
    std::filesystem::path m_path = {};
    Frame<RGBFloat> m_frame      = {};

  public:
    Image(fs::path const &file) : m_path(file) {}

    auto width() const -> size_t { return m_frame.width(); }
    auto height() const -> size_t { return m_frame.height(); }

    auto data() const -> const RGBFloat * { return m_frame.data(); }
    auto data() -> RGBFloat * { return const_cast<RGBFloat *>(std::as_const(*this).data()); }

    /// static loading and writing methods
    static auto load_raw(fs::path const &file, LibRaw *processor) -> Image {
        Image im{file};

        spdlog::info("opening image \"{}\"", file.string());
        processor->open_file(file.c_str());
        auto const &sizes = processor->imgdata.sizes;
        spdlog::info("Image size: {} {}", sizes.width, sizes.height);

        processor->unpack();
        processor->raw2image();

        im.m_frame         = Frame<RGBFloat>{(size_t)sizes.width - 1, (size_t)sizes.height - 1};
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
                auto to_float = [](ushort value) {
                    return static_cast<float>(value) / std::numeric_limits<ushort>::max();
                };

                ushort r = get_channel(0);
                ushort g = get_channel(1) + get_channel(3);
                ushort b = get_channel(2);

                im.m_frame.push_back({to_float(r), to_float(g) * 0.5f, to_float(b)});
                // m_frame.m_data.push_back(1.0f);
            }
        }

        processor->recycle();
        return im;
    }

    static auto load_without_processor(fs::path const &file) {
        LibRaw processor;
        return Image::load_raw(file, &processor);
    }

    static void write_to_binary(fs::path const &filename, Frame<RGBFloat> const &frame) {
        std::ofstream ofile{filename, std::ios::binary};

        size_t n_bytes = sizeof(RGBFloat) * frame.width() * frame.height();
        auto w         = frame.width();
        auto h         = frame.height();
        ofile.write(reinterpret_cast<const char *>(&w), sizeof(size_t));
        ofile.write(reinterpret_cast<const char *>(&h), sizeof(size_t));
        ofile.write(reinterpret_cast<char const *>(frame.data()), n_bytes);
    }

    static auto load_binary(fs::path const &filename) -> Image {
        std::ifstream ifile{filename, std::ios::binary};
        size_t w;
        size_t h;

        Image im{filename};

        if (!ifile.is_open()) {
            spdlog::error("error opening binary image file");
            return im;
        }

        ifile.read(reinterpret_cast<char *>(&w), sizeof(size_t));
        ifile.read(reinterpret_cast<char *>(&h), sizeof(size_t));

        im.m_frame.resize(w, h);
        RGBFloat *data = im.data();

        ifile.read(reinterpret_cast<char *>(data), sizeof(RGBFloat) * im.width() * im.height());

        return im;
    }
};

#endif // IMAGE_H_
