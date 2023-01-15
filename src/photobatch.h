#ifndef PHOTOBATCH_H_
#define PHOTOBATCH_H_

#include "image.h"
#include <chrono>
#include <filesystem>
#include <future>
#include <imgui.h>
#include <libraw/libraw.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>

#include "screen.h"

using namespace std::chrono_literals;
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

  public:
    fs::path m_root;

    std::vector<Image> m_lights;
    std::vector<Image> m_darks;
};

// TODO: move this into separate files

class BatchUi {
    std::unique_ptr<PhotoBatch> m_photobatch = nullptr;
    std::future<std::unique_ptr<PhotoBatch>> m_future_load;

    // for display, cache the filenames here
    struct FrameMeta {
        std::string name;
        bool selected;
    };
    std::vector<FrameMeta> m_lights_metas;
    std::vector<FrameMeta> m_darks_metas;

    int m_selected_light = -1;
    int m_selected_dark  = -1;

    enum Status {
        Empty,
        Loading,
        Ready,
    };

    Status m_status = Empty;

  public:
    void load(std::filesystem::path const &root) {
        m_status = Loading;

        m_future_load = std::async(std::launch::async, [&root] { return std::make_unique<PhotoBatch>(root); });
    }

    /// Draws the photobatch UI.
    /// If a different
    void draw_imgui(Screen &screen) {
        if (!ImGui::CollapsingHeader("Batch"))
            return;
        switch (m_status) {
        case Empty: {
            ImGui::Text("empty");
            break;
        }
        case Loading: {
            ImGui::Text(" loading...");
            if (m_future_load.wait_for(0s) == std::future_status::ready) {
                m_photobatch  = m_future_load.get();
                m_future_load = {};
                m_status      = Ready;

                m_lights_metas.clear();
                m_lights_metas.reserve(m_photobatch->m_lights.size());
                for (auto const &l : m_photobatch->m_lights) {
                    m_lights_metas.push_back({l.m_path.filename().string(), true});
                }
                m_darks_metas.clear();
                m_darks_metas.reserve(m_photobatch->m_darks.size());
                for (auto const &l : m_photobatch->m_darks) {
                    m_darks_metas.push_back({l.m_path.filename().string(), true});
                }
            }
            break;
        }
        case Ready: {
            std::vector<const char *> items;
            for (const auto &[l, _c] : m_lights_metas)
                items.push_back(l.c_str());

            int selected = m_selected_light;
            ImGui::ListBox("Lights", &selected, items.data(), items.size());
            if (selected != m_selected_light) {
                if (selected >= 0) {
                    auto const &image = m_photobatch->m_lights[selected];
                    screen.load_data_cpu(image.width(), image.height(), image.data());
                }
                m_selected_light = selected;
                m_selected_dark  = -1;
            }

            selected = m_selected_dark;
            items.clear();
            items.reserve(m_darks_metas.size());
            for (const auto &[l, _c] : m_darks_metas)
                items.push_back(l.c_str());
            ImGui::ListBox("Darks", &selected, items.data(), items.size());

            if (selected != m_selected_dark) {
                if (selected >= 0) {
                    auto const &image = m_photobatch->m_darks[selected];
                    screen.load_data_cpu(image.width(), image.height(), image.data());
                }
                m_selected_dark  = selected;
                m_selected_light = -1;
            }
            break;
        }
        }
    }
};

#endif // PHOTOBATCH_H_
