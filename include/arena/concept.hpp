#pragma once

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace arena {

template <typename> inline constexpr bool always_false = false;

// Satisfied when the provided type is an instance of 'std::unique_ptr'
template <typename T>
concept IsUniquePtr = requires(T x) {
  {
    std::unique_ptr { std::move(x) }
    } -> std::convertible_to<T>;
};

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
