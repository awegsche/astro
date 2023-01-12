#include "mainwindow.h"
#include <filesystem>
#include <iostream>

#include <chrono>
#include <future>
#include <spdlog/spdlog.h>
#include <thread>

using namespace std::chrono_literals;

#include "image.h"

using std::cout, std::endl;

// DEBUG
#ifdef WIN32
constexpr char ROOT[] = "C:/";
#else
constexpr char ROOT[] = "/mnt/c";
#endif

int main(int argc, char *argv[]) {
    MainWindow window{2560, 1440};

    std::future<Image> future_im = std::async(std::launch::async, []() {
        return Image::load_without_processor(std::filesystem::path{ROOT} /
                                             "Astro/Raw2208/Andro_day1/lights/IMG_7419.CR2");
    });

    spdlog::info("load image");
    auto start = std::chrono::steady_clock::now();

    bool queue = true;

    while (!window.should_close()) {
        if (queue) {
            if (future_im.wait_for(1ms) == std::future_status::ready) {
                const auto elapsed    = std::chrono::steady_clock::now() - start;
                const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
                spdlog::info("image loaded in {} ms", elapsed_ms.count());

                const auto &im = future_im.get();
                window.m_screen.load_data_cpu(im.width(), im.height(), im.data());

                auto data = reinterpret_cast<float const *>(im.data());

                for (int i = 0; i < 10; ++i) {
                    spdlog::info("{} {} {}", data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
                }
                queue = false;
            }
        }

        window.begin_frame();
        window.end_frame();
    }
    return 0;
}
