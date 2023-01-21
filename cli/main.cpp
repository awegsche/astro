#include "frame.h"
#include <filesystem>
#include <image.h>
#include <iostream>
#include <libraw/libraw.h>
#include <nbt.h>
#include <stdint.h>

using nbt::nbt_node;
using std::cout, std::endl;

template <typename Color> void to_nbt(fs::path const &path, Frame<Color> const &frame) {
    uint32_t width  = static_cast<uint32_t>(frame.width());
    uint32_t height = static_cast<uint32_t>(frame.height());
    uint32_t size   = width * height * static_cast<uint32_t>(sizeof(Color));

    nbt_node node{nbt::NbtTagType::TAG_Compound};
}

int main() {
    cout << "Hello" << endl;
    cout << "this tool will convert raw files into a more manageable nbt format" << endl;

    fs::path root = "C:/Astro/test/lights";

    LibRaw processor;

    nbt_node::compound meta;

    meta.insert_node(12, "width");
    meta.insert_node(12, "height");
    meta.insert_node(1024LL, "size");

    nbt_node n = std::move(meta);
    cout << n << endl;

    for (const auto &info : fs::directory_iterator{root}) {
        cout << info.path() << endl;

        processor.open_file(info.path().string().c_str());

        cout << "unpacking raw" << endl;
        processor.unpack();

        cout << "raw -> image" << endl;
        processor.raw2image();

        Frame<Cu16> reds;
        Frame<Cu16> blues;
        Frame<Cu16> greens_1;
        Frame<Cu16> greens_2;

        auto width        = processor.imgdata.sizes.width / 2;
        auto height       = processor.imgdata.sizes.height / 2;
        auto const &image = processor.imgdata.image;

        reds.resize(width, height);
        blues.resize(width, height);
        greens_1.resize(width, height);
        greens_2.resize(width, height);

        const auto get_index = [width, height](int x, int y) { return y * width + x; };

        cout << "converting raw data" << endl;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                greens_1.at_mut(x, y).ch = image[get_index(2 * x, 2 * y)][1];
                blues.at_mut(x, y).ch    = image[get_index(2 * x + 1, 2 * y)][2];
                reds.at_mut(x, y).ch     = image[get_index(2 * x, 2 * y + 1)][0];
                greens_2.at_mut(x, y).ch = image[get_index(2 * x + 1, 2 * y + 1)][3];
            }
        }

        cout << " done " << endl;

        for (int i = 0; i < 10; ++i) {
            cout << greens_1.data()[i].ch << endl;
        }
    }

    return 0;
}
