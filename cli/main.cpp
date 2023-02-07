#include <algorithm>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <stdint.h>
#include <vector>

#include <libraw/libraw.h>
#include <nbt.h>
#include <spdlog/spdlog.h>
#include <armadillo>

#include "image.h"
#include "frame.h"
#include "star_detection.h"
#include "PngImage.h"

using nbt::nbt_node;
using std::cout, std::endl;

int main(int argc, char **argv) {
    cout << "Hello" << endl;
    cout << "this tool will convert raw files into a more manageable nbt format" << endl;

    for (int arg = 0; arg < argc; ++arg)
        cout << argv[arg] << endl;

    // TODO: ${CMAKE_SRC_DIR}/tests/gimp
    fs::path root = "K:/cpp/astro/tests/gimp";

    constexpr int WINDOW        = 7;
    constexpr int CLOSEST_STARS = 5;
    constexpr int BUFFER        = CLOSEST_STARS + 5;
    std::vector<std::vector<arma::vec2>> all_the_stars;

    for (const auto &info : fs::directory_iterator{root}) {

        spdlog::info("extension: {}", info.path().extension().string());
        PngImage im{info.path()};
        im.reload();
        //    this is for importing and converting
        //    auto im = BayerImage::import_raw(info.path());

        // detect stars
        im.detect_stars();
        //
        //

        auto &detected_stars = all_the_stars.emplace_back();
        std::transform(im.detected_stars().cbegin(), im.detected_stars().cbegin() + BUFFER,
                       std::back_inserter(detected_stars), [](pixel_value<float> const &s) -> arma::vec2 {
                           return {s.x, s.y};
                       });
    }

    std::array<float, WINDOW> distances{};
    std::array<pixel_value<float>, CLOSEST_STARS> best_matches{};

    for (int i = 0; i < CLOSEST_STARS; ++i) {
        auto reference = all_the_stars[0][i];
        spdlog::info("reference: ({}, {})", reference[0], reference[1]);

        int offset = std::max(0, i - WINDOW / 2);

        for (int j = 0; j < WINDOW; ++j) {
            distances[j] = arma::norm(all_the_stars[1][offset + j] - reference);
        }

        auto idx = std::distance(distances.cbegin(), std::min_element(distances.cbegin(), distances.cend()));

        auto const &closest = all_the_stars[1][offset + idx];
        spdlog::info("closest to reference: {} | ({}, {})", distances[idx], closest[0], closest[1]);
        spdlog::info("");
        best_matches[i] = {(float)closest[0], (float)closest[1], distances[idx]};
    }

    // std::sort(best_matches.begin(), best_matches.end());

    spdlog::info("sorted by best match: ");
    for (const auto &[x, y, value] : best_matches) {
        spdlog::info("({}, {}) [{}]", x, y, value);
    }

    float average = std::accumulate(best_matches.begin(), best_matches.end(), 0.0f,
                                    [](float sum, pixel_value<float> const &p) { return sum + p.value; }) /
                    static_cast<float>(best_matches.size());

    spdlog::info("average: {}", average);
    spdlog::info("median: {}", best_matches[best_matches.size() / 2].value);

    using arma::mat, arma::vec6;

    spdlog::info("select 3 points: ");
    auto x       = all_the_stars[0][0];
    arma::vec2 x_prime = {best_matches[0].x, best_matches[0].y};
    auto y       = all_the_stars[0][1];
    arma::vec2 y_prime = {best_matches[1].x, best_matches[1].y};
    auto z       = all_the_stars[0][2];
    arma::vec2 z_prime = {best_matches[2].x, best_matches[2].y};

    mat A{6,6, arma::fill::zeros};
    A(0,0) = x(0);
    A(0,1) = x(1);
    A(0,4) = 1.0;
    A(1,2) = x(0);
    A(1,3) = x(1);
    A(1,5) = 1.0;

    A(2,0) = y(0);
    A(2,1) = y(1);
    A(2,4) = 1.0;
    A(3,2) = y(0);
    A(3,3) = y(1);
    A(3,5) = 1.0;

    A(4,0) = z(0);
    A(4,1) = z(1);
    A(4,4) = 1.0;
    A(5,2) = z(0);
    A(5,3) = z(1);
    A(5,5) = 1.0;

    arma::vec6 b = {
        x_prime[0], x_prime[1],
        y_prime[0], y_prime[1],
        z_prime[0], z_prime[1]
    };

    arma::vec solution = arma::solve(A, b);

    mat trafo = {
        {solution[0], solution[1], solution[4]},
        {solution[2], solution[3], solution[5]},
        {0.0, 0.0, 1.0}
    };

    std::cout << "Solution: " << solution << std::endl;

    for (int i = 0; i < 5; ++i) {
        auto reference = all_the_stars[0][i];
        arma::vec3 test_star         = {best_matches[i].x, best_matches[i].y, 1.0};
        arma::vec3 a = {reference[0], reference[1], 1.0};

        std::cout << "reference: \n" << reference << "\n";
        std::cout << "b        : \n" << test_star << "\n";

        arma::vec b_ = trafo * a;
        std::cout << "b'       : \n" << b_ << "\n";
    }
    std::cout << std::endl;

    return 0;
}
