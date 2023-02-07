//
// Created by andiw on 07/02/2023.
//

#include <cstdio>
#include <cstdlib>
#include <spdlog/spdlog.h>

#include "image.h"
#include "frame.h"

#ifndef ASTRO_PNGIMAGE_H
#define ASTRO_PNGIMAGE_H

class PngImage: public Image {
  public:
    explicit PngImage(const std::filesystem::path& filename)
    : Image(filename) { }

    void force_reload() override {
        // loading: https://gist.github.com/niw/5963798
        FILE *fp = fopen(m_filename.string().c_str(), "rb");

        auto _abort = [fp](const char* message) {
            spdlog::error("unable to load png file: {}", message);
            fclose(fp);
        };

        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png) {
            _abort("couldn't read version string");
            return;
        }
        png_infop info = png_create_info_struct(png);
        if (!info) {
            _abort("couldn't create info struct");
            return;
        }

        if (setjmp(png_jmpbuf(png))) {
            _abort("couldn't set jmpbuf");
            return;
        }

        png_init_io(png, fp);
        png_read_info(png, info);

        const auto w = png_get_image_width(png, info);
        const auto h = png_get_image_height(png, info);
        const auto color_type = png_get_color_type(png, info);
        const auto bit_depth = png_get_bit_depth(png, info);

        if (bit_depth == 16)
            png_set_strip_16(png);

        if (color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_palette_to_rgb(png);

        // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
        if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
            png_set_expand_gray_1_2_4_to_8(png);

        if(png_get_valid(png, info, PNG_INFO_tRNS))
            png_set_tRNS_to_alpha(png);

        // These color_type don't have an alpha channel then fill it with 0xff.
        if(color_type == PNG_COLOR_TYPE_RGB ||
           color_type == PNG_COLOR_TYPE_GRAY ||
           color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

        if(color_type == PNG_COLOR_TYPE_GRAY ||
           color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png);

        png_read_update_info(png, info);

        auto row_pointers = reinterpret_cast<png_bytep*>(malloc(sizeof(png_bytep)*h));
        for (int y = 0; y < h; ++y) {
            row_pointers[y] = reinterpret_cast<png_byte*>(malloc(png_get_rowbytes(png, info)));
        }
        png_read_image(png, row_pointers);
        fclose(fp);

        m_frame.resize(w, h);

        for (int y = 0; y < h; ++y) {
            png_bytep row = row_pointers[y];
            for (int x = 0; x < w; ++x) {
                png_bytep px = &(row[x*4]);
                m_frame.at_mut(x,y) = {
                    px[0] / 255.0f,
                    px[1] / 255.0f,
                    px[2] / 255.0f
                };
            }
        }

        png_destroy_read_struct(&png, &info, NULL);

    }

    void detect_stars() {
        m_detected_stars = ::detect_stars(m_frame);
    }

    bool is_loaded() const override {
        return !m_frame.empty();
    }

    void unload() override {
        // TODO: implement
        assert(false);
    }

    [[nodiscard]]
    Frame<RGBFloat> const& frame() const {
        return m_frame;
    }

    [[nodiscard]]
    std::vector<pixel_value<float>> const& detected_stars() {
        return m_detected_stars;
    }

  private:
    Frame<RGBFloat> m_frame;
    std::vector<pixel_value<float>> m_detected_stars;

};

#endif // ASTRO_PNGIMAGE_H
