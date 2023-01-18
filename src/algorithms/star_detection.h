#ifndef STAR_DETECTION_H_
#define STAR_DETECTION_H_

#include <algorithm>
#include <imgui.h>
#include <numeric>
#include <spdlog/spdlog.h>

#include "frame.h"
#include "screen.h"

template <typename T> struct pixel_value {
    T x;
    T y;
    float value;
};

inline auto detect_stars(Frame<RGBFloat> const &frame) -> std::vector<pixel_value<float>> {

    Frame<float> reduced_frame;
    reduced_frame.resize(frame.width(), frame.height());
    float threshold = 0.0f;

    for (int i = 0; i < frame.width(); ++i) {
        for (int j = 0; j < frame.height(); ++j) {
            float mag                  = frame.at(i, j).mag();
            reduced_frame.at_mut(i, j) = mag;
            threshold += mag;
        }
    }
    threshold *= 2.0f / static_cast<float>(
                            frame.width() *
                            frame.height()); // this will detect stars that are at least twice as bright as the average

    const auto add_adj = [](std::vector<std::pair<int, int>> &stack, int i, int j) {
        stack.push_back({i - 1, j - 1});
        stack.push_back({i, j - 1});
        stack.push_back({i + 1, j - 1});
        stack.push_back({i - 1, j});
        stack.push_back({i + 1, j});
        stack.push_back({i - 1, j + 1});
        stack.push_back({i, j + 1});
        stack.push_back({i + 1, j + 1});
    };

    std::vector<pixel_value<float>> stars;

    for (int i = 10; i < reduced_frame.width() - 10; ++i) {
        for (int j = 10; j < reduced_frame.height() - 10; ++j) {

            if (reduced_frame.at(i, j) > threshold) {
                std::vector<pixel_value<int>> adjacent_pixels;
                adjacent_pixels.push_back({i, j, reduced_frame.at(i, j)});
                reduced_frame.at_mut(i, j) = 0.0f;

                std::vector<std::pair<int, int>> stack;
                add_adj(stack, i, j);

                while (!stack.empty()) {
                    auto [x, y] = stack.back();
                    stack.pop_back();

                    if (x >= 0 && y >= 0 && x < reduced_frame.width() && y < reduced_frame.height() &&
                        reduced_frame.at(x, y) > threshold) {
                        adjacent_pixels.push_back({x, y, reduced_frame.at(x, y)});
                        reduced_frame.at_mut(x, y) = 0.0f;
                        add_adj(stack, x, y);
                    }
                }

                const auto accumulator = [](pixel_value<float> const &a,
                                            pixel_value<int> const &b) -> pixel_value<float> {
                    return {a.x + (float)b.x * b.value, a.y + (float)b.y * b.value, a.value + b.value};
                };
                float l              = static_cast<float>(adjacent_pixels.size());
                pixel_value<float> p = std::accumulate(adjacent_pixels.begin(), adjacent_pixels.end(),
                                                       pixel_value<float>{0.0f, 0.0f, 0.0f}, accumulator);
                p.x /= p.value;
                p.y /= p.value;
                stars.push_back(p);
            }
        }
    }

    spdlog::info("star finding complete, found {} stars", stars.size());
    return stars;

    /*
    for (const auto &star : stars) {
        // draw a tiny cross hair
        spdlog::info("({}, {}, {})", star.x, star.y, star.value);

        int x = star.x;
        int y = star.y;

        for (int d = 2; d <= 5; ++d) {
            frame.at_mut(x - d, y) = {1.0f, 0.0f, 0.0f};
            frame.at_mut(x + d, y) = {1.0f, 0.0f, 0.0f};

            frame.at_mut(x, y - d) = {1.0f, 0.0f, 0.0f};
            frame.at_mut(x, y + d) = {1.0f, 0.0f, 0.0f};
        }
    }
    */
}

class StarDetectorUi {
    Screen m_screen;
    Frame<RGBFloat> m_frame;

  public:
    StarDetectorUi(Frame<RGBFloat> const &image) {
        // only a subset, TODO: remove

        m_frame.resize(image.width(), image.height());

        for (int i = 0; i < image.width(); ++i) {
            for (int j = 0; j < image.height(); ++j) {
                m_frame.at_mut(i, j) = image.at(i, j);
            }
        }

        detect_stars(m_frame);

        m_screen.init();
        m_screen.load_data_cpu(image.width(), image.height(), reinterpret_cast<const float *>(m_frame.data()));
    }

    void draw_imgui() const {
        ImGui::Begin("StarDetector");
        m_screen.display(m_frame.width(), m_frame.height());
        ImGui::End();
    }
};

#endif // STAR_DETECTION_H_
