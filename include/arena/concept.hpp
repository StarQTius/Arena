#pragma once

#include <concepts>
#include <memory>
#include <utility>

namespace arena {

// Satisfied when the provided type is an instance of 'std::unique_ptr'
template <typename T> concept IsUniquePtr = requires(T x) {
  {
    std::unique_ptr { std::move(x) }
  }
  ->std::same_as<T>;
};

} // namespace arena
