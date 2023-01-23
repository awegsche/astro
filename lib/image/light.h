#ifndef LIGHT_H_
#define LIGHT_H_

#include <filesystem>
#include <fstream>
#include <libraw/libraw.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <utility>
#include <vector>

#include "frame.h"
#include "image.h"
#include "image_path.h"
#include "star_detection.h"

using nlohmann::json;

template <typename TImage> class Light {
  public:
    Light() {}

    Light(fs::path const &path) : m_image(path) {

        auto meta_path = path;
        meta_path.replace_extension("json");
        if (fs::exists(meta_path)) {
            std::ifstream ifile{meta_path};
            json meta  = json::parse(ifile);
            auto stars = meta["stars"];
            for (const auto &s : stars) {
                m_stars.push_back({s[0], s[1], s[2]});
            }
        }
    }

    void save() const {
        if (!m_stars.empty()) {
            json meta;
            json stars;

            for (const auto &star : m_stars) {
                json json_star;
                json_star.push_back(star.x);
                json_star.push_back(star.y);
                json_star.push_back(star.value);
                stars.push_back(json_star);
            }
            meta.push_back(stars);

            auto meta_path = m_image.path();
            meta_path.replace_extension("json");
            std::ofstream ofile{meta_path};
            ofile << meta;
        }
    }

  private:
    TImage m_image;
    std::vector<pixel_value<float>> m_stars = {};
};

#endif // LIGHT_H_
