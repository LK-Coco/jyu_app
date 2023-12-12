#include "input.h"
#include "application.h"

#include <GLFW/glfw3.h>

namespace Jyu {

bool Input::is_key_down(KeyCode key_code) {
    GLFWwindow* window_handle = Application::get().get_window_handle();
    int state = glfwGetKey(window_handle, (int)key_code);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::is_mouse_button_down(MouseButton button) {
    GLFWwindow* window_handle = Application::get().get_window_handle();
    int state = glfwGetMouseButton(window_handle, (int)button);
    return state == GLFW_PRESS;
}

glm::vec2 Input::get_mouse_position() {
    GLFWwindow* window_handle = Application::get().get_window_handle();

    double x, y;
    glfwGetCursorPos(window_handle, &x, &y);
    return {(float)x, (float)y};
}

void Input::set_cursor_mode(CursorMode mode) {
    GLFWwindow* window_handle = Application::get().get_window_handle();
    glfwSetInputMode(window_handle, GLFW_CURSOR,
                     GLFW_CURSOR_NORMAL + (int)mode);
}

}  // namespace Jyu