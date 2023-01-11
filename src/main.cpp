#include "mainwindow.h"
#include <filesystem>
#include <iostream>

#include "image.h"

using std::cout, std::endl;

// DEBUG
#ifdef WIN32
constexpr char ROOT[] = "C:/";
#else
constexpr char ROOT[] = "/mnt/c";
#endif

int main(int argc, char *argv[]) {
    MainWindow window;

    Image im =
        Image::load_without_processor(std::filesystem::path{ROOT} / "Astro/Raw2208/Andro_day1/lights/IMG_7419.CR2");

    while (!window.should_close()) {
        window.begin_frame();
        window.end_frame();
    }
    return 0;
}
