#pragma once

#ifdef RGL_DEBUG
#define RGL_STRINGIFY_DETAIL(x) #x
#define RGL_STR(x) RGL_STRINGIFY_DETAIL(x)
#define RGL_LINEINFO __FILE__ ":" RGL_STR(__LINE__)
#endif  // RGL_DEBUG

#include "glad/glad.h"
