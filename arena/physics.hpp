#pragma once

#include <arena/physics.hpp>

//! \brief Holds of 2D vector of quantities
template <units::Quantity Q> struct quantity_vector {
  using quantity = Q;

  Q x, y;
};

// \brief Satisfied when `T` is an instance of `quantity_vector`
template <typename T>
concept QuantityVector = requires(T x) {
  {
    quantity_vector { x }
    } -> std::convertible_to<T>;
};

using length_vec = quantity_vector<arena::length_t>;
using speed_vec = quantity_vector<arena::speed_t>;
