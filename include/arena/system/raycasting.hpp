#pragma once

#include <arena/physics.hpp>
#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/component/ray.hpp>
#include <arena/component/body.hpp>

namespace arena {
namespace system {

length_t cast(const component::Ray &, b2Body *);

} // namespace system
} // namespace arena
