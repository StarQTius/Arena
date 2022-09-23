#pragma once

namespace arena {
namespace coc {
namespace component {

struct CherryLike {
private:
  [[maybe_unused]] int filling;
};

} // namespace component
} // namespace coc
} // namespace arena

template <> struct arena_component_info<arena::coc::component::CherryLike> {};
