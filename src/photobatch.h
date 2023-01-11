#ifndef PHOTOBATCH_H_
#define PHOTOBATCH_H_

#include "image.h"
#include <filesystem>
#include <libraw/libraw.h>
#include <spdlog/spdlog.h>
#include <vector>

namespace fs = std::filesystem;

class PhotoBatch {
  public:
    explicit PhotoBatch(fs::path const &path) : m_root(path) {
        const auto path_lights = m_root / "lights";
        const auto path_darks  = m_root / "darks";

        LibRaw processor;

        for (auto const &f : fs::directory_iterator{path_lights}) {
            if (f.is_regular_file()) {
                m_lights.emplace_back(f.path(), &processor);
            }
        }

        for (auto const &f : fs::directory_iterator{path_darks}) {
            if (f.is_regular_file()) {
                m_darks.emplace_back(f.path(), &processor);
            }
        }
    }

    /// Combines darks (adding up their values).
    ///
    void combine_darks() {}

    /// Combines lights (adding up their values).
    ///
    void combine_lights() {}

  private:
    fs::path m_root;

    std::vector<Image> m_lights;
    std::vector<Image> m_darks;
};

#endif // PHOTOBATCH_H_
