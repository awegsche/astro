#ifndef PHOTOBATCH_H_
#define PHOTOBATCH_H_

#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

class PhotoBatch {
  public:
    explicit PhotoBatch(fs::path const &path) : m_root(path) {}

    /// Combines darks (adding up their values).
    ///
    void combine_darks() {}

    /// Combines lights (adding up their values).
    ///
    void combine_lights() {}

  private:
    fs::path m_root;

    std::vector<fs::path> m_lights;
    std::vector<fs::path> m_darks;
};

#endif // PHOTOBATCH_H_
