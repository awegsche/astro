#include <algorithm>
#include <filesystem>
#include <image.h>
#include <iostream>
#include <libraw/libraw.h>
#include <nbt.h>
#include <spdlog/spdlog.h>
#include <stdint.h>
#include <vector>

#include <eigen3/Eigen/Dense>

#include "frame.h"
#include "star_detection.h"

using nbt::nbt_node;
using std::cout, std::endl;

int main(int argc, char **argv) {
    cout << "Hello" << endl;
    cout << "this tool will convert raw files into a more manageable nbt format" << endl;

    for (int arg = 0; arg < argc; ++arg)
        cout << argv[arg] << endl;

    fs::path root = "K:/Astro/test/lights";

    std::vector<std::vector<Eigen::Vector2f>> all_the_stars;

    for (const auto &info : fs::directory_iterator{root}) {

        spdlog::info("extension: {}", info.path().extension().string());
        if (info.path().extension() != ".nbt")
            continue;

        spdlog::info("loading nbt image {}", info.path().string());
        BayerImage im{info.path()};
        // im.reload();
        //  this is for importing and converting
        //  auto im = BayerImage::import_raw(info.path());

        // detect stars
        // im.detect_stars();
        //
        //
        im.load_meta();

        auto &detected_stars = all_the_stars.emplace_back();
        std::transform(im.reds().detected_stars.cbegin(), im.reds().detected_stars.cbegin() + 30,
                       std::back_inserter(detected_stars), [](pixel_value<float> const &s) -> Eigen::Vector2f {
                           return {s.x, s.y};
                       });
    }

    constexpr int WINDOW = 5;

    std::array<float, WINDOW> distances;

    for (int i = 0; i < 10; ++i) {
        auto reference = all_the_stars[0][i];
        spdlog::info("reference: ({}, {})", reference.x(), reference.y());

        int offset = std::max(0, i - WINDOW / 2);

        for (int j = 0; j < WINDOW; ++j) {
            distances[j] = (all_the_stars[1][offset + j] - reference).norm();
        }

        auto idx = std::distance(distances.cbegin(), std::min_element(distances.cbegin(), distances.cend()));

        auto const &closest = all_the_stars[1][offset + idx];
        spdlog::info("closest to reference: {} | ({}, {})", distances[idx], closest.x(), closest.y());
    }

    return 0;
}
