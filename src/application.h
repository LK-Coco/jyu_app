#pragma once

#include <vector>
#include <memory>
#include <type_traits>
#include <string>

#include "layer.h"

#include "GLFW/glfw3.h"

namespace Jyu {

struct ApplicationSpec {
    std::string name = "Jyu App";
    uint32_t width = 1280;
    uint32_t height = 720;
};

class Application {
public:
    Application(const ApplicationSpec& spec = ApplicationSpec());
    ~Application();

    void run();

    void close();

    float get_time();

    void push_layer(const std::shared_ptr<Layer>& layer) {
        layer_stack_.emplace_back(layer);
        layer->on_start();
    }

    template <typename T,
              typename = std::enable_if_t<std::is_base_of<Layer, T>::value>>
    void push_layer() {
        layer_stack_.emplace_back(std::make_shared<T>())->on_start();
    };

private:
    void init();

    void destroy();

private:
    ApplicationSpec spec_;
    bool is_running{false};
    std::vector<std::shared_ptr<Layer>> layer_stack_;

    GLFWwindow* window_{nullptr};
    float last_frame_time_{0};
    float time_step_;
    float frame_time_;
};

}  // namespace Jyu