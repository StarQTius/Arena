#pragma once

#include <arena/physics.hpp>
#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/component/ray.hpp>
#include <arena/component/body.hpp>

namespace arena {
namespace system {

length_t cast(const component::Ray &, b2Body *);
std::vector<length_t> sweep(component::Ray, b2Body *, angle_t, std::size_t);

} // namespace system
} // namespace arena
