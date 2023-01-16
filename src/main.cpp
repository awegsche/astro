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
constexpr char ROOT[] = "K:/";
#else
constexpr char ROOT[] = "/mnt/c";
#endif

int main(int argc, char *argv[]) {
    MainWindow window{std::filesystem::path{ROOT} / "Astro/test", 1920, 1024};

    spdlog::info("load image");
    auto start = std::chrono::steady_clock::now();

    bool queue = true;

    while (!window.should_close()) {
        window.begin_frame();
        window.end_frame();
    }
    return 0;
}
