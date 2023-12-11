#pragma once

namespace Jyu {

class Layer {
public:
    virtual ~Layer() = default;

    virtual void on_start(){};
    virtual void on_destroy(){};

    virtual void on_update(float dt){};
    virtual void on_ui_update(){};
};

}  // namespace Jyu