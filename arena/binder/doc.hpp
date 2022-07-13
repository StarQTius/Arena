#pragma once

#include "traits.hpp" // IWYU pragma: keep
#include <forward.hpp>

decltype(auto) operator|(Binding auto &&binding, std::string_view doc) {
  binding.doc() = doc;

  return FWD(binding);
}
