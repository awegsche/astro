#ifndef IMAGE_H_
#define IMAGE_H_

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <libraw/libraw.h>
#include <limits>
#include <memory>
#include <optional>
#include <spdlog/spdlog.h>
#include <stdint.h>
#include <zlib.h>

#include "frame.h"
#include "image_path.h"
#include "libraw/libraw_const.h"
#include "nbt.h"
#include "star_detection.h"

using nbt::NbtTagType;

class Image {
  protected:
    fs::path m_filename;

  public:
    Image(fs::path const &filename) : m_filename(filename) {}

    virtual void reload() = 0;

    virtual void force_reload() = 0;

    virtual void unload() = 0;

    auto path() const -> fs::path const & { return m_filename; }
};

struct BayerChannel {
    Frame<Cu16> frame                              = {};
    std::vector<pixel_value<float>> detected_stars = {};
};

class BayerImage : public Image {
    BayerChannel m_red     = {};
    BayerChannel m_blue    = {};
    BayerChannel m_green_1 = {};
    BayerChannel m_green_2 = {};

  public:
    explicit BayerImage(fs::path const &filename) : Image(filename) {}

    /// imports a camera raw image and immediately saves it as `.nbt` file
    static BayerImage import_raw(fs::path const &filename) {
        auto nbt_filename = filename;
        nbt_filename.replace_extension("nbt");

        BayerImage im{nbt_filename};

        LibRaw processor{};

        processor.open_file(filename.string().c_str());

        spdlog::info("RAW file {}", filename.filename().string());
        processor.unpack();

        processor.raw2image();

        std::vector<uint8_t> reds;
        std::vector<uint8_t> blues;
        std::vector<uint8_t> greens_1;
        std::vector<uint8_t> greens_2;

        auto width        = processor.imgdata.sizes.width / 2;
        auto height       = processor.imgdata.sizes.height / 2;
        auto const &image = processor.imgdata.image;

        reds.reserve(width * height * 2);
        blues.reserve(width * height * 2);
        greens_1.reserve(width * height * 2);
        greens_2.reserve(width * height * 2);

        im.m_red.frame.reserve(width, height);
        im.m_blue.frame.reserve(width, height);
        im.m_green_1.frame.reserve(width, height);
        im.m_green_2.frame.reserve(width, height);

        const auto push_pixel = [width, height, &image](BayerChannel &channel, std::vector<uint8_t> &vec, int x, int y,
                                                        int ch) {
            auto idx       = y * width * 2 + x;
            uint16_t value = image[idx][ch];

            channel.frame.push_back({value});
            vec.push_back(static_cast<uint8_t>(value >> 8));
            vec.push_back(static_cast<uint8_t>(value & 0xFF));
        };

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                push_pixel(im.m_red, reds, 2 * x, 2 * y, 0);
                push_pixel(im.m_green_1, greens_1, 2 * x + 1, 2 * y, 1);
                push_pixel(im.m_green_2, greens_2, 2 * x, 2 * y + 1, 3);
                push_pixel(im.m_blue, blues, 2 * x + 1, 2 * y + 1, 2);
            }
        }

        nbt::compound nbt_file{};

        nbt_file.insert_node(static_cast<int32_t>(width), "width");
        nbt_file.insert_node(static_cast<int32_t>(height), "height");
        nbt_file.insert_node(reds, "reds");
        nbt_file.insert_node(greens_1, "greens_1");
        nbt_file.insert_node(greens_2, "greens_2");
        nbt_file.insert_node(blues, "blues");

        nbt::nbt_node n{std::move(nbt_file)};

        nbt::write_to_file(n, nbt_filename.string());

        return im;
    }

    // ---- meta information -----------------------------------------------------------------------
    //
    /// Saves the meta information to a file besides the origninal image
    void save_meta() const {
        nbt::compound nbt_stars{};

        auto const pixel_values_to_nbt = [](std::vector<pixel_value<float>> const &stars) -> std::vector<nbt::nbt_node> {
            std::vector<nbt::nbt_node> float_stars;
            for (const auto &s : stars) {
                nbt::compound nbt_s;

                nbt_s.insert_node(s.x, "x");
                nbt_s.insert_node(s.y, "y");
                nbt_s.insert_node(s.value, "value");

                float_stars.push_back(nbt::nbt_node{std::move(nbt_s)});
            }
            return float_stars;
        };

        nbt_stars.insert_node(pixel_values_to_nbt(m_red.detected_stars), "red stars");
        nbt_stars.insert_node(pixel_values_to_nbt(m_blue.detected_stars), "blue stars");
        nbt_stars.insert_node(pixel_values_to_nbt(m_green_1.detected_stars), "green1 stars");
        nbt_stars.insert_node(pixel_values_to_nbt(m_green_2.detected_stars), "green2 stars");

        nbt::write_to_file(std::move(nbt_stars), get_meta_path().string());
    }

    fs::path get_meta_path() const {

        auto meta_path = path();
        meta_path.replace_filename(meta_path.filename().string() + "_meta");
        return meta_path;
    }

    void load_meta() {
        auto meta = nbt::read_from_file(get_meta_path().string());
        auto const pixel_values_from_nbt =
            [](std::vector<nbt::nbt_node> const &list) -> std::vector<pixel_value<float>> {
            std::vector<pixel_value<float>> stars;
            for (const auto &s : list) {
                stars.push_back({
                    s.get_field<nbt::TAG_Float>("x"),
                    s.get_field<nbt::TAG_Float>("y"),
                    s.get_field<nbt::TAG_Float>("value"),
                });
            }
            return stars;
        };

        m_red.detected_stars     = pixel_values_from_nbt(meta.get_field<nbt::TAG_List>("red stars"));
        m_blue.detected_stars    = pixel_values_from_nbt(meta.get_field<nbt::TAG_List>("blue stars"));
        m_green_1.detected_stars = pixel_values_from_nbt(meta.get_field<nbt::TAG_List>("green1 stars"));
        m_green_2.detected_stars = pixel_values_from_nbt(meta.get_field<nbt::TAG_List>("green2 stars"));
    }

    // ---- lazy loading ---------------------------------------------------------------------------

    void reload() override {
        if (m_red.frame.empty())
            force_reload();
    }

    void force_reload() override {
        spdlog::info("reloading");
        nbt::nbt_node n = nbt::read_from_file(m_filename.string());
        if (!n) {
            spdlog::error("couldn't open bayer image '{}'", m_filename.string());
            return;
        }

        auto width  = n.get_field<NbtTagType::TAG_Int>("width");
        auto height = n.get_field<NbtTagType::TAG_Int>("height");
        // copy data over
        auto copy_nbt_array = [width, height](nbt::nbt_node const &node, char const *field) -> Frame<Cu16> {
            Frame<Cu16> frame{};
            frame.reserve(width, height);
            std::vector<uint8_t> nbt_frame = node.get_field<NbtTagType::TAG_Byte_Array>(field);
            for (int i = 0; i < width * height; ++i) {
                frame.push_back({static_cast<uint16_t>(nbt_frame[2 * i] << 8 | nbt_frame[2 * i + 1])});
            }
            return frame;
        };

        m_red.frame     = copy_nbt_array(n, "reds");
        m_blue.frame    = copy_nbt_array(n, "blues");
        m_green_1.frame = copy_nbt_array(n, "greens_1");
        m_green_2.frame = copy_nbt_array(n, "greens_2");
    }

    void unload() override {
        m_red     = {};
        m_blue    = {};
        m_green_1 = {};
        m_green_2 = {};
    }

    // ---- star detection -------------------------------------------------------------------------

    void detect_stars() {
        spdlog::info("detecting stars");
        m_red.detected_stars     = ::detect_stars(m_red.frame);
        m_green_1.detected_stars = ::detect_stars(m_green_1.frame);
        m_green_2.detected_stars = ::detect_stars(m_green_2.frame);
        m_blue.detected_stars    = ::detect_stars(m_blue.frame);

        std::sort(m_red.detected_stars.begin(), m_red.detected_stars.end(), std::greater<>{});
        std::sort(m_blue.detected_stars.begin(), m_blue.detected_stars.end(), std::greater<>{});
        std::sort(m_green_1.detected_stars.begin(), m_green_1.detected_stars.end(), std::greater<>{});
        std::sort(m_green_2.detected_stars.begin(), m_green_2.detected_stars.end(), std::greater<>{});

        save_meta();
    }

    auto reds() const -> BayerChannel const & { return m_red; }
    auto blues() const -> BayerChannel const & { return m_blue; }
    auto greens_1() const -> BayerChannel const & { return m_green_1; }
    auto greens_2() const -> BayerChannel const & { return m_green_2; }
};

#endif // IMAGE_H_
