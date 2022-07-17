#pragma once

#include <arena/arena.hpp> // IWYU pragma: export

#define ARENA_FWD(X) static_cast<decltype(X) &&>(X)
