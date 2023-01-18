#ifndef IMAGE_H_
#define IMAGE_H_

#include <filesystem>
#include <fstream>
#include <libraw/libraw.h>
#include <limits>
#include <memory>
#include <optional>
#include <spdlog/spdlog.h>
#include <zlib.h>

#include "frame.h"
#include "image_path.h"
#include "libraw/libraw_const.h"

class ImageFile {
  public:
    ImageFile(){};

    ImageFile(Frame<RGBFloat> const &frame, fs::path const &path) : m_frame(frame), m_path(path) {}

  public:
    /// reloads the image (to save RAM)
    void unload() { m_frame = {}; }

    /// reloads after an unload
    void reload() { load_binary(m_path); }

    bool is_loaded() const { return m_frame.is_loaded(); }

    auto frame() const -> Frame<RGBFloat> const & { return m_frame; }

    auto frame() -> Frame<RGBFloat> & { return const_cast<Frame<RGBFloat> &>(std::as_const(*this).frame()); }

    void change_path(fs::path const &path) { m_path = path; }

    auto path() const -> fs::path const & { return m_path; }

    auto filename() const -> fs::path { return m_path.filename(); }

    auto id() const -> std::string const & { return m_id; }

  protected:
    bool image_from_image_path(ImagePath const &path, std::unique_ptr<LibRaw> const &processor) {
        switch (path.type) {
        case ImagePath::RawImage: {
            // first loading, always convert
            load_raw(path.raw_path, processor);
            m_path = path.converted_path;
            return true;
        }
        case ImagePath::Converted: {

            // load_binary(path.converted_path);
            m_path = path.converted_path;
            return true;
        }
        case ImagePath::None:
            return false;
        }
    }

    void save_to_binary() const {

        // TODO: add checks
        std::ofstream ofile{m_path, std::ios::binary};

        size_t n_bytes = sizeof(RGBFloat) * m_frame.width() * m_frame.height();
        auto w         = m_frame.width();
        auto h         = m_frame.height();
        ofile.write(reinterpret_cast<const char *>(&w), sizeof(size_t));
        ofile.write(reinterpret_cast<const char *>(&h), sizeof(size_t));
        ofile.write(reinterpret_cast<const char *>(&m_meta), sizeof(FileMeta));

        ofile.write(reinterpret_cast<char const *>(m_frame.data()), n_bytes);
    }

  private:
    /// Loads a raw image file (e.g. Canon CR2) at the given file path.
    void load_raw(fs::path const &file, std::unique_ptr<LibRaw> const &processor) {

        spdlog::info("opening raw image \"{}\"", file.string());
        if (processor->open_file(file.c_str()) != LIBRAW_SUCCESS) {
            spdlog::error("opening raw image failed");
            return;
        }
        auto const &sizes = processor->imgdata.sizes;
        spdlog::info("Image size: {} {}", sizes.width, sizes.height);

        m_meta.aperture = processor->imgdata.other.aperture;
        m_meta.shutter  = processor->imgdata.other.shutter;
        m_meta.iso      = processor->imgdata.other.iso_speed;

        // we can stop here for lazy loading

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
                auto to_float = [](ushort value) {
                    return static_cast<float>(value) / std::numeric_limits<ushort>::max();
                };

                ushort r = get_channel(0);
                ushort g = get_channel(1) + get_channel(3);
                ushort b = get_channel(2);

                frame.push_back({to_float(r), to_float(g) * 0.5f, to_float(b)});
            }
        }
        processor->recycle();
    }

    void load_binary(fs::path const &filename) {
        std::ifstream ifile{filename, std::ios::binary};
        size_t w;
        size_t h;

        if (!ifile.is_open()) {
            spdlog::error("error opening binary image file");
            return;
        }

        ifile.read(reinterpret_cast<char *>(&w), sizeof(size_t));
        ifile.read(reinterpret_cast<char *>(&h), sizeof(size_t));

        ifile.read(reinterpret_cast<char *>(&m_meta), sizeof(FileMeta));

        auto frame     = Frame<RGBFloat>::empty(w, h);
        RGBFloat *data = frame.data();

        ifile.read(reinterpret_cast<char *>(data), sizeof(RGBFloat) * w * h);
    }

  public:
    enum FileType {
        Raw,
        Converted,
    };
    struct FileMeta {
        float shutter  = 0.0f;
        float iso      = 0.0f;
        float aperture = 0.0f;
    };

  private:
    fs::path m_path         = {};
    FileType m_type         = Raw;
    FileMeta m_meta         = {};
    Frame<RGBFloat> m_frame = {};

  protected:
    std::string m_id = "";
};

#endif // IMAGE_H_
