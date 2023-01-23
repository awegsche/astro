#include "ui/mainwindow.h"
#include <filesystem>
#include <iostream>

#include <chrono>
#include <future>
#include <spdlog/spdlog.h>
#include <thread>

// DEBUG
#ifdef WIN32
constexpr char ROOT[] = "K:/";
#else
constexpr char ROOT[] = "/mnt/c";
#endif

int main(int argc, char *argv[]) {
    MainWindow window{std::filesystem::path{ROOT} / "Astro/test", 1920, 1024};

    while (!window.should_close()) {
        window.begin_frame();
        window.end_frame();
    }
    return 0;
}
