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

        processor->unpack();
        processor->raw2image();

        m_frame            = Frame<float>{(size_t)sizes.width - 1, (size_t)sizes.height - 1};
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

                m_frame.m_data.push_back(to_float(r));
                m_frame.m_data.push_back(to_float(g) * 0.5f);
                m_frame.m_data.push_back(to_float(b));
                // m_frame.m_data.push_back(1.0f);
            }
        }

        processor->recycle();
    }

    auto width() const -> int { return m_frame.m_width; }
    auto height() const -> int { return m_frame.m_height; }
    auto data() const -> const void * { return (void *)m_frame.m_data.data(); }

    static auto load_without_processor(fs::path const &file) {
        LibRaw processor;
        return Image{file, &processor};
    }
};
#endif // IMAGE_H_
