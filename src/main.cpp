#include "mainwindow.h"
#include <filesystem>
#include <iostream>

using std::cout, std::endl;

int main(int argc, char *argv[]) {

    cout << "hello from astro" << endl;

    MainWindow window;

    while (!window.should_close()) {
        window.begin_frame();
        window.end_frame();
    }

    return 0;
}
