#pragma once

#if defined(ARENA_IWYU)
#define ListInitializableFrom ListInitializableFrom
#define Empty Empty
#endif // defined(ARENA_IWYU)

namespace arena {

template <typename T>
concept Empty = std::is_empty_v<T>;

template <typename T, typename... Args>
concept ListInitializableFrom = requires(Args... args) {
  T{FWD(args)...};
};

} // namespace arena
