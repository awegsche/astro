#include <algorithm>
#include <filesystem>
#include <image.h>
#include <iostream>
#include <libraw/libraw.h>
#include <nbt.h>
#include <numeric>
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

    constexpr int WINDOW        = 7;
    constexpr int CLOSEST_STARS = 30;
    constexpr int BUFFER        = CLOSEST_STARS + 10;
    std::vector<std::vector<Eigen::Vector2f>> all_the_stars;

    for (const auto &info : fs::directory_iterator{root}) {

        spdlog::info("extension: {}", info.path().extension().string());
        if (info.path().extension() != ".nbt")
            continue;

        spdlog::info("loading nbt image {}", info.path().string());
        BayerImage im{info.path()};
        //  im.reload();
        //    this is for importing and converting
        //    auto im = BayerImage::import_raw(info.path());

        // detect stars
        // im.detect_stars();
        //
        //
        im.load_meta();

        auto &detected_stars = all_the_stars.emplace_back();
        std::transform(im.reds().detected_stars.cbegin(), im.reds().detected_stars.cbegin() + BUFFER,
                       std::back_inserter(detected_stars), [](pixel_value<float> const &s) -> Eigen::Vector2f {
                           return {s.x, s.y};
                       });
    }

    std::array<float, WINDOW> distances;
    std::array<pixel_value<float>, CLOSEST_STARS> best_matches;

    for (int i = 0; i < CLOSEST_STARS; ++i) {
        auto reference = all_the_stars[0][i];
        spdlog::info("reference: ({}, {})", reference.x(), reference.y());

        int offset = std::max(0, i - WINDOW / 2);

        for (int j = 0; j < WINDOW; ++j) {
            distances[j] = (all_the_stars[1][offset + j] - reference).norm();
        }

        auto idx = std::distance(distances.cbegin(), std::min_element(distances.cbegin(), distances.cend()));

        auto const &closest = all_the_stars[1][offset + idx];
        spdlog::info("closest to reference: {} | ({}, {})", distances[idx], closest.x(), closest.y());
        spdlog::info("");
        best_matches[i] = {closest.x(), closest.y(), distances[idx]};
    }

    std::sort(best_matches.begin(), best_matches.end());

    spdlog::info("sorted by best match: ");
    for (const auto &[x, y, value] : best_matches) {
        spdlog::info("({}, {}) [{}]", x, y, value);
    }

    float average = std::accumulate(best_matches.begin(), best_matches.end(), 0.0f,
                                    [](float sum, pixel_value<float> const &p) { return sum + p.value; }) /
                    static_cast<float>(best_matches.size());

    spdlog::info("average: {}", average);
    spdlog::info("median: {}", best_matches[best_matches.size() / 2].value);

    return 0;
}
