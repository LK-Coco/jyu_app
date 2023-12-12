#pragma once

#include "key_codes.h"
#include "glm/glm.hpp"

namespace Jyu {

class Input {
public:
    static bool is_key_down(KeyCode key_code);

    static bool is_mouse_button_down(MouseButton button);

    static glm::vec2 get_mouse_position();

    static void set_cursor_mode(CursorMode mode);
};

}  // namespace Jyu