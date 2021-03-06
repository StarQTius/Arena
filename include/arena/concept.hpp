#pragma once

#include <arena/arena.hpp> // IWYU pragma: export

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

// For IWYU
#define Empty Empty

#define ARENA_ALWAYS_FALSE ([]() {}, false)

namespace arena {

// Satisfied when the provided type is an instance of 'std::unique_ptr'
template <typename T>
concept IsUniquePtr = requires(T x) {
  {
    std::unique_ptr { std::move(x) }
    } -> std::convertible_to<T>;
};

template <typename T>
concept Empty = std::is_empty_v<T>;

inline auto dereference_if_needed = []<typename T>(T &&x) -> decltype(auto) {
  if constexpr (IsUniquePtr<std::remove_cvref_t<T>>) {
    return *std::forward<T>(x);
  } else {
    return std::forward<T>(x);
  }
};

inline auto dereference_tuple_elements_if_needed = []<typename T>(T &&t) {
  auto impl = [](auto &&...xs) {
    return []<typename... Args>(Args &&...args) {
      return std::tuple<Args...>{args...};
    }(dereference_if_needed(static_cast<decltype(xs)>(xs))...);
  };

  return std::apply(impl, std::forward<T>(t));
};

} // namespace arena
