#pragma once

template <typename T, typename... Args>
concept ListInitializableFrom = requires(Args... args) {
  T{FWD(args)...};
};
