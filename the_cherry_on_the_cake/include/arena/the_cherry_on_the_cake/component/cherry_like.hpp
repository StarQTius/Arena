#pragma once

#include <arena/arena.hpp> // IWYU pragma: export

namespace arena {
namespace coc {
namespace component {

struct CherryLike {
  int __unused;
};

} // namespace component
} // namespace coc
} // namespace arena

template <> struct arena_component_info<arena::coc::component::CherryLike> {};
