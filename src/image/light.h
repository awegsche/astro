#ifndef LIGHT_H_
#define LIGHT_H_

#include <filesystem>
#include <fstream>
#include <libraw/libraw.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <utility>
#include <vector>

#include "algorithms/star_detection.h"
#include "frame.h"
#include "image.h"
#include "image_path.h"

using nlohmann::json;

class Light : public ImageFile {
  public:
    Light() {}

    static std::optional<Light> load(ImagePath const &path, std::unique_ptr<LibRaw> const &processor) {
        Light l{};
        if (!l.image_from_image_path(path, processor))
            return {};
        // TODO: load stars from json
        auto meta_path = path.converted_path;
        meta_path.replace_extension("json");
        if (fs::exists(meta_path)) {
            std::ifstream ifile{meta_path};
            json meta  = json::parse(ifile);
            auto stars = meta["stars"];
            for (const auto &s : stars) {
                l.m_stars.push_back({s[0], s[1], s[2]});
            }
        } else {
            spdlog::info("detecting stars");
            l.m_stars = detect_stars(l.frame());
        }

        return l;
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

            auto meta_path = path();
            meta_path.replace_extension("json");
            std::ofstream ofile{meta_path};
            ofile << meta;
        }
    }

  private:
    std::vector<pixel_value<float>> m_stars = {};
};

#endif // LIGHT_H_
