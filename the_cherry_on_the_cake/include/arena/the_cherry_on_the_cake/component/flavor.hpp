#pragma once

namespace arena {
namespace coc {
namespace component {

enum class Flavor { ICING, CREAM, SPONGE_CAKE };

} // namespace component
} // namespace coc
} // namespace arena

template <> struct arena_component_info<arena::coc::component::Flavor> {};
