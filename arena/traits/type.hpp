#pragma once

#include <forward.hpp>

// For IWYU
#define ListInitializableFrom ListInitializableFrom

template <typename T, typename... Args>
concept ListInitializableFrom = requires(Args... args) {
  T{FWD(args)...};
};
