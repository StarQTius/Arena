#pragma once

#include <box2d/b2_math.h>

#include <arena/physics.hpp>

#include "physics.hpp"
#include <forward.hpp>

//! \brief Convert quantities to Box2D objects
//! - A conversion from a quantity will result in the numeric representation of this quantity in the Box2D system of
//! units.
//! - A conversion from `quantity_vector` will result in a `b2Vec2` whose content is the converted quantity coordinates.
//! - Otherwise, no conversion is performed.
decltype(auto) to_box2d(auto &&x) {
  using T = std::remove_cvref_t<decltype(x)>;
  if constexpr (units::Quantity<T>) {
    return arena::box2d_number(FWD(x));
  } else if constexpr (QuantityVector<T>) {
    return b2Vec2{arena::box2d_number(x.x), arena::box2d_number(x.y)};
  } else {
    return FWD(x);
  }
}

//! \brief Convert Box2D objects to quantities
//! It is the inverse function of `to_box2d`.
template <typename To> decltype(auto) from_box2d(auto &&x) {
  using T = std::remove_cvref_t<decltype(x)>;
  if constexpr (std::same_as<T, float> && units::Quantity<To>) {
    return To{x / arena::box2d_number(To{1})};
  } else if constexpr (std::same_as<T, b2Vec2> && QuantityVector<To>) {
    using Q = typename To::quantity;
    return To{from_box2d<Q>(x.x), from_box2d<Q>(x.y)};
  } else {
    return FWD(x);
  }
}

template <typename To, typename From> using from_box2d_t = decltype(from_box2d<To>(std::declval<From>()));

template <typename From> using to_box2d_t = decltype(to_box2d(std::declval<From>()));
